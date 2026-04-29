// =============================================================================
// 09_exceptions.cpp
//
// Thema        : Exception Handling.
// Lernziel     : try/catch, eigene Exception-Klassen, noexcept, Exception-Safety
//                Garantien.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra 09_exceptions.cpp -o 09_exc
// =============================================================================

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// Abschnitt A: Eigene Exception-Klasse
// -----------------------------------------------------------------------------
// Best Practices:
//   - Von std::exception oder (besser) einer passenden Unterklasse erben
//     (std::runtime_error, std::logic_error, std::invalid_argument, ...)
//   - Zusätzliche Kontextinformationen als Member speichern
//
// Unterschied logic_error vs runtime_error:
//   - logic_error   : Bug im Programm (falsche Vorbedingung, falsche Nutzung)
//   - runtime_error : Laufzeit-Problem außerhalb der Kontrolle (I/O, Netz)

class SensorError : public std::runtime_error {
public:
    SensorError(std::string sensor, std::string message)
        : std::runtime_error{std::move(message)}
        , sensor_{std::move(sensor)}
    {}

    const std::string& sensor() const noexcept { return sensor_; }

private:
    std::string sensor_;
};

// -----------------------------------------------------------------------------
// Abschnitt B: Werfen und Fangen
// -----------------------------------------------------------------------------
// Regeln:
//   - throw by VALUE, catch by REFERENCE (const-ref)
//   - Reihenfolge der catch-Blöcke: von SPEZIFISCH nach ALLGEMEIN
//   - catch(...) fängt alles — in der Regel nur am äußersten Rand sinnvoll

double read_temperature(const std::string& sensor, bool simulate_failure)
{
    if (sensor.empty())
        throw std::invalid_argument{"Sensor-Name darf nicht leer sein"};
    if (simulate_failure)
        throw SensorError{sensor, "Kalibrierung verloren"};
    return 23.5;
}

void demo_try_catch()
{
    try {
        double t = read_temperature("temp-1", true);
        std::cout << "T = " << t << '\n';
    }
    catch (const SensorError& e) {            // spezifisch zuerst
        std::cout << "Sensor-Fehler an '" << e.sensor()
                  << "': " << e.what() << '\n';
    }
    catch (const std::invalid_argument& e) {
        std::cout << "Ungültiges Argument: " << e.what() << '\n';
    }
    catch (const std::exception& e) {         // Basisklasse
        std::cout << "Generischer Fehler: " << e.what() << '\n';
    }
    catch (...) {                             // unbekannter Typ
        std::cout << "Unbekannte Exception\n";
        throw;                                // ggf. weiterwerfen
    }
}

// -----------------------------------------------------------------------------
// Abschnitt C: noexcept
// -----------------------------------------------------------------------------
// noexcept markiert Funktionen, die GARANTIERT nicht werfen.
// Nutzen:
//   1) Dokumentation — Aufrufer weiß: keine Exception möglich
//   2) Optimierung — STL-Container nutzen bei noexcept-Move den Move-Pfad
//      statt Copy (z. B. vector-Realloc)
//   3) Destruktoren sind IMPLIZIT noexcept — Werfen aus Dtor wird terminate()
//
// Warnung: Wenn eine als noexcept markierte Funktion doch wirft, wird
// std::terminate() aufgerufen — das Programm stirbt.

class Widget {
public:
    // Move-Operationen noexcept → std::vector<Widget> kann bei realloc
    // effizient bewegen statt kopieren.
    Widget(Widget&&) noexcept            = default;
    Widget& operator=(Widget&&) noexcept = default;

    // triviale Getter typischerweise noexcept
    int value() const noexcept { return value_; }

private:
    int value_ = 0;
};

// -----------------------------------------------------------------------------
// Abschnitt D: Exception-Safety-Garantien
// -----------------------------------------------------------------------------
//
//  No-throw     : wirft niemals (→ noexcept)
//  Strong       : Commit-or-Rollback — bei Wurf bleibt der Zustand unverändert
//  Basic        : Invarianten erhalten, keine Leaks, aber Zustand kann sich ändern
//  No guarantee : undefiniert — VERMEIDEN
//
// Basic Guarantee ist das MINIMUM in produktivem Code — RAII garantiert sie
// quasi automatisch.
//
// Beispiel für Strong Guarantee — Copy-and-Swap-Idiom:

class Registry {
public:
    // Strong Guarantee: bei Exception bleibt this in altem Zustand.
    // Trick: erst neues Objekt fertig bauen, DANN tauschen — swap wirft nie.
    void replace(std::vector<std::string> new_items)
    {
        // Wenn move-assign wirft (hier: tut es nicht), bleibt items_ unberührt.
        // Variante, die explizit den Copy-and-Swap-Idiom nutzt:
        std::vector<std::string> tmp = std::move(new_items);  // kein Wurf
        items_.swap(tmp);                                      // noexcept
        // tmp (alter Inhalt) wird am Scope-Ende zerstört.
    }

    std::size_t size() const noexcept { return items_.size(); }

private:
    std::vector<std::string> items_;
};

// -----------------------------------------------------------------------------
// Abschnitt E: Niemals aus dem Destruktor werfen
// -----------------------------------------------------------------------------
// Wenn während des Stack-Unwinding (Exception-Verarbeitung) ein weiterer Dtor
// wirft → std::terminate(). Daher: Destruktoren müssen Exceptions INTERN
// behandeln, oder Operationen vermeiden, die werfen könnten.

class LogHandle {
public:
    ~LogHandle()
    {
        try {
            flush();
        } catch (...) {
            // Fehler loggen, aber NICHT weiterwerfen.
            // In der Praxis: Error-Code irgendwo hinterlegen, ggf. spätere
            // Prüfung durch den Aufrufer.
        }
    }
private:
    void flush() { /* kann werfen */ }
};

// -----------------------------------------------------------------------------
int main()
{
    std::cout << "=== try / catch ===\n";
    demo_try_catch();

    std::cout << "\n=== Exception-Safety: Registry ===\n";
    Registry r;
    r.replace({"a", "b", "c"});
    std::cout << "Registry-Größe: " << r.size() << '\n';

    std::cout << "\n=== Invalid-argument-Pfad ===\n";
    try {
        read_temperature("", false);
    } catch (const std::exception& e) {
        std::cout << "erwartet: " << e.what() << '\n';
    }
}
