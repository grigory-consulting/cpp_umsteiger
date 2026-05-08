// =============================================================================
// 07b_smart_pointer_uebung_loesung.cpp
//
// Thema        : Smart Pointer — LÖSUNGEN zu 07b_smart_pointer_uebung.cpp
// Kompilieren  : g++ -std=c++20 -Wall -Wextra
//                07b_smart_pointer_uebung_loesung.cpp -o 07b_loesung
// =============================================================================

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// Demo-Hierarchie — wie in der Übung.
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
// Dispatch über String. std::unique_ptr<Sensor> erlaubt polymorphe Rückgabe —
// der konkrete Typ steckt in der vtable, der Aufrufer sieht nur Sensor.
//
// NRVO: kein std::move beim return einer lokalen unique_ptr nötig — die
// Sprache erlaubt hier impliziten Move.
std::unique_ptr<Sensor> make_sensor(const std::string& kind, std::string name)
{
    if (kind == "temp")     return std::make_unique<Temperature>(std::move(name));
    if (kind == "pressure") return std::make_unique<Pressure>(std::move(name));
    return nullptr;                  // unbekannter Typ → leerer Smart-Pointer
}

// -----------------------------------------------------------------------------
// Aufgabe 2: Vector polymorpher Smart Pointer
// -----------------------------------------------------------------------------
// emplace_back vermeidet das temporäre unique_ptr — make_unique direkt rein.
// Iteration mit `const auto&` → keine Kopie (unique_ptr ist nicht kopierbar
// und der Compiler würde es sowieso ablehnen).
void demo_polymorph_vector()
{
    std::cout << "--- Aufgabe 2: polymorpher Vector ---\n";

    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.emplace_back(std::make_unique<Temperature>("t-1"));
    sensors.emplace_back(std::make_unique<Pressure>("p-1"));
    sensors.emplace_back(std::make_unique<Temperature>("t-2"));

    for (const auto& s : sensors) {
        std::cout << "    " << s->name() << " = " << s->read() << '\n';
    }
    // Am Scope-Ende: vector zerstört → unique_ptrs zerstört → Sensoren freigegeben.
}

// -----------------------------------------------------------------------------
// Aufgabe 3: shared_ptr — Refcount beobachten
// -----------------------------------------------------------------------------
// use_count() verrät den Refcount. Vorsicht: in echtem Multithread-Code ist der
// Wert nur ein Snapshot — hier didaktisch ok.
void demo_refcount()
{
    std::cout << "--- Aufgabe 3: shared_ptr Refcount ---\n";

    auto s1 = std::make_shared<Temperature>("shared");
    std::cout << "  use_count = " << s1.use_count() << '\n';     // 1

    {
        auto s2 = s1;
        std::cout << "  use_count = " << s1.use_count() << '\n'; // 2

        {
            auto s3 = s1;
            std::cout << "  use_count = " << s1.use_count() << '\n'; // 3
        }   // s3 zerstört
        std::cout << "  use_count = " << s1.use_count() << '\n'; // 2
    }       // s2 zerstört
    std::cout << "  use_count = " << s1.use_count() << '\n';     // 1
}           // s1 zerstört → Sensor wird freigegeben

// -----------------------------------------------------------------------------
// Aufgabe 4: Zyklus auflösen mit weak_ptr
// -----------------------------------------------------------------------------
// Lösung: parent als std::weak_ptr<Node>.
//   - parent hält child als shared_ptr (Eigentümer-Beziehung von oben nach unten)
//   - child  hält parent als weak_ptr  (nur Beobachten, kein Eigentum)
//   → Refcounts kommen sauber auf 0 → kein Leak.
struct Node {
    std::string             name;
    std::shared_ptr<Node>   child;
    std::weak_ptr<Node>     parent;   // ✓ FIX: weak statt shared

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

    a->child  = b;       // a hält b per shared_ptr
    b->parent = a;       // b beobachtet a per weak_ptr — kein Refcount++

    // Optional: Rückwärts-Zugriff demonstrieren
    if (auto p = b->parent.lock()) {
        std::cout << "  b sieht parent: " << p->name << '\n';
    }
    // Am Scope-Ende: a-Refcount 1 → 0 (b->parent ist weak), a wird zerstört.
    //                Damit verschwindet a->child → b-Refcount 1 → 0 → b zerstört.
}

// -----------------------------------------------------------------------------
// Aufgabe 5 (Stretch): die richtige Übergabeform
// -----------------------------------------------------------------------------

// 5a) take_ownership — übernimmt: by-value unique_ptr.
//     Aufrufer muss std::move() schreiben → Ownership-Transfer ist im Code sichtbar.
void take_ownership(std::unique_ptr<Sensor> s)
{
    std::cout << "  übernommen: " << s->name() << '\n';
    // s wird am Scope-Ende zerstört → Sensor freigegeben.
}

// 5b) observe — nur lesen, kein Smart Pointer im Parameter:
//     const Sensor& ist kürzer, klarer und akzeptiert auch lokal-Stack-Sensoren.
//     (Niemals shared_ptr<Sensor> für reine Beobachtung — das verlangt unnötige
//      atomare Refcount-Operationen.)
void observe(const Sensor& s)
{
    std::cout << "  beobachte: " << s.name() << " = " << s.read() << '\n';
}

// 5c) share_with_log — fügt sich in die geteilte Eigentümerschaft ein:
//     by-value shared_ptr (nicht const-Ref!) — weil wir den Refcount tatsächlich
//     erhöhen wollen, solange die Funktion läuft.
void share_with_log(std::shared_ptr<Sensor> s)
{
    std::cout << "  geteilte Verwendung: " << s->name()
              << " (use_count=" << s.use_count() << ")\n";
}

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== Aufgabe 1: Factory ===\n";
    auto t = make_sensor("temp",     "t-1");
    auto p = make_sensor("pressure", "p-1");
    auto x = make_sensor("foo",      "x-?");        // unbekannt → nullptr
    if (t) std::cout << "  " << t->name() << " = " << t->read() << '\n';
    if (p) std::cout << "  " << p->name() << " = " << p->read() << '\n';
    if (!x) std::cout << "  unknown kind → nullptr (erwartet)\n";

    std::cout << "\n";
    demo_polymorph_vector();

    std::cout << "\n";
    demo_refcount();

    std::cout << "\n";
    demo_cycle();
    // Erwartete Ausgabe nach dem Fix:
    //   + Node 'A'
    //   + Node 'B'
    //   b sieht parent: A
    //   - Node 'A'
    //   - Node 'B'

    std::cout << "\n--- Stretch: Parameter-Formen ---\n";
    auto s1 = std::make_shared<Temperature>("stretch");
    observe(*s1);                           // 5b: nur lesen
    share_with_log(s1);                     // 5c: shared lifetime
    std::cout << "  use_count nach share_with_log = " << s1.use_count() << '\n';

    auto s2 = std::make_unique<Temperature>("sink");
    take_ownership(std::move(s2));          // 5a: Sink übernimmt
    if (!s2) std::cout << "  s2 ist nach Move nullptr (erwartet)\n";
}
