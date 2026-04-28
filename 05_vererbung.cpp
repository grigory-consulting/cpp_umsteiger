// =============================================================================
// 05_vererbung.cpp
//
// Thema        : Vererbung, Polymorphismus, Slicing-Falle.
// Lernziel     : Korrekte Nutzung virtueller Funktionen, polymorphe Container,
//                häufige Fallstricke (Slicing, fehlender virtueller Destruktor).
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 05_vererbung.cpp -o 05_vererbung
// =============================================================================

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <numbers>

// -----------------------------------------------------------------------------
// Abschnitt A: Polymorphe Klassenhierarchie
// -----------------------------------------------------------------------------
// Regeln für polymorphe Basisklassen:
//   1) VIRTUELLER DESTRUKTOR — sonst: delete über Base-Pointer = UB
//   2) Mindestens eine virtual-Funktion → Klasse erhält eine vtable
//   3) override AUF JEDER überschreibenden Methode (Compiler checkt, ob
//      die Signatur tatsächlich eine Base-Methode überschreibt)
//   4) Optional: final verhindert weitere Ableitung oder weiteres Override

class Shape {
public:
    virtual ~Shape() = default;

    virtual double area() const = 0; // pure virtual
    
    virtual std::string name() const { return "Shape"; } // nicht-pure virtual, kann überschrieben werden
};

class Circle : public Shape {
    double r_;
public:
    explicit Circle(double r) : r_{r} {} // explicit verbietet Circle c = 1.0

    double area() const override {return std::numbers::pi * r_ * r_;}
    std::string name() const override { return "Circle"; }
};

class Square : public Shape {
    double side_;
public:
    explicit Square(double side) : side_{side} {}

    double area() const override {return side_ * side_;}
    std::string name() const override { return "Square"; }
    
};

// final: 'Triangle' kann keine weiteren Ableitungen haben.
class Triangle final : public Shape {
    double base_, height_;
public:
    Triangle(double b, double h ) : base_{b}, height_{h} {}

    double area() const override {return  0.5*base_* height_;}
    std::string name() const override { return "Triangle"; }
};

// -----------------------------------------------------------------------------
// Abschnitt B: Polymorphismus — dynamisches Dispatch
// -----------------------------------------------------------------------------
// Ein Container HETEROGENER Shape-Objekte ist nur über Pointer/Referenzen
// möglich (Objekte selbst sind unterschiedlich groß).
// unique_ptr<Shape> ist die Standard-Wahl — klares Ownership.
double total_area(const std::vector<std::unique_ptr<Shape>>& shapes)
{
    double total = 0;

    for (const auto& s : shapes){
        total += s->area();
    }
    
    return total;
}

// -----------------------------------------------------------------------------
// Abschnitt C: Slicing-Falle
// -----------------------------------------------------------------------------
// BY VALUE übergeben bedeutet KOPIEREN — und zwar NUR den Base-Teil.
// Der abgeleitete Teil wird "abgeschnitten" (sliced), Polymorphismus weg.
//
// Korrekt: per Referenz oder Pointer — Polymorphismus bleibt intakt.
void process_good(const Shape& s)
{
    std::cout << " " << s.name() << s.area() << "\n";
}

// HINWEIS: Eine Funktion `void process(Shape s)` lässt sich hier gar nicht
// erst kompilieren — Shape ist abstrakt. Der Compiler schützt uns in diesem
// Fall schon. Gefährlich wird Slicing erst mit konkreter Basis wie unten.

// Demo der Slicing-Falle mit nicht-abstrakter Basis
class Animal {
public:
    virtual ~Animal() = default;
    virtual std::string sound() const { return "..."; }
};

class Dog : public Animal {
public:
    std::string sound() const override { return "Wuff!"; }
};



void speak_by_value(Animal a)            // ⚠ slice — immer Animal::sound()
{
    std::cout << " " << a.sound() << "\n";
}

void speak_by_ref(const Animal& a)       // ✓ polymorph
{
    std::cout << " " << a.sound() << "\n";
}

// -----------------------------------------------------------------------------
int main()
{


    Circle c{2.0};
    Shape& s = c;
    Shape* p = &c;

    std::cout << c.Shape::name() << "\n";
    std::cout << s.name() << "\n";
    std::cout << p->Shape::name() << "\n";


//     std::cout << "=== Polymorphe Hierarchie ===\n";
     std::vector<std::unique_ptr<Shape>> shapes;
     shapes.push_back(std::make_unique<Circle>(1.0));
     shapes.push_back(std::make_unique<Square>(2.0));
     shapes.push_back(std::make_unique<Triangle>(3.0, 4.0));

     for (const auto& s : shapes) {
         process_good(*s);
     }
     std::cout << "Gesamtfläche: " << total_area(shapes) << '\n';

//     std::cout << "\n=== Slicing-Demo ===\n";
     Dog rex;
     speak_by_value(rex);                 // "..."       — slice!
     speak_by_ref(rex);                   // "Wuff!"     — korrekt
// 

}
