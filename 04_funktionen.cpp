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


void by_value(int x) { x+=1; (void)x; }


void by_const_ref(const std::string& s)
{
    std::cout << s << "\n";
   
}

void by_ref(std::string& s) { s+= " modifiziert";  }


void by_pointer(std::string* s)
{
    if (s) *s += " modifiziert via Pointer";
}


void by_rvalue_ref(std::string&& s)
{
    std::string own = std::move(s); // Ownership übernommen
    std::cout << " übernommen";
}

// -----------------------------------------------------------------------------
// Abschnitt B: Überladung & default-Argumente
// -----------------------------------------------------------------------------
int max_of(int a, int b)          { return a>b ? a : b; }
double max_of(double a, double b) { return a>b ? a : b; }

void connect(std::string_view host, int port = 8080)
{
    std::cout << port << "\n";
}


void fnc(bool (*func)(int))
{
    std::cout << func(1);
}





void lambdas()
{
    // anonyme Funktionen

    int threshold = 10;

    // Capture by Value
    auto above_one = [threshold](int x) {return x > threshold; };

    // Capture by Reference
    auto above_two = [&threshold](int x) {return x > threshold; };

    threshold = 100;


    std::cout << "\n" << above_one(50) << "\n" << above_two(50) << "\n";

    // Generic Lambda (C++14)
    auto plus = [](auto a, auto b){return a+b;};
    std::cout << plus(1.5, 2) << "\n";

    // Lambda-Init-Capture
    std::vector<int> data{1 , 2, 3};
    auto owned = [v = std::move(data)](){ // v ist owned mit std::vector<int>{1,2,3} als Inhalt
        int sum = 0;
        for (int x : v){
            sum += x;
        }
        return sum;
    };


}

// -----------------------------------------------------------------------------
// Abschnitt D: constexpr — compile-time Funktionen
// -----------------------------------------------------------------------------
// constexpr bedeutet: Funktion KANN zur Compile-Zeit ausgewertet werden,
// wenn alle Argumente Compile-Zeit-Konstanten sind.
constexpr int square(int x) { return x * x; }

static_assert(square(5) == 25, "square ist kaputt");
static_assert(square(0) == 0);

// -----------------------------------------------------------------------------
// Abschnitt E: Return Value Optimization (RVO)
// -----------------------------------------------------------------------------
// "Return by value" ist in C++ NICHT teuer. Der Compiler optimiert so, dass
// das Objekt DIREKT am Zielort konstruiert wird — keine Kopie.
// Seit C++17 ist das bei Prvalues GARANTIERT (guaranteed copy elision).
std::vector<int> make_data(std::size_t n)
{
    std::vector<int> v;
    v.reserve(n);
    {
        v.push_back(static_cast<int>(i));
    }

    return v; // keine Kopie

}

// Typische ANTI-Pattern (alte C-Schule): Output-Parameter statt Return.
// In modernem C++ NICHT NÖTIG — RVO macht den "Return by value" genauso schnell
// und der Code ist lesbarer.
void make_data_bad(std::size_t n, std::vector<int>& out)
{

    out.clear();
    out.reserve(n);
    for(std::size_t i=0; i<n; ++i){
        out.push_back(static_cast<int>(i));
    }

}

// -----------------------------------------------------------------------------
int main()
{

    int i = 0;

    int a = ++i; // a ist 1
    int b = i++; // b ist auch 1



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
    lambdas();

    // std::cout << "\n=== Überladung ===\n";
    // std::cout << max_of(3, 7)     << '\n';
    // std::cout << max_of(2.5, 1.8) << '\n';
    // connect("example.com");                 // port default 8080
    // connect("example.com", 443);

    // std::cout << "\n=== Lambdas ===\n";
    // lambdas();

    // std::cout << "\n=== constexpr ===\n";
    constexpr int v = square(7);        // zur Compile-Zeit ausgewertet
    std::cout << "square(7) = " << v << '\n';

    // std::cout << "\n=== RVO ===\n";
    // auto data = make_data(5);
    // std::cout << "make_data(5) ergab " << data.size() << " Elemente\n";
}
