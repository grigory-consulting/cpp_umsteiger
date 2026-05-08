// =============================================================================
// backup_a_move.cpp
//
// Thema        : Move-Semantik — lvalues/rvalues, std::move, Rule of 5,
//                Perfect Forwarding.
// Lernziel     : Verstehen, wann und warum Move passiert; korrekt eigene
//                Move-Operationen schreiben.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra backup_a_move.cpp -o backup_a_move
// =============================================================================

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// =============================================================================
// EINSTIEG: Warum gibt es Move-Semantik überhaupt?
// =============================================================================
// Vor C++11 hatten Funktions-Returns mit großen Objekten (z.B. std::vector mit
// 1 Million Elementen) ein Problem:
//
//     std::vector<int> make_data() {
//         std::vector<int> v(1'000'000);
//         /* füllen */
//         return v;        // <-- KOPIE? Wirklich 1 Mio. Ints kopieren?
//     }
//
// Ohne Move-Semantik müsste der Compiler entweder:
//   (a) tatsächlich kopieren (langsam!) oder
//   (b) tricksen (RVO — funktionierte nicht immer)
//
// Move-Semantik (C++11) löst das: der Inhalt eines TEMPORÄRES Objekt darf
// "ausgeschlachtet" werden — der neue Besitzer übernimmt einfach die
// Pointer/Heap-Allokation, der alte gibt sie auf. Keine Kopie, nur
// Pointer-Umhängung — also O(1) statt O(n).
//
// Dafür braucht es ein Vokabular: lvalue / rvalue. Das ist Abschnitt A.

// -----------------------------------------------------------------------------
// Abschnitt A: lvalues vs rvalues — das grundlegende Vokabular
// -----------------------------------------------------------------------------
// Jeder Ausdruck in C++ ist entweder ein lvalue oder ein rvalue.
//
//   lvalue — hat einen NAMEN / eine ADRESSE, lebt länger als der aktuelle
//            Ausdruck. Beispiele: Variablen, Member, Array-Elemente.
//            Merksatz: "steht links vom = " (locator value).
//
//   rvalue — TEMPORÄRER Wert, der nach dem aktuellen Ausdruck stirbt.
//            Beispiele: Literale (42), Ergebnisse von Berechnungen (x + 1),
//            Rückgabewerte von Funktionen.
//            Merksatz: "darf ausgeschlachtet werden, niemand vermisst es".
//
// Daraus folgen zwei Referenz-Arten:
//
//   T&   — lvalue-Referenz: bindet nur an lvalues. "Ich modifiziere ein Original."
//   T&&  — rvalue-Referenz: bindet nur an rvalues. "Ich darf ausschlachten."
//
//   const T& — Sonderfall: bindet an BEIDES.
//              Deshalb war "const T&" jahrelang der Default für read-only Parameter.

void demo_lvalue_rvalue()
{
    std::cout << "--- lvalue / rvalue ---\n";
    int x = 42;             // x   ist LVALUE — hat Namen, hat Adresse (&x)
    int y = x + 1;          // x+1 ist RVALUE — temporäres Ergebnis, kein Name
    (void)y;

    // Bindungsregeln:
    int&  lref = x;         // ✓  lvalue-ref an lvalue
    int&& rref = x + 1;     // ✓  rvalue-ref an rvalue
                            //    (verlängert sogar die Lebensdauer des Temporären
                            //     bis zum Ende des Scopes — siehe const T& tricks)

    // Verbotene Kombinationen:
    // int&  bad1 = x + 1;  // ✗  rvalue an lvalue-ref
    //                      //    Würde Modifikation eines gleich-toten Objekts erlauben.
    // int&& bad2 = x;      // ✗  lvalue an rvalue-ref
    //                      //    x lebt weiter — nicht "ausschlachtbar".
    //                      //    Mit std::move(x) würde es gehen (siehe Abschnitt B).

    std::cout << "  lref=" << lref << " rref=" << rref << '\n';
}

