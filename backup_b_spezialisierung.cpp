// =============================================================================
// backup_b_spezialisierung.cpp
//
// Thema        : Template-Spezialisierung — vollständig und partiell.
// Lernziel     : Wann spezialisiert man, wann nimmt man lieber Overloading /
//                if constexpr? Wie bindet man eigene Typen an std::hash?
// Kompilieren  : g++ -std=c++20 -Wall -Wextra backup_b_spezialisierung.cpp \
//                    -o backup_b
// =============================================================================

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

// -----------------------------------------------------------------------------
// Abschnitt A: Vollständige Spezialisierung
// -----------------------------------------------------------------------------
// Das PRIMÄRE Template ist die "generische" Fassung. Für ausgewählte konkrete
// Typen kann man es VOLLSTÄNDIG überschreiben — jede Template-Variable wird
// dabei festgelegt.

// Primary template
template <typename T>
struct TypeName {
    static constexpr const char* value = "unknown";
};

// Vollständige Spezialisierungen — jeweils für einen konkreten T.
template <>
struct TypeName<int> {
    static constexpr const char* value = "int";
};

template <>
struct TypeName<double> {
    static constexpr const char* value = "double";
};

template <>
struct TypeName<std::string> {
    static constexpr const char* value = "std::string";
};

// Tests zur Compile-Zeit (kein Laufzeit-Overhead).
static_assert(std::string_view{TypeName<int>::value}         == "int");
static_assert(std::string_view{TypeName<double>::value}      == "double");
static_assert(std::string_view{TypeName<std::string>::value} == "std::string");

// -----------------------------------------------------------------------------
// Abschnitt B: Partielle Spezialisierung
// -----------------------------------------------------------------------------
// Nicht alle Template-Parameter festlegen — nur die STRUKTUR des Typs.
// Klassisches Beispiel: "ist T ein Pointer?".

template <typename T>
struct IsPointer {
    static constexpr bool value = false;
};

// Partielle Spezialisierung: alle Pointer-Typen T* → true
template <typename T>
struct IsPointer<T*> {
    static constexpr bool value = true;
};

static_assert(!IsPointer<int>::value);
static_assert( IsPointer<int*>::value);
static_assert( IsPointer<const double*>::value);

// -----------------------------------------------------------------------------
// WICHTIG: FUNKTIONEN können NICHT partiell spezialisiert werden.
// Der richtige Weg bei Funktionen ist entweder
//   (a) Overloading (eigene Signatur für den Fall), oder
//   (b) if constexpr im Body (C++17).
// -----------------------------------------------------------------------------

// Variante (a): Overloading
template <typename T>
void describe(const T&) { std::cout << "  generisch\n"; }

template <typename T>
void describe(T*) { std::cout << "  ist Pointer\n"; }

// Variante (b): if constexpr
template <typename T>
void describe2(const T& x)
{
    if constexpr (std::is_pointer_v<T>) {
        std::cout << "  constexpr-Zweig: Pointer, dereferenziert=" << *x << '\n';
    } else if constexpr (std::is_arithmetic_v<T>) {
        std::cout << "  constexpr-Zweig: Zahl = " << x << '\n';
    } else {
        std::cout << "  constexpr-Zweig: anderes\n";
    }
}

// -----------------------------------------------------------------------------
// Abschnitt C: std::hash spezialisieren für eigenen Typ
// -----------------------------------------------------------------------------
// Voraussetzung für die Verwendung in unordered_map/unordered_set ist:
//   1) operator== (Gleichheit)
//   2) eine Spezialisierung von std::hash<T>
//
// Die Spezialisierung muss im Namespace std leben — DAS ist einer der wenigen
// Fälle, in denen das erlaubt (und sogar vorgesehen) ist.

struct SensorId {
    std::string vendor;
    int         serial;

    bool operator==(const SensorId&) const = default;   // C++20 — Memberwise-Vergleich
};

namespace std {
    template <>
    struct hash<SensorId> {
        // noexcept-Zusage hilft Optimierern.
        std::size_t operator()(const SensorId& s) const noexcept
        {
            // Eine einfache Kombination — für produktiven Code z. B. boost::hash_combine.
            std::size_t h1 = std::hash<std::string>{}(s.vendor);
            std::size_t h2 = std::hash<int>{}(s.serial);
            return h1 ^ (h2 << 1);
        }
    };
}

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Vollspezialisierung (TypeName) ===\n";
    std::cout << "  int      → " << TypeName<int>::value         << '\n';
    std::cout << "  double   → " << TypeName<double>::value      << '\n';
    std::cout << "  string   → " << TypeName<std::string>::value << '\n';
    std::cout << "  char     → " << TypeName<char>::value        << " (fallback)\n";

    std::cout << "\n=== Partielle Spezialisierung (IsPointer) ===\n";
    std::cout << std::boolalpha;
    std::cout << "  int      → " << IsPointer<int>::value    << '\n';
    std::cout << "  int*     → " << IsPointer<int*>::value   << '\n';

    std::cout << "\n=== describe (Overloading) ===\n";
    int  n = 5;
    int* p = &n;
    describe(n);
    describe(p);

    std::cout << "\n=== describe2 (if constexpr) ===\n";
    describe2(42);
    describe2(p);
    describe2(std::string{"x"});

    std::cout << "\n=== hash-Spezialisierung ===\n";
    std::unordered_map<SensorId, double> readings;
    readings[{"Acme", 1}] = 23.5;
    readings[{"Acme", 2}] = 24.1;
    for (const auto& [id, value] : readings) {
        std::cout << "  " << id.vendor << '#' << id.serial << " = " << value << '\n';
    }
}
