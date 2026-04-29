// =============================================================================
// 08_templates.cpp
//
// Thema        : Generische Programmierung — Templates und Concepts.
// Lernziel     : Function- und Class-Templates schreiben, C++20-Concepts
//                einsetzen, if constexpr verstehen.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 08_templates.cpp -o 08_tmpl
// =============================================================================

#include <array>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// -----------------------------------------------------------------------------
// Abschnitt A: Function Template
// -----------------------------------------------------------------------------
// Der Compiler erzeugt pro verwendetem Typ eine eigene Funktions-Instanz.
// Die Template-Parameter werden aus den Argument-Typen deduziert.
template <typename T>
T max_of(const T& a, const T& b)
{
    return (a < b) ? b : a;
}

// -----------------------------------------------------------------------------
// Abschnitt B: Class Template — Ringpuffer
// -----------------------------------------------------------------------------
// Nicht-Typ-Template-Parameter (hier N) sind ebenfalls möglich — sie müssen
// Compile-Zeit-Konstanten sein. So können wir std::array<T, N> als Speicher
// nutzen (Stack-basiert, ohne Heap-Allokation).
template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0, "RingBuffer braucht Kapazität > 0");

public:
    void push(const T& value)
    {
        data_[head_] = value;
        head_ = (head_ + 1) % N;
        if (size_ < N) ++size_;         // solange nicht voll: mitzählen
    }

    std::size_t size()     const noexcept { return size_; }
    std::size_t capacity() const noexcept { return N; }

    // Zugriff auf das i-te Element (0 = ältestes).
    // Hinweis: bei voll ist ältestes Element an Position head_ (danach überschrieben
    // wir als nächstes). Bei nicht-voll beginnt Buffer bei Index 0.
    const T& at(std::size_t i) const
    {
        std::size_t start = (size_ == N) ? head_ : 0;
        return data_[(start + i) % N];
    }

private:
    std::array<T, N> data_{};
    std::size_t      head_ = 0;
    std::size_t      size_ = 0;
};

// -----------------------------------------------------------------------------
// Abschnitt C: if constexpr (C++17)
// -----------------------------------------------------------------------------
// if constexpr entscheidet ZUR COMPILE-ZEIT, welcher Zweig übersetzt wird.
// Der nicht-gewählte Zweig wird IGNORIERT (muss nicht mal kompilieren).
// Das ersetzt sehr oft ältere SFINAE-Tricks oder Template-Spezialisierungen.
template <typename T>
void describe(const T& value)
{
    if constexpr (std::is_integral_v<T>) {
        std::cout << "  integraler Typ, Wert = " << value << '\n';
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "  Gleitkomma-Typ, Wert = " << value << '\n';
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "  String der Länge " << value.size() << '\n';
    } else {
        std::cout << "  anderer Typ\n";
    }
}

// -----------------------------------------------------------------------------
// Abschnitt D: Concepts (C++20)
// -----------------------------------------------------------------------------
// Ein Concept ist ein Compile-Zeit-Prädikat über Typen. Damit werden
// Template-Anforderungen EXPLIZIT und Fehlermeldungen viel lesbarer.

// Standard-Concepts aus <concepts>: std::integral, std::floating_point, ...
template <std::integral T>
T gcd(T a, T b)
{
    while (b) { a %= b; std::swap(a, b); }
    return a;
}

// Eigenes Concept: "T unterstützt das +=-Reduzieren und besitzt ein Nullelement".
// requires-Ausdruck prüft, welche Operationen GÜLTIG sind.
template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    T{};                                // default-konstruierbar
};

template <Addable T>
T sum(const std::vector<T>& v)
{
    T acc{};
    for (const auto& x : v) acc = acc + x;
    return acc;
}

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Function Template ===\n";
    std::cout << "max_of(3, 7)       = " << max_of(3, 7)       << '\n';
    std::cout << "max_of(2.5, 1.8)   = " << max_of(2.5, 1.8)   << '\n';
    std::cout << "max_of(\"a\", \"b\") = "
              << max_of<std::string>("a", "b") << '\n';

    std::cout << "\n=== Class Template (RingBuffer) ===\n";
    RingBuffer<int, 4> rb;
    for (int x : {1, 2, 3, 4, 5, 6}) rb.push(x);     // 3,4,5,6 bleibt übrig
    for (std::size_t i = 0; i < rb.size(); ++i) {
        std::cout << "  rb[" << i << "] = " << rb.at(i) << '\n';
    }

    std::cout << "\n=== if constexpr ===\n";
    describe(42);
    describe(3.14);
    describe(std::string{"hallo"});

    std::cout << "\n=== Concepts ===\n";
    std::cout << "gcd(48, 18) = " << gcd(48, 18) << '\n';
    // gcd(1.5, 2.0);              // Compile-Fehler — floating_point nicht integral

    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "sum(v) = " << sum(v) << '\n';
}
