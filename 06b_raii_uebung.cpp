// =============================================================================
// 06b_raii_uebung.cpp
//
// Thema        : RAII — Übungsaufgaben zum eigenständigen Lösen.
// Lernziel     : RAII selbst anwenden — eigene Wrapper schreiben, Rule of 5
//                praktisch durchspielen.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 06b_raii_uebung.cpp -o 06b_uebung
// =============================================================================

#include <chrono>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// -----------------------------------------------------------------------------
// Aufgabe 1: ScopeGuard — Lambda beim Verlassen des Scopes ausführen
// -----------------------------------------------------------------------------
// Idee: Eine Mini-Klasse, die im Konstruktor ein Lambda speichert und es im
// Destruktor aufruft. Klassischer RAII-Helper für Cleanup-Code, ohne eigenen
// Wrapper-Typ zu schreiben.
//
// Verwendung im Test:
//   {
//       ScopeGuard g{[]{ std::cout << "Cleanup!\n"; }};
//       // ... Code ...
//   }   // <- "Cleanup!" wird hier ausgegeben, auch bei Exception
//
// Tipps:
//   - Template-Parameter F für den Lambda-Typ
//   - Konstruktor speichert das Lambda als Member
//   - Destruktor ruft es auf
//   - Kopie verbieten (= delete) — sonst doppelter Aufruf!
template <typename F>
class ScopeGuard {
public:
    // explicit ScopeGuard(F f) : f_{std::move(f)} {}
    // ~ScopeGuard() { f_(); }
    // ScopeGuard(const ScopeGuard&) = delete;
    // ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    // F f_;
};

// -----------------------------------------------------------------------------
// Aufgabe 2: Timer — misst die Dauer zwischen Konstruktion und Destruktion
// -----------------------------------------------------------------------------
// Verwendung:
//   {
//       Timer t{"Hauptschleife"};
//       // ... was Zeit kostet ...
//   }   // <- Ausgabe z.B.: "Hauptschleife: 12 ms"
//
// Tipps:
//   - std::chrono::steady_clock für Zeitmessung
//   - Im Konstruktor: Startzeit speichern, Label übernehmen
//   - Im Destruktor: Differenz berechnen und ausgeben
class Timer {
public:
    explicit Timer(std::string label)
        // : label_{std::move(label)}, start_{...}
    {
        // TODO
    }

    ~Timer()
    {
        // TODO: Differenz berechnen, ausgeben
    }

private:
    std::string label_;
    // ...
};

// -----------------------------------------------------------------------------
// Aufgabe 3: TempFile — RAII-Wrapper für eine temporäre Datei
// -----------------------------------------------------------------------------
// Aufgabe:
//   - Konstruktor: öffnet eine Datei mit tmpnam() oder festem Namen zum Schreiben
//   - Destruktor: schließt die Datei UND löscht sie vom Dateisystem (std::remove)
//   - Methode write(const std::string&) zum Schreiben
//   - Kopie verbieten, Move erlauben (Rule of 5 — was ist nötig?)
//
// Tipps:
//   - std::FILE* fp_ als Member
//   - std::string path_ um den Pfad für std::remove zu kennen
//   - Im Move: anderen "leer" machen (fp_ = nullptr)
class TempFile {
public:
    explicit TempFile(std::string path)
    {
        // TODO
    }

    ~TempFile()
    {
        // TODO: schließen + löschen
    }

    void write(const std::string& s)
    {
        // TODO
    }

    // TODO: Copy verbieten, Move implementieren

private:
    std::FILE*  fp_   = nullptr;
    std::string path_;
};

// -----------------------------------------------------------------------------
// Aufgabe 4: DynArray<T> — eigener Container mit Rule of 5
// -----------------------------------------------------------------------------
// Implementiere eine sehr einfache Variante von std::vector<T> ohne Wachstum:
//
//   - Konstruktor mit Größe — allokiert n Elemente auf dem Heap
//   - Destruktor — gibt frei
//   - Copy-Ctor — TIEFE Kopie
//   - Copy-Assign — Self-Assignment beachten!
//   - Move-Ctor — übernimmt, lässt anderen leer
//   - Move-Assign — alten Speicher freigeben, übernehmen
//   - operator[] und size()
//
// Tipps:
//   - data_ als T*, size_ als std::size_t
//   - new T[n] / delete[] — für die Übung absichtlich roh; in echtem Code
//     würde man std::unique_ptr<T[]> halten und Rule of 0 ausnutzen
template <typename T>
class DynArray {
public:
    explicit DynArray(std::size_t n)
    {
        // TODO
    }

    ~DynArray()
    {
        // TODO
    }

    // TODO: Copy-Ctor, Copy-Assign, Move-Ctor, Move-Assign

    T&       operator[](std::size_t i)       { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    std::size_t size() const noexcept { return size_; }

private:
    T*          data_ = nullptr;
    std::size_t size_ = 0;
};

// -----------------------------------------------------------------------------
// Aufgabe 5 (Stretch): make_scope_guard() — Factory mit Type-Deduktion
// -----------------------------------------------------------------------------
// Problem: ScopeGuard<F> ist Template — man kann es vor C++17 (CTAD) nicht
// elegant ohne Typ-Angabe verwenden. Eine Helfer-Funktion löst das:
//
//   auto guard = make_scope_guard([]{ std::cout << "bye\n"; });
//
// Tipp: function template, das einen ScopeGuard<F> per Wert zurückgibt
// (Move/RVO).
template <typename F>
auto make_scope_guard(F f)
{
    // TODO
    // return ScopeGuard<F>{std::move(f)};
}

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Aufgabe 1: ScopeGuard ===\n";
    // {
    //     ScopeGuard g{[]{ std::cout << "  -> Cleanup beim Scope-Ende\n"; }};
    //     std::cout << "  Arbeite ...\n";
    // }   // erwartet: "Cleanup beim Scope-Ende"

    std::cout << "\n=== Aufgabe 2: Timer ===\n";
    // {
    //     Timer t{"Schleife"};
    //     volatile long sum = 0;
    //     for (long i = 0; i < 10'000'000; ++i) sum += i;
    // }

    std::cout << "\n=== Aufgabe 3: TempFile ===\n";
    // {
    //     TempFile f{"/tmp/cpp_uebung_test.txt"};
    //     f.write("Hallo aus RAII-Übung\n");
    //     // Datei existiert hier
    // }   // <- hier wird sie automatisch gelöscht
    // std::cout << "  TempFile aus dem Scope — automatisch gelöscht\n";

    std::cout << "\n=== Aufgabe 4: DynArray ===\n";
    // DynArray<int> a{5};
    // for (std::size_t i = 0; i < a.size(); ++i) a[i] = static_cast<int>(i * i);
    //
    // DynArray<int> b{a};                  // Copy
    // DynArray<int> c{std::move(a)};       // Move — a leer
    // b = c;                               // Copy-Assign
    // b = std::move(c);                    // Move-Assign
    // for (std::size_t i = 0; i < b.size(); ++i) std::cout << b[i] << ' ';
    // std::cout << '\n';

    std::cout << "\n=== Stretch: make_scope_guard ===\n";
    // auto guard = make_scope_guard([]{ std::cout << "  Stretch-Cleanup\n"; });
}