// -----------------------------------------------------------------------------
// Abschnitt B: std::move ist NUR ein Cast — kein Move!
// -----------------------------------------------------------------------------
// MISSVERSTÄNDNIS Nr. 1: "std::move bewegt das Objekt"
// FALSCH. std::move(x) bewegt GAR NICHTS. Es ist ein reiner Compile-Time-Cast,
// der x zu einer rvalue-Referenz macht. Es ist äquivalent zu:
//
//     static_cast<T&&>(x)
//
// Was wirklich passiert, hängt davon ab, was DANACH mit dem Cast-Ergebnis
// gemacht wird:
//
//   - Wird es an einen MOVE-Konstruktor / MOVE-Assign übergeben?
//     -> dann findet ein Move statt (Pointer-Umhängung statt Kopie).
//   - Wird es an einen COPY-Konstruktor übergeben (weil es keinen Move-Ctor
//     gibt)?
//     -> dann wird trotzdem kopiert.
//   - Bei trivialen Typen (int, double, Pointer) gibt es keinen "Move":
//     std::move(int) = Kopie. Also harmlos, aber sinnlos.
//
// MISSVERSTÄNDNIS Nr. 2: "Nach std::move ist das Original kaputt"
// FALSCH. Es ist "valid but unspecified state" — gültig, aber sein Inhalt
// ist nicht garantiert. Du DARFST das Objekt:
//
//   ✓ zerstören  (immer ok — der Destruktor muss das überleben)
//   ✓ neu zuweisen  (s = "neuer wert";  s.clear();  s.assign(...))
//   ✓ size() / empty() abfragen  (für die Standard-Typen wohldefiniert)
//
// Du DARFST aber nicht annehmen, dass der Inhalt unverändert ist:
//
//   ✗ den alten Wert lesen
//   ✗ darauf vertrauen, dass es leer ist (für custom Klassen ggf. nicht!)
//
// Bei std::string und std::vector ist nach Move typischerweise leer, aber
// das ist NICHT garantiert — nur empirisch häufig.

void demo_std_move()
{
    std::cout << "--- std::move ---\n";

    std::string s1 = "eine längere Zeichenkette mit Heap-Speicher";

    // COPY: s2 bekommt EINE EIGENE Heap-Allokation, deren Inhalt von s1
    // bytweise kopiert wird. Bei langen Strings: teuer.
    //
    //   vorher:    s1 ─→ [Heap: "eine längere ..."]
    //   nachher:   s1 ─→ [Heap: "eine längere ..."]   (unverändert)
    //              s2 ─→ [Heap: "eine längere ..."]   (Kopie)
    std::string s2 = s1;

    // MOVE: s3 übernimmt den Pointer auf den Heap-Speicher von s1. Kein
    // Allokieren, kein Kopieren — nur Pointer-Umhängung.
    //
    //   vorher:    s1 ─→ [Heap: "eine längere ..."]
    //   nachher:   s1 ─→ [Heap: ""]   (leer/unspezifiziert)
    //              s3 ─→ [Heap: "eine längere ..."]   (übernommen)
    std::string s3 = std::move(s1);

    // s1 ist jetzt "moved-from": gültig, aber Inhalt unbestimmt.
    // size() ist erlaubt — hier typischerweise 0.
    std::cout << "  s1 (moved-from) size = " << s1.size() << '\n';
    std::cout << "  s2 (Kopie)         = " << s2 << '\n';
    std::cout << "  s3 (übernommen)    = " << s3 << '\n';

    // Wir DÜRFEN s1 wieder neu beschreiben — es ist ein gültiges Objekt:
    s1 = "neu befüllt";
    std::cout << "  s1 (neu)           = " << s1 << '\n';
}

// -----------------------------------------------------------------------------
// Abschnitt C: Rule of 5 — die fünf "speziellen" Member-Funktionen
// -----------------------------------------------------------------------------
// Wenn eine Klasse selbst eine Ressource verwaltet (Heap-Speicher, File-Handle,
// Mutex, ...) muss sie genau definieren, was bei den fünf Lebens-Ereignissen
// passiert:
//
//   1. Destruktor             ~T()                 Wie wird die Ressource freigegeben?
//   2. Copy-Konstruktor       T(const T&)          Wie wird tief kopiert?
//   3. Copy-Assignment        T& operator=(const T&)
//   4. Move-Konstruktor       T(T&&) noexcept      Wie wird der Inhalt geklaut?
//   5. Move-Assignment        T& operator=(T&&) noexcept
//
// REGEL: Schreibst du EINE davon explizit, schreibe ALLE fünf (oder = default /
//        = delete sie bewusst). Sonst bekommst du subtile Fehler — z.B. einen
//        Compiler-generierten Copy-Ctor, der den selbst geschriebenen Destruktor
//        nicht versteht und doppelt freigibt.
//
// IM IDEALFALL aber: keine eigene Ressourcenverwaltung — stattdessen STL-Typen
// (std::vector, std::unique_ptr, std::string) als Member halten. Dann kann der
// Compiler ALLE fünf richtig generieren. Das ist die "Rule of 0" — und das ist
// der Default-Stil im modernen C++.
//
// Der untenstehende Buffer ist PÄDAGOGISCH — in echtem Code würde man einfach
// schreiben: std::vector<char> data_; und alles ginge automatisch.

