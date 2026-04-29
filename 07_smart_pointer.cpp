// =============================================================================
// 07_smart_pointer.cpp
//
// Thema        : Smart Pointer — unique_ptr, shared_ptr, weak_ptr.
// Lernziel     : Richtige Wahl des Smart-Pointer-Typs für Ownership-Semantik;
//                Vermeidung zyklischer Referenzen.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 07_smart_pointer.cpp -o 07_sp
// =============================================================================

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Eine kleine Demo-Klasse mit auffälligen Ctor/Dtor-Messages.
class Sensor {
public:
    explicit Sensor(std::string name) : name_{std::move(name)}
    {
        std::cout << "  + Sensor '" << name_ << "' konstruiert\n";
    }
    ~Sensor()
    {
        std::cout << "  - Sensor '" << name_ << "' zerstört\n";
    }

    const std::string& name() const noexcept { return name_; }

private:
    std::string name_;
};

// -----------------------------------------------------------------------------
// Abschnitt A: std::unique_ptr — exklusives Ownership
// -----------------------------------------------------------------------------
// Eigenschaften:
//   - GENAU EIN Besitzer, klare Lebensdauer
//   - NICHT kopierbar, aber MOVE-bar
//   - Kein Overhead gegenüber Raw-Pointer
//
// Faustregel: DEFAULT-Wahl für dynamisch erzeugte Einzelobjekte.
void demo_unique()
{
    std::cout << "--- unique_ptr ---\n";

    // make_unique (C++14) — vorzuziehen gegenüber `new`:
    //   * exception-sicher
    //   * weniger redundante Typnennung
    auto p = std::make_unique<Sensor>("temp-1");

    // Benutzen wie Raw-Pointer
    std::cout << "  name: " << p->name() << '\n';

    // Kopie NICHT erlaubt:
    //   auto q = p;                       // Compile-Fehler
    // Move ist erlaubt — p ist danach nullptr.
    auto q = std::move(p);

    // Reset gibt Ressource frei.
    q.reset();                             // Sensor wird hier zerstört.

    // In Container speichern — typisches Pattern für polymorphe Hierarchien.
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<Sensor>("temp-2"));
    sensors.push_back(std::make_unique<Sensor>("pressure-1"));
    // Alle werden am Ende des Scopes automatisch zerstört.
}

// -----------------------------------------------------------------------------
// Abschnitt B: std::shared_ptr — geteiltes Ownership mit Refcount
// -----------------------------------------------------------------------------
// Eigenschaften:
//   - Mehrere Besitzer, Refcount-basiert
//   - Atomare Refcount-Operationen → kleiner Overhead
//   - Objekt wird zerstört, wenn DER LETZTE shared_ptr verschwindet
//
// Einsatz NUR, wenn Ownership wirklich geteilt ist (Objekt wird von mehreren
// Stellen aus am Leben gehalten). Nicht als "Universal-Smart-Pointer" missbrauchen.
void demo_shared()
{
    std::cout << "--- shared_ptr ---\n";

    auto s1 = std::make_shared<Sensor>("shared-1");   // refcount = 1
    std::cout << "  use_count = " << s1.use_count() << '\n';

    {
        auto s2 = s1;                                  // refcount = 2
        std::cout << "  use_count = " << s1.use_count() << '\n';

        auto s3 = s1;                                  // refcount = 3
        std::cout << "  use_count = " << s1.use_count() << '\n';
    }   // s2, s3 zerstört → refcount = 1

    std::cout << "  use_count = " << s1.use_count() << '\n';
}   // s1 zerstört → Sensor wird freigegeben

// -----------------------------------------------------------------------------
// Abschnitt C: std::weak_ptr — beobachten ohne besitzen
// -----------------------------------------------------------------------------
// weak_ptr referenziert ein shared_ptr-Objekt, OHNE den Refcount zu erhöhen.
// Einsatzfälle:
//   - Caches (Objekt darf gelöscht werden, wenn keiner es sonst referenziert)
//   - Observer-Pattern (Listener überlebt Subject nicht künstlich)
//   - ZYKLEN AUFBRECHEN (parent ↔ child) — sonst Leak trotz shared_ptr!

struct Node {
    std::string            name;
    std::shared_ptr<Node>  child;
    std::weak_ptr<Node>    parent;   // weak! — bricht den Zyklus

    explicit Node(std::string n) : name{std::move(n)}
    {
        std::cout << "  + Node '" << name << "'\n";
    }
    ~Node()
    {
        std::cout << "  - Node '" << name << "'\n";
    }
};

void demo_weak()
{
    std::cout << "--- weak_ptr / Zyklus-Vermeidung ---\n";
    auto parent = std::make_shared<Node>("parent");
    auto child  = std::make_shared<Node>("child");

    parent->child  = child;        // parent hält child als shared_ptr
    child ->parent = parent;       // child  hält parent nur als weak_ptr

    // Zugriff auf parent über weak_ptr → muss mit lock() in shared_ptr gehoben werden.
    if (auto p = child->parent.lock()) {
        std::cout << "  child sieht parent: " << p->name << '\n';
    } else {
        std::cout << "  parent bereits zerstört\n";
    }
    // Am Scope-Ende: parent-shared-count geht 1→0, parent wird zerstört,
    // dadurch child-shared-count 1→0, child wird zerstört.
    // WÄRE parent auch shared_ptr in child, hätten wir einen Zyklus → Leak.
}

// -----------------------------------------------------------------------------
// Abschnitt D: Smart Pointer in Funktionsparametern
// -----------------------------------------------------------------------------
// Entscheidungstabelle:
//   - Nimm unique_ptr<T> als Parameter, wenn die Funktion den Besitz ÜBERNIMMT
//   - Nimm shared_ptr<T>, wenn die Funktion den Besitz TEILT (und behält)
//   - Nimm T* oder const T& für reinen Beobachter-Zugriff (KEIN Smart Pointer!)

// Observer — nimmt den Sensor nur zum Lesen, Ownership bleibt beim Aufrufer.
void print_sensor(const Sensor& s)
{
    std::cout << "  observing: " << s.name() << '\n';
}

// Sink — übernimmt den Besitz.
void consume(std::unique_ptr<Sensor> s)
{
    std::cout << "  consuming: " << s->name() << '\n';
    // s wird am Ende zerstört.
}

// -----------------------------------------------------------------------------
int main()
{
    demo_unique();
    std::cout << '\n';
    demo_shared();
    std::cout << '\n';
    demo_weak();

    std::cout << "\n--- Parameter-Übergabe ---\n";
    auto s = std::make_unique<Sensor>("transfer");
    print_sensor(*s);                  // nur beobachten
    consume(std::move(s));             // Besitz übergeben — s ist danach leer
}
