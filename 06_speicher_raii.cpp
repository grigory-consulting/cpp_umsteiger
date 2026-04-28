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
    
}

// -----------------------------------------------------------------------------
// Abschnitt B: RAII — Resource Acquisition Is Initialization
// -----------------------------------------------------------------------------
// Kernidee: Jede Ressource wird an die Lebensdauer eines OBJEKTS gebunden.
// Der Destruktor gibt die Ressource frei. Das funktioniert AUTOMATISCH —
// auch bei Exceptions (Stack-Unwinding ruft Destruktoren auf).
//
// File ist ein klassisches RAII-Beispiel: wir verpacken FILE* in ein Objekt.

class File {

    
};


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


// -----------------------------------------------------------------------------
int main()
{
    
}