class Buffer {
public:
    explicit Buffer(std::size_t n, const char* tag = "")
        : data_{new char[n]}, size_{n}, tag_{tag}
    {
        std::cout << "  + ctor(" << tag_ << ", " << n << ")\n";
    }

    ~Buffer()
    {
        delete[] data_;
        std::cout << "  - dtor(" << tag_ << ", " << size_ << ")\n";
    }

    // Copy — tiefe Kopie.
    Buffer(const Buffer& other)
        : data_{new char[other.size_]}
        , size_{other.size_}
        , tag_{other.tag_}
    {
        std::memcpy(data_, other.data_, size_);
        std::cout << "  copy-ctor(" << tag_ << ")\n";
    }

    Buffer& operator=(const Buffer& other)
    {
        if (this != &other) {
            char* new_data = new char[other.size_];
            std::memcpy(new_data, other.data_, other.size_);
            delete[] data_;
            data_ = new_data;
            size_ = other.size_;
            tag_  = other.tag_;
        }
        std::cout << "  copy-assign(" << tag_ << ")\n";
        return *this;
    }

    // Move — "klaut" Ressourcen. noexcept ist WICHTIG:
    // std::vector<Buffer> kann bei Realloc nur dann MOVE statt COPY nutzen,
    // wenn der Move-Ctor noexcept ist (sonst Exception-Safety-Problem).
    Buffer(Buffer&& other) noexcept
        : data_{other.data_}
        , size_{other.size_}
        , tag_{other.tag_}
    {
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "  move-ctor(" << tag_ << ")\n";
    }

    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other) {
            delete[] data_;
            data_       = other.data_;
            size_       = other.size_;
            tag_        = other.tag_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        std::cout << "  move-assign(" << tag_ << ")\n";
        return *this;
    }

private:
    char*       data_ = nullptr;
    std::size_t size_ = 0;
    const char* tag_  = "";
};

void demo_rule_of_five()
{
    std::cout << "--- Rule of 5 ---\n";

    // Was hier passiert (Schritt für Schritt):
    //
    //   Buffer a{64, "A"};       --> + ctor(A, 64)         a.data_ ─→ [Heap 64B]
    //
    //   Buffer b{a};             --> copy-ctor(A)          b.data_ ─→ [eigene Heap 64B] (Inhalt kopiert)
    //
    //   Buffer c{std::move(a)};  --> move-ctor(A)          c.data_ ─→ a.data_  (übernommen)
    //                                                      a.data_ = nullptr  (a ist jetzt "leer")
    //
    //   b = c;                   --> copy-assign(A)        b.data_ alter Block freigegeben,
    //                                                      neuer Heap 64B mit Inhalt von c
    //
    //   b = std::move(c);        --> move-assign(A)        b.data_ alter Block freigegeben,
    //                                                      übernimmt c.data_
    //                                                      c.data_ = nullptr
    //
    // Am Scope-Ende:
    //   - dtor(c, 0)             c hat data_ = nullptr → delete[] nullptr ist ok
    //   - dtor(b, 64)            gibt seinen aktuellen Block frei
    //   - dtor(a, 0)             a wurde gemoved → leerer Block
    Buffer a{64, "A"};
    Buffer b{a};                          // copy
    Buffer c{std::move(a)};               // move — a ist danach leer
    b = c;                                // copy-assign
    b = std::move(c);                     // move-assign
}

