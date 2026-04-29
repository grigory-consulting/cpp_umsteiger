// =============================================================================
// 10_modern_cpp.cpp
//
// Thema        : C++11 / 14 / 17 / 20 — wichtigste Sprachfeatures.
// Lernziel     : Durchgang der in der Praxis relevantesten Neuerungen.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 10_modern_cpp.cpp -o 10_modern
// =============================================================================

#include <algorithm>
#include <any>
#include <filesystem>
#include <format>                 // C++20 — falls vom Compiler unterstützt
#include <iostream>
#include <map>
#include <optional>
#include <ranges>                 // C++20
#include <span>                   // C++20
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

// -----------------------------------------------------------------------------
// C++11 — Die Grundlage modernen C++
// -----------------------------------------------------------------------------
namespace cpp11 {

enum class Color { Red, Green, Blue };    // strong-typed enum: kein impliziter int

struct Point {
    int x = 0;                            // default-Member-Initialisierung
    int y = 0;
};

void demo()
{
    std::cout << "--- C++11 ---\n";

    // auto
    auto i = 42;                 // int
    auto s = std::string{"hi"};
    (void)i; (void)s;

    // Range-based for
    std::vector<int> v{1, 2, 3, 4, 5};
    for (const auto& x : v) std::cout << x << ' ';
    std::cout << '\n';

    // Lambda
    auto is_even = [](int x) { return x % 2 == 0; };
    std::cout << "gerade Werte: " << std::count_if(v.begin(), v.end(), is_even) << '\n';

    // nullptr statt NULL/0
    int* p = nullptr;
    (void)p;

    // enum class — typsicher, benötigt Qualifikation
    Color c = Color::Red;
    (void)c;

    // uniform initialization
    Point pt{10, 20};
    std::cout << "Point: " << pt.x << "," << pt.y << '\n';
}

} // namespace cpp11

// -----------------------------------------------------------------------------
// C++14 — Feinschliff
// -----------------------------------------------------------------------------
namespace cpp14 {

// Generic Lambda — auto-Parameter.
// Praktisch: funktioniert wie ein Template, ohne Template-Syntax.
auto print = [](const auto& x) { std::cout << x << '\n'; };

// Return-Type-Deduktion für normale Funktionen.
auto add(int a, int b) { return a + b; }

void demo()
{
    std::cout << "--- C++14 ---\n";

    // Lambda-Init-Captures — Move in die Lambda hinein.
    auto s = std::make_unique<std::string>("owned by lambda");
    auto lambda = [s = std::move(s)] { std::cout << *s << '\n'; };
    lambda();

    // Binärliterale & Ziffern-Separator
    int mask     = 0b1010'1100;
    int million  = 1'000'000;
    std::cout << "mask=" << mask << " million=" << million << '\n';

    print(add(2, 3));
}

} // namespace cpp14

// -----------------------------------------------------------------------------
// C++17 — Große Ergänzungen
// -----------------------------------------------------------------------------
namespace cpp17 {

// std::optional<T> — "möglicherweise fehlender Wert" ohne Sentinel-Hack.
std::optional<int> parse_port(std::string_view s)
{
    try {
        int p = std::stoi(std::string{s});
        if (p < 0 || p > 65535) return std::nullopt;
        return p;
    } catch (...) {
        return std::nullopt;
    }
}

// std::variant — typsichere Summe mehrerer Typen.
// std::visit + lambda mit if constexpr ist ein häufiges Muster.
using JsonValue = std::variant<int, double, std::string, bool>;

void print_json_value(const JsonValue& v)
{
    std::visit([](const auto& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, bool>)        std::cout << "bool  : " << std::boolalpha << x << '\n';
        else if constexpr (std::is_same_v<T, int>)    std::cout << "int   : " << x << '\n';
        else if constexpr (std::is_same_v<T, double>) std::cout << "double: " << x << '\n';
        else                                          std::cout << "string: " << x << '\n';
    }, v);
}

void demo()
{
    std::cout << "--- C++17 ---\n";

    // Structured bindings
    std::map<std::string, int> ages{{"Alice", 30}, {"Bob", 25}};
    for (const auto& [name, age] : ages) {
        std::cout << name << " = " << age << '\n';
    }

    // if mit Init-Statement
    if (auto it = ages.find("Alice"); it != ages.end()) {
        std::cout << "gefunden: " << it->second << '\n';
    }

    // std::optional
    auto port = parse_port("443");
    if (port) std::cout << "Port = " << *port << '\n';
    else      std::cout << "kein Port\n";

    // std::variant + visit
    JsonValue jv = std::string{"hello"};
    print_json_value(jv);
    jv = 42;
    print_json_value(jv);

    // std::string_view — non-owning String-Referenz. Ideal für Parameter,
    // die read-only Strings akzeptieren (vermeidet Kopien von std::string).
    auto len = [](std::string_view sv) { return sv.size(); };
    std::cout << "len('hello') = " << len("hello") << '\n';

    // CTAD — Template-Argumente werden aus Initialisierer deduziert.
    std::pair p{1, 2.5};                  // std::pair<int, double>
    (void)p;

    // Nested namespaces
    // namespace a::b::c { ... }          // statt drei verschachtelter namespaces
}

} // namespace cpp17

// -----------------------------------------------------------------------------
// C++20 — Moderne Werkzeuge
// -----------------------------------------------------------------------------
namespace cpp20 {

void demo()
{
    std::cout << "--- C++20 ---\n";

    // Ranges + Views — lazy, komponierbar.
    // Die Kette wird erst beim Iterieren ausgeführt.
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = v
        | std::views::filter([](int x) { return x % 2 == 0; })  // nur gerade
        | std::views::transform([](int x) { return x * x; })    // quadrieren
        | std::views::take(3);                                  // erste 3

    std::cout << "ranges: ";
    for (int x : result) std::cout << x << ' ';
    std::cout << '\n';

    // std::span — non-owning View auf zusammenhängende Sequenzen.
    // Perfekter Parameter-Typ für "Funktion bekommt ein Array", ohne sich
    // auf einen bestimmten Container festzulegen.
    auto print_span = [](std::span<const int> s) {
        for (int x : s) std::cout << x << ' ';
        std::cout << '\n';
    };
    print_span(v);                   // vector geht
    int arr[] = {10, 20, 30};
    print_span(arr);                 // C-Array auch

    // std::format (C++20) — Python-style Format-Strings.
    // Hinweis: bis in libstdc++ gcc-13 nur teilweise; wir demonstrieren nur
    // die Syntax. Bei Problemen: einfach auskommentieren.
#if defined(__cpp_lib_format)
    std::cout << std::format("pi ≈ {:.4f}, name = {}\n", 3.14159, "Claude");
#endif
}

} // namespace cpp20

// -----------------------------------------------------------------------------
int main()
{
    cpp11::demo();
    std::cout << '\n';
    cpp14::demo();
    std::cout << '\n';
    cpp17::demo();
    std::cout << '\n';
    cpp20::demo();
}
