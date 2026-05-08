// =============================================================================
// 01_grundlagen.cpp
//
// Thema        : Syntaktischer Überblick — Variablen, Kontrollstrukturen,
//                Klassen.
// Lernziel     : Refresher über moderne Syntax. Die Konzepte werden als
//                bekannt vorausgesetzt.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 01_grundlagen.cpp -o 01_grundlagen
// =============================================================================

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>

// -----------------------------------------------------------------------------
// Abschnitt A: Variablen & Typen
// -----------------------------------------------------------------------------
// Die Grundtypen sollten bekannt sein. Interessant sind die MODERNEN
// Initialisierungsformen und die fest-breiten Integer-Typen aus <cstdint>.
namespace variablen {

void demo()
{
    // Fundamentale Typen
    int    i = 42;
    double d = 3.14;
    char   c = 'x';
    bool   b = true;

    // Fest-breite Integer — bevorzugt in Schnittstellen / Protokollen,
    // wo die genaue Byte-Größe garantiert sein muss.
    std::int32_t  n32 = 100'000;         // Ziffern-Separator ' (C++14)
    std::uint64_t big = 1ULL << 40;

    // Brace-Initialisierung (C++11) — verbietet IMPLIZITE narrowing-Conversions.
    // int bad{3.14};   // Compile-Fehler: double → int würde Information verlieren
    int x{7};

    // auto — Typ wird vom Initialisierer abgeleitet.
    // Vorteil: weniger Redundanz; Nachteil: Typ ist nicht immer auf den ersten
    // Blick erkennbar. In Schnittstellen eher zurückhaltend einsetzen.
    auto y = 2.5;                        // double
    auto s = std::string{"hallo"};       // std::string

    // const   → Wert darf zur LAUFZEIT nicht geändert werden.
    // constexpr → Wert muss zur COMPILE-ZEIT berechenbar sein.
    const     int N   = 10;
    constexpr int MAX = 2 * N;

    std::cout << "i=" << i << " d=" << d << " c=" << c << " b=" << b << '\n';
    std::cout << "n32=" << n32 << " big=" << big << '\n';
    std::cout << "x=" << x << " y=" << y << " s=" << s << '\n';
    std::cout << "N=" << N << " MAX=" << MAX << '\n';
}

} // namespace variablen

// -----------------------------------------------------------------------------
// Abschnitt B: Kontrollstrukturen
// -----------------------------------------------------------------------------
namespace kontrollfluss {

// Hilfsfunktion für die if-mit-init-Demo
std::pair<bool, int> lookup(int key)
{
    // Simulierter Lookup
    if (key == 42) return {true, 1337};
    return {false, 0};
}

void demo()
{
    // if mit Init-Statement (C++17) — Variable lebt NUR innerhalb des if/else.
    // Schöner als vorgezogene Deklaration, engerer Scope = weniger Fehlerflächen.
    if (auto [found, value] = lookup(42); found) {
        std::cout << "gefunden: " << value << '\n';
    } else {
        std::cout << "nicht gefunden\n";
    }

    // switch mit [[fallthrough]] — macht absichtliches Durchfallen EXPLIZIT.
    int mode = 2;
    switch (mode) {
        case 1:
            std::cout << "Modus 1\n";
            break;
        case 2:
            std::cout << "Modus 2 — fällt durch\n";
            [[fallthrough]];                 // bewusst, Compiler ruhigstellen
        case 3:
            std::cout << "Modus 3\n";
            break;
        default:
            std::cout << "unbekannt\n";
    }

    // Schleifen
    std::array<int, 5> data{10, 20, 30, 40, 50};

    // Range-based for (C++11) — wenn kein Index gebraucht wird: IMMER diese Form.
    // const auto& vermeidet Kopie, erzwingt read-only Zugriff.
    int sum = 0;
    for (const auto& x : data) sum += x;
    std::cout << "Summe = " << sum << '\n';
    
    for (auto& n : data) {
        n *= 2;
    }

    for (const auto& n : data) {
        std::cout << n << " ";
    }

    

    // Klassische for-Schleife — nur noch nötig wenn Index benötigt wird.
    for (std::size_t i = 0; i < data.size(); ++i) {
        std::cout << "data[" << i << "] = " << data[i] << '\n';
    }
}

} // namespace kontrollfluss

// -----------------------------------------------------------------------------
// Abschnitt C: Klassen — Grundform
// -----------------------------------------------------------------------------
// Eine kleine Domain-Klasse als Refresher. Punkte zum Mitnehmen:
//   - Member-Initialisierungsliste (nicht Zuweisung im Body!)
//   - const-correctness: Getter ist const
//   - noexcept wenn die Methode GARANTIERT nicht wirft — erlaubt Optimierungen
//   - Trailing-underscore-Konvention für Member (vermeidet Namenskonflikte)
class Sensor {
public:
    Sensor(std::string name, double threshold)
        : name_{std::move(name)}          // move, da 'name' lokale Kopie ist
        , threshold_{threshold}
    {}

    // Getter mit drei modernen Annotationen:
    //
    //   const std::string&  name()  const  noexcept  { return name_; }
    //   ──────┬──────────  ──┬──   ──┬──   ──┬─────
    //         1.              2.      3.       4.
    //
    // 1. const std::string&  Rückgabe per const-Referenz — keine Kopie,
    //                        Aufrufer darf den String nicht verändern.
    // 2. name()              Methodenname (Konvention: wie das Member ohne
    //                        trailing underscore).
    // 3. const  (nach den Klammern!)
    //                        Diese Methode verändert das Objekt nicht.
    //                        Sie darf deshalb auch auf const-Sensoren aufgerufen
    //                        werden — Faustregel: jeder Getter ist const.
    // 4. noexcept            Versprechen: wirft KEINE Exception. Erlaubt dem
    //                        Compiler Optimierungen; bei trivialen Gettern
    //                        immer setzen.
    //
    // Merksatz: 1 und 2 wirken auf den AUFRUFER, 3 und 4 auf das OBJEKT.
    const std::string& name() const noexcept { return name_; }

    bool triggers(double value) const noexcept { return value > threshold_; }

private:
    std::string name_;
    double      threshold_;
};

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Variablen ===\n";
    variablen::demo();

    std::cout << "\n=== Kontrollstrukturen ===\n";
    kontrollfluss::demo();

    std::cout << "\n=== Klassen ===\n";
    Sensor temp{"Temp-1", 80.0};
    std::cout << temp.name() << ": 75°C → triggers? "
              << std::boolalpha << temp.triggers(75.0) << '\n';
    std::cout << temp.name() << ": 85°C → triggers? "
              << std::boolalpha << temp.triggers(85.0) << '\n';
}
