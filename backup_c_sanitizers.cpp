// =============================================================================
// backup_c_sanitizers.cpp
//
// Thema        : Sanitizers — ASan, UBSan, TSan.
// Lernziel     : Typische Fehler bewusst auslösen und sehen, wie Sanitizers
//                sie zur LAUFZEIT mit Stacktrace reporten.
//
// ACHTUNG      : Dieses Programm enthält ABSICHTLICHE Bugs. Es ist nicht dafür
//                gedacht, "normal" ausgeführt zu werden — der Wert liegt im
//                Sanitizer-Report.
//
// Kompilieren mit AddressSanitizer + UndefinedBehaviorSanitizer:
//     g++ -std=c++20 -Wall -Wextra \
//         -fsanitize=address,undefined \
//         -fno-omit-frame-pointer -g -O1 \
//         backup_c_sanitizers.cpp -o backup_c
//
// Ausführen:
//     ./backup_c
//
// Für ThreadSanitizer (separat, NICHT mit ASan kombinieren!):
//     g++ -std=c++20 -Wall -Wextra \
//         -fsanitize=thread -g -O1 -pthread \
//         backup_c_sanitizers.cpp -o backup_c_tsan \
//         -DDEMO_TSAN
// =============================================================================

#include <iostream>
#include <thread>
#include <vector>

// -----------------------------------------------------------------------------
// Bug 1 (ASan): Heap-Buffer-Overflow
// -----------------------------------------------------------------------------
// Wir allokieren ein Array mit 10 Elementen und lesen an Index 10 — einer zu
// viel. ASan erkennt das direkt mit Stacktrace.
void bug_heap_overflow()
{
    int* arr = new int[10];
    for (int i = 0; i < 10; ++i) arr[i] = i;

    int out_of_bounds = arr[10];             // ⚠ HEAP-BUFFER-OVERFLOW
    std::cout << "  (sollte nicht reportet werden): " << out_of_bounds << '\n';

    delete[] arr;
}

// -----------------------------------------------------------------------------
// Bug 2 (ASan): Use-after-free
// -----------------------------------------------------------------------------
// Nach dem delete benutzen wir den Pointer weiter — ASan reportet sowohl den
// Frame des delete als auch den Frame des fehlerhaften Access.
void bug_use_after_free()
{
    int* p = new int{42};
    delete p;

    *p = 99;                                 // ⚠ USE-AFTER-FREE
}

// -----------------------------------------------------------------------------
// Bug 3 (UBSan): Signed Integer Overflow
// -----------------------------------------------------------------------------
// Signed Overflow ist in C++ UNDEFINIERT — der Compiler darf beliebigen Code
// generieren. UBSan erkennt es zur Laufzeit.
void bug_signed_overflow()
{
    int max = 2'147'483'647;                 // INT_MAX
    int x   = max + 1;                       // ⚠ SIGNED OVERFLOW (UB)
    std::cout << "  (undefined) " << x << '\n';
}

// -----------------------------------------------------------------------------
// Bug 4 (UBSan): Nullpointer-Dereferenzierung
// -----------------------------------------------------------------------------
void bug_null_deref()
{
    int* p = nullptr;
    int  v = *p;                             // ⚠ NULL POINTER DEREFERENCE (UB)
    std::cout << "  (unreachable) " << v << '\n';
}

// -----------------------------------------------------------------------------
// Bug 5 (TSan): Data Race
// -----------------------------------------------------------------------------
// Zwei Threads schreiben/lesen dieselbe Variable ohne Synchronisation — TSan
// reportet das als data race mit beiden beteiligten Stacktraces.
#ifdef DEMO_TSAN
int shared_counter = 0;                      // KEINE Synchronisation — absichtlich!

void racy_increment(int n)
{
    for (int i = 0; i < n; ++i) ++shared_counter;   // ⚠ DATA RACE
}

void bug_data_race()
{
    std::thread t1{racy_increment, 100'000};
    std::thread t2{racy_increment, 100'000};
    t1.join();
    t2.join();
    std::cout << "  counter = " << shared_counter
              << " (undefiniert, nicht notwendig 200'000)\n";
}
#endif

// -----------------------------------------------------------------------------
// main — WELCHEN Bug wir demonstrieren, steuern wir über ein Argument.
// Bei Ausführung ohne Argument: Hinweis drucken.
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout <<
            "Dieses Programm triggert absichtliche Fehler.\n"
            "Aufruf:\n"
            "  ./backup_c overflow   # Heap-buffer-overflow (ASan)\n"
            "  ./backup_c use-free   # Use-after-free       (ASan)\n"
            "  ./backup_c signed     # Signed overflow      (UBSan)\n"
            "  ./backup_c null       # Null-Dereferenz      (UBSan)\n"
#ifdef DEMO_TSAN
            "  ./backup_c race       # Data race            (TSan)\n"
#endif
            "\n"
            "Voraussetzung: Build mit -fsanitize=... (siehe Dateikopf).\n";
        return 0;
    }

    std::string which = argv[1];
    if      (which == "overflow") bug_heap_overflow();
    else if (which == "use-free") bug_use_after_free();
    else if (which == "signed")   bug_signed_overflow();
    else if (which == "null")     bug_null_deref();
#ifdef DEMO_TSAN
    else if (which == "race")     bug_data_race();
#endif
    else std::cout << "unbekanntes Argument: " << which << '\n';
}
