#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <vector>

// -----------------------------------------------------------------------------
// Aufgabe 1: Statistik
// -----------------------------------------------------------------------------
// std::minmax_element gibt ein Paar {min_it, max_it} in einem Durchgang zurück.
// Das ist effizienter als zwei separate Aufrufe von min_element / max_element.
struct Stats { double min, max, mean, median; };

Stats compute_stats(std::vector<double> temps)  // by value — wir sortieren lokal
{
    

}

// -----------------------------------------------------------------------------
// Aufgabe 2: Filter — Werte über Schwellwert
// -----------------------------------------------------------------------------
// std::copy_if + std::back_inserter ist das Standard-Pattern für Filterung.
// Die Lambda captured den Schwellwert per Copy — dadurch sehr billig.
std::vector<double> filter_above(const std::vector<double>& temps, double thr)
{
    
}

// -----------------------------------------------------------------------------
// Aufgabe 3: Binning — Histogramm in 10er-Buckets
// -----------------------------------------------------------------------------
// std::map ist sortiert, daher kommen die Buckets automatisch in aufsteigender
// Reihenfolge — ideal für Ausgabe.
std::map<int, int> histogram(const std::vector<double>& temps)
{
    
}

// -----------------------------------------------------------------------------
// Aufgabe 4: Top-3 — die drei höchsten Werte
// -----------------------------------------------------------------------------
// std::partial_sort_copy: sortiert die ersten K Werte in den Zielbereich —
// ohne den Quellbereich zu verändern. std::greater<>{} dreht die Ordnung um.
std::vector<double> top_three(const std::vector<double>& temps)
{
    
}

// -----------------------------------------------------------------------------
// Aufgabe 5 (Stretch): Ausreißer via Z-Score entfernen
// -----------------------------------------------------------------------------
// Z-Score = (x - Mittelwert) / Standardabweichung
// Werte mit |Z| > threshold (meist 3) gelten als Ausreißer.
std::vector<double> remove_outliers(std::vector<double> temps, double z_thr = 3.0)
{
    
}

// -----------------------------------------------------------------------------
int main()
{
    std::vector<double> temps{
        22.1, 23.4, 21.8, 24.5, 25.1, 23.9,
        31.2, 35.6, 28.4, 27.3, 29.1,
        22.5, 23.0, 24.0, 80.0 /* Ausreißer */
    };

    // auto s = compute_stats(temps);
    // std::cout << "Stats — min=" << s.min << " max=" << s.max
    //           << " mean=" << s.mean << " median=" << s.median << '\n';

    // auto hot = filter_above(temps, 25.0);
    // std::cout << "Werte > 25°C:";
    // for (double x : hot) std::cout << ' ' << x;
    // std::cout << '\n';

    // std::cout << "Histogramm:\n";
    // for (const auto& [bucket, count] : histogram(temps)) {
    //     std::cout << "  [" << bucket << ", " << bucket + 10 << "): " << count << '\n';
    // }

    // auto top = top_three(temps);
    // std::cout << "Top-3:";
    // for (double x : top) std::cout << ' ' << x;
    // std::cout << '\n';

    // auto clean = remove_outliers(temps);
    // std::cout << "Nach Ausreißer-Entfernung: " << clean.size()
    //           << " Werte (von " << temps.size() << ")\n";
}
