// =============================================================================
// 04_funktionen.cpp
//
// Thema        : Funktionen und Parameterübergabe.
// Lernziel     : Überblick der Übergabe-Mechanismen, Überladung, Lambdas,
//                constexpr, RVO / NRVO.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 04_funktionen.cpp -o 04_funktionen
// =============================================================================

#include <iostream>
#include <string>
#include <string_view>
#include <vector>


void by_value(int x) {  }


void by_const_ref(const std::string& s)
{
   
}

void by_ref(std::string& s) {  }


void by_pointer(std::string* s)
{
    
}


void by_rvalue_ref(std::string&& s)
{
    
}

// -----------------------------------------------------------------------------
// Abschnitt B: Überladung & default-Argumente
// -----------------------------------------------------------------------------
int max_of(int a, int b)          {  }
double max_of(double a, double b) {  }

void connect(std::string_view host, int port = 8080)
{
}


void lambdas()
{
}

// -----------------------------------------------------------------------------
// Abschnitt D: constexpr — compile-time Funktionen
// -----------------------------------------------------------------------------
// constexpr bedeutet: Funktion KANN zur Compile-Zeit ausgewertet werden,
// wenn alle Argumente Compile-Zeit-Konstanten sind.
constexpr int square(int x) {  }

//static_assert(square(5) == 25, "square ist kaputt");
//static_assert(square(0) == 0);

// -----------------------------------------------------------------------------
// Abschnitt E: Return Value Optimization (RVO)
// -----------------------------------------------------------------------------
// "Return by value" ist in C++ NICHT teuer. Der Compiler optimiert so, dass
// das Objekt DIREKT am Zielort konstruiert wird — keine Kopie.
// Seit C++17 ist das bei Prvalues GARANTIERT (guaranteed copy elision).
std::vector<int> make_data(std::size_t n)
{
}

// Typische ANTI-Pattern (alte C-Schule): Output-Parameter statt Return.
// In modernem C++ NICHT NÖTIG — RVO macht den "Return by value" genauso schnell
// und der Code ist lesbarer.
void make_data_bad(std::size_t n, std::vector<int>& out)
{
}

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Parameterübergabe ===\n";
    int a = 5;
    by_value(a);
    std::cout << "nach by_value: a = " << a << " (unverändert)\n";

    std::string s = "hallo";
    by_const_ref(s);
    by_ref(s);
    std::cout << "nach by_ref: " << s << '\n';

    by_pointer(&s);
    by_pointer(nullptr);                // erlaubt
    std::cout << "nach by_pointer: " << s << '\n';

    by_rvalue_ref(std::string{"temporär"});

    std::cout << "\n=== Überladung ===\n";
    std::cout << max_of(3, 7)     << '\n';
    std::cout << max_of(2.5, 1.8) << '\n';
    connect("example.com");                 // port default 8080
    connect("example.com", 443);

    std::cout << "\n=== Lambdas ===\n";
    lambdas();

    std::cout << "\n=== constexpr ===\n";
    //constexpr int v = square(7);        // zur Compile-Zeit ausgewertet
    // std::cout << "square(7) = " << v << '\n';

    std::cout << "\n=== RVO ===\n";
    auto data = make_data(5);
    std::cout << "make_data(5) ergab " << data.size() << " Elemente\n";
}
