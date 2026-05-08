// =============================================================================
// 06_speicher_raii.cpp
//
// Thema        : Dynamische Speicherverwaltung & RAII.
// Lernziel     : Warum new/delete fehleranfällig ist, wie RAII das Problem löst,
//                Rule of 0 / 3 / 5.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 06_speicher_raii.cpp -o 06_raii
// =============================================================================

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// -----------------------------------------------------------------------------
// Abschnitt A: Warum new/delete problematisch ist
// -----------------------------------------------------------------------------
// Typische Probleme:
//   - Leak: delete vergessen
//   - Double-Free: zweimal delete
//   - Dangling: nach delete noch dereferenzieren
//   - delete vs delete[]: falsch gepaart
//   - Exception-Unsicherheit: Wurf zwischen new und delete → Leak
//
// Beispiel für Exception-Unsicherheit:
void exception_unsafe()
{
    int* p = new int[100];

    // wenn
    // may_throw();

    delete[] p; // könnte nicht erreichbar sein
}

// -----------------------------------------------------------------------------
// Abschnitt B: RAII — Resource Acquisition Is Initialization
// -----------------------------------------------------------------------------
// Kernidee: Jede Ressource wird an die Lebensdauer eines OBJEKTS gebunden.
// Der Destruktor gibt die Ressource frei. Das funktioniert AUTOMATISCH —
// auch bei Exceptions (Stack-Unwinding ruft Destruktoren auf).
//
// File ist ein klassisches RAII-Beispiel: wir verpacken FILE* in ein Objekt.

//class File {

    
//};


// -----------------------------------------------------------------------------
// Abschnitt C: Rule of 0 / 3 / 5
// -----------------------------------------------------------------------------
//
//  Rule of 0  : Nutze Typen, die ihre Ressourcen selbst verwalten (vector,
//               unique_ptr, string, ...). Schreibe KEINE Spezialmember selbst.
//               → Bevorzugte Variante, wann immer möglich.
//
//  Rule of 3  : Wenn du EINEN von {Dtor, Copy-Ctor, Copy-Assign} schreibst,
//               brauchst du meistens alle drei (C++98).
//
//  Rule of 5  : Erweitert Rule of 3 um Move-Ctor und Move-Assign (C++11).
//
// Beispiel für Rule of 0: dieselbe Klasse wie File, nur ohne manuelle Verwaltung.
// Hier hält std::unique_ptr mit custom Deleter den FILE* — Compiler macht den
// Rest automatisch korrekt.

// Deklaration des Deleters für fclose als Funktor-Typ — wird Smart-Pointer-
// Template-Parameter. Alternative: Lambda-Deleter, aber Typen werden umständlich.


// -----------------------------------------------------------------------------
// Abschnitt D: Rule of 5 an einem eigenen Buffer-Typ
// -----------------------------------------------------------------------------
// Pädagogisch wichtig — in der Praxis würde man std::vector<char> nehmen.


class Buffer{
public:
    explicit Buffer(std::size_t n)
        : data_{new char[n]}, size_{n}
    {
        std::cout << " Buffer" << n << " konstruiert";
    }

    // Destruktor
    ~Buffer()
    {
        delete[] data_;
        std::cout << " Buffer" << " zerstört";

    }

    // Copy-Ctor ... Copy Constructor
    // Buffer a{8}
    // Buffer b{a} <- Copy Constructor

    //Buffer(const Buffer& a)
    //    : data_{new char[a.size_]}, size_{a.size_}
    //{
    //    std::copy_n(a.data_, size_, data_);
    //}
    Buffer(const Buffer& a) = delete; // Kopieren verboten

    // Copy-Assign
    // Buffer a{8}
    // Buffer b{8}
    // b = a;

    // Buffer& operator=(const Buffer& other){
    //     if (this != &other) {               // Selbst-Zuweisung abfangen
    //         char* new_data = new char[other.size_];  // zuerst allokieren …
    //         std::copy_n(other.data_, other.size_, new_data);
    //         delete[] data_;                           // … dann alten freigeben
    //         data_ = new_data;
    //         size_ = other.size_;
    //     }
    //     std::cout << "  Buffer copy-assign\n";
    //     return *this;
    // }

    Buffer& operator=(const Buffer& other) = delete;

    // Move-Ctor
    // Buffer a{8}
    // Buffer b = std::move(a);
    Buffer(Buffer&& other) noexcept : data_{other.data_}, size_{other.size_}
    {
        other.data_ = nullptr;
        other.size_ = 0;

    }


    // Move-Assign
    // Buffer a{8}
    // Buffer b{8}
    // b  = std::move(a);
    Buffer& operator=(Buffer&& other) = delete;

private:
    char* data_ = nullptr;
    std::size_t size_ = 0;
};



// -----------------------------------------------------------------------------
int main()
{
    Buffer a{1024};
    Buffer b{a};                      // Copy-Ctor
    Buffer c{std::move(a)};           // Move-Ctor — a ist danach "leer"
    b = c;                            // Copy-Assign
    b = std::move(c);                 // Move-Assign
    
}