// -----------------------------------------------------------------------------
// Abschnitt D: Perfect Forwarding — Argumente weiterreichen ohne Info-Verlust
// -----------------------------------------------------------------------------
// PROBLEM:
// Eine Factory-Funktion (denke an std::make_unique, std::make_shared,
// emplace_back, ...) soll ihre Argumente AN einen Konstruktor weiterreichen
// — ohne dabei Kopien zu erzwingen UND ohne die lvalue/rvalue-Information
// zu verlieren.
//
// Naiver Versuch:
//
//     template <typename T, typename Arg>
//     std::unique_ptr<T> make_mine(Arg arg) { return ...new T(arg)...; }
//
// Das KOPIERT arg einmal in den Parameter, übergibt die KOPIE als lvalue
// an T(...). Selbst wenn der Aufrufer std::move benutzt hat, ist die
// Move-Information beim T-Konstruktor nicht mehr verfügbar.
//
// LÖSUNG (Scott Meyers' "Universal/Forwarding References"):
//
//     template <typename... Args>
//     ... f(Args&&... args) { ... new T(std::forward<Args>(args)...) ... }
//
// Das "Args&&" sieht aus wie eine rvalue-Ref, ist aber im TEMPLATE-Kontext
// etwas Besonderes: eine "Forwarding-Reference". Sie passt sich dem Aufrufer
// an — die Reference-Collapsing-Regeln sorgen dafür, dass:
//
//    Aufruf:                Args wird deduziert als:    Args&&  ist effektiv:
//    f(int_lvalue)          int&                        int&&& -> int&
//    f(int_rvalue)          int                         int&&
//    f(std::move(x))        int                         int&&
//
// std::forward<Args>(args) leitet diese Information weiter:
//    - war Args ein lvalue-Typ -> forward gibt lvalue zurück
//    - war Args ein rvalue-Typ -> forward gibt rvalue zurück
//
// Resultat: T's Konstruktor sieht das Argument so, wie es der Aufrufer
// übergeben hat — als lvalue oder rvalue. Nichts wird unnötig kopiert.

struct Widget {
    Widget()                      { std::cout << "    Widget()\n"; }
    Widget(int n)                 { std::cout << "    Widget(int=" << n << ")\n"; }
    Widget(const std::string& s)  { std::cout << "    Widget(const string&=" << s << ")\n"; }
    Widget(std::string&& s)       { std::cout << "    Widget(string&&=" << s << ")\n"; }
};

// make_mine ist eine vereinfachte std::make_unique-Implementierung.
// Args&& sind FORWARDING-REFERENCES (kein "rvalue-Ref"!), weil Args ein
// abgeleiteter Template-Parameter ist.
template <typename T, typename... Args>
std::unique_ptr<T> make_mine(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void demo_perfect_forwarding()
{
    std::cout << "--- Perfect Forwarding ---\n";

    // Kein Argument: Args ist leer, T() wird gerufen.
    auto w1 = make_mine<Widget>();                // -> Widget()

    // 42 ist ein int-Literal, also ein RVALUE.
    // Args wird zu (int), Args&& effektiv int&&. forward<int>(42) bleibt rvalue.
    // T(int) wird aufgerufen.
    auto w2 = make_mine<Widget>(42);              // -> Widget(int)

    std::string s = "hallo";

    // s ist ein LVALUE (hat Namen).
    // Args wird zu (std::string&), Args&& effektiv std::string& (Reference Collapsing).
    // forward<std::string&>(s) liefert lvalue -> Widget(const string&) wird gewählt.
    auto w3 = make_mine<Widget>(s);               // -> Widget(const string&)

    // std::move(s) macht aus s einen RVALUE-Cast.
    // Args wird zu (std::string), Args&& effektiv std::string&&.
    // forward<std::string>(...) liefert rvalue -> Widget(string&&) wird gewählt.
    auto w4 = make_mine<Widget>(std::move(s));    // -> Widget(string&&)

    // Im echten Code würdest du das hier einfach als
    //   auto w = std::make_unique<Widget>(...);
    // schreiben — make_mine ist nur die didaktische Variante.
}

// -----------------------------------------------------------------------------
int main()
{
    demo_lvalue_rvalue();
    std::cout << '\n';
    demo_std_move();
    std::cout << '\n';
    demo_rule_of_five();
    std::cout << '\n';
    demo_perfect_forwarding();
}
