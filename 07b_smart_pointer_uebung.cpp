// =============================================================================
// 07b_smart_pointer_uebung.cpp
//
// Thema        : Smart Pointer — Übungsaufgaben.
// Lernziel     : unique_ptr / shared_ptr / weak_ptr richtig einsetzen,
//                Ownership-Entscheidungen treffen, Zyklen vermeiden.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 07b_smart_pointer_uebung.cpp -o 07b_uebung
// =============================================================================

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// Demo-Hierarchie für die Aufgaben — nicht editieren.
// -----------------------------------------------------------------------------
class Sensor {
public:
    explicit Sensor(std::string name) : name_{std::move(name)} {
        std::cout << "  + Sensor '" << name_ << "'\n";
    }
    virtual ~Sensor() {
        std::cout << "  - Sensor '" << name_ << "'\n";
    }
    virtual double read() const = 0;
    const std::string& name() const noexcept { return name_; }
protected:
    std::string name_;
};

class Temperature : public Sensor {
public:
    using Sensor::Sensor;
    double read() const override { return 21.5; }
};

class Pressure : public Sensor {
public:
    using Sensor::Sensor;
    double read() const override { return 1013.25; }
};

// -----------------------------------------------------------------------------
// Aufgabe 1: Factory mit make_unique
// -----------------------------------------------------------------------------
// Schreibe make_sensor(kind, name), die je nach String "temp" oder "pressure"
// einen passenden Sensor als std::unique_ptr<Sensor> zurückgibt. Bei
// unbekanntem Typ: leerer unique_ptr (nullptr).
//
// Tipps:
//   - std::make_unique<Temperature>(name) bzw. <Pressure>(name)
//   - Rückgabe: std::unique_ptr<Sensor> — Polymorphie funktioniert
//   - Kein std::move beim return einer lokalen unique_ptr nötig (NRVO)
std::unique_ptr<Sensor> make_sensor(const std::string& kind, std::string name)
{
    // TODO
    return nullptr;
}

// -----------------------------------------------------------------------------
// Aufgabe 2: Vector polymorpher Smart Pointer
// -----------------------------------------------------------------------------
// Lege einen std::vector<std::unique_ptr<Sensor>> an, fülle ihn mit drei
// verschiedenen Sensoren und gib für jeden Namen + Wert aus.
//
// Tipps:
//   - emplace_back(std::make_unique<Temperature>("t1")) — direkter Konstrukt
//   - Iteration mit `for (const auto& s : sensors)` → s ist const unique_ptr&
//   - Aufruf: s->read(), s->name()
void demo_polymorph_vector()
{
    std::cout << "--- Aufgabe 2: polymorpher Vector ---\n";
    // TODO
}

// -----------------------------------------------------------------------------
// Aufgabe 3: shared_ptr — Refcount beobachten
// -----------------------------------------------------------------------------
// Lege einen shared_ptr an, kopiere ihn in mehrere Variablen, und gib nach
// jeder Aktion s.use_count() aus. Beobachte:
//   - Wann steigt der Count?
//   - Wann fällt er wieder?
//
// Tipps:
//   - std::make_shared<Temperature>("shared")
//   - Verschachtelte Scopes mit { ... } fürs Beobachten von Refcount-Drops
void demo_refcount()
{
    std::cout << "--- Aufgabe 3: shared_ptr Refcount ---\n";
    // TODO
}

// -----------------------------------------------------------------------------
// Aufgabe 4: Den Bug fixen — Memory Leak durch Zyklus
// -----------------------------------------------------------------------------
// Der folgende Code hat einen ZYKLUS — Parent und Child halten sich
// gegenseitig per shared_ptr → ihre Refcounts kommen nie auf 0 → LEAK.
//
// Aufgabe: ändere EINEN der beiden Pointer-Typen so, dass kein Leak entsteht.
// Hinweis: weak_ptr<...> bricht den Zyklus.
//
// Wenn richtig gefixt, sehen wir am Programm-Ende beide Destruktoren laufen.
struct Node {
    std::string             name;
    std::shared_ptr<Node>   child;
    std::shared_ptr<Node>   parent;   // ⚠ TODO: ändere diesen Typ
                                      //         um den Zyklus aufzubrechen

    explicit Node(std::string n) : name{std::move(n)} {
        std::cout << "  + Node '" << name << "'\n";
    }
    ~Node() {
        std::cout << "  - Node '" << name << "'\n";
    }
};

void demo_cycle()
{
    std::cout << "--- Aufgabe 4: Zyklus aufbrechen ---\n";
    auto a = std::make_shared<Node>("A");
    auto b = std::make_shared<Node>("B");

    a->child  = b;       // a hält b
    b->parent = a;       // ⚠ b hält a → Zyklus, wenn beide shared_ptr sind

    // Erwartete Ausgabe nach Fix:
    //   + Node 'A'
    //   + Node 'B'
    //   - Node 'A'
    //   - Node 'B'
    // Ohne Fix: keine Destruktor-Ausgabe → Leak.
}

// -----------------------------------------------------------------------------
// Aufgabe 5 (Stretch): Funktions-Parameter — die richtige Übergabeform wählen
// -----------------------------------------------------------------------------
// Drei Funktionen unten — schreibe für jede den korrekten Smart-Pointer-/
// Reference-Parameter. Hinweise stehen jeweils im Kommentar.
//
// 5a) take_ownership — übernimmt den Besitz und behält ihn
// void take_ownership( /* TODO */ );

// 5b) observe — liest den Sensor nur, Ownership bleibt beim Aufrufer
// void observe( /* TODO */ );

// 5c) share_with_log — fügt sich in die geteilte Eigentümerschaft ein
//                      (verlängert die Lebenszeit, solange diese Funktion läuft)
// void share_with_log( /* TODO */ );

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Aufgabe 1: Factory ===\n";
    // auto t = make_sensor("temp",     "t-1");
    // auto p = make_sensor("pressure", "p-1");
    // if (t) std::cout << "  " << t->name() << " = " << t->read() << '\n';
    // if (p) std::cout << "  " << p->name() << " = " << p->read() << '\n';

    std::cout << "\n";
    demo_polymorph_vector();

    std::cout << "\n";
    demo_refcount();

    std::cout << "\n";
    demo_cycle();
    // Nach dem Fix: hier müssen die Destruktoren von A und B laufen.
}
