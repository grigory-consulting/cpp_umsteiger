// =============================================================================
// 11_multithreading.cpp
//
// Thema        : Multithreading — Einführung.
// Lernziel     : std::thread, Mutex mit RAII-Locks, std::async, std::atomic.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra -pthread 11_multithreading.cpp \
//                    -o 11_mt
// =============================================================================

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// -----------------------------------------------------------------------------
// Abschnitt A: std::thread — die Basis
// -----------------------------------------------------------------------------
// Wichtig: Ein thread muss entweder JOINED oder DETACHED werden, sonst ruft
// sein Destruktor std::terminate(). Seit C++20 gibt es std::jthread, der im
// Destruktor automatisch joined — das vermeidet diese Falle.

void worker(int id, int iterations)
{
    for (int i = 0; i < iterations; ++i) {
        std::cout << "  Thread " << id << " iter " << i << '\n';
        std::this_thread::sleep_for(50ms);
    }
}

void demo_basic_threads()
{
    std::cout << "--- std::thread ---\n";
    std::thread t1{worker, 1, 3};
    std::thread t2{worker, 2, 3};

    t1.join();                       // blockiert, bis t1 fertig
    t2.join();
}

// -----------------------------------------------------------------------------
// Abschnitt B: Mutex mit RAII-Locks
// -----------------------------------------------------------------------------
// REGEL: Niemals mutex.lock() / mutex.unlock() manuell — bei Exception zwischen
// lock und unlock bleibt der Mutex gelockt (Deadlock). Immer RAII-Wrapper:
//   - std::lock_guard   (C++11) — einfachster Fall, nicht bewegbar
//   - std::unique_lock  (C++11) — flexibler (delayed lock, manuelles unlock)
//   - std::scoped_lock  (C++17) — kann MEHRERE Mutexe deadlock-frei locken

class ThreadSafeCounter {
public:
    void increment()
    {
        std::lock_guard<std::mutex> lk{m_};      // RAII — unlock im Dtor
        ++value_;
    }

    int get() const
    {
        std::lock_guard<std::mutex> lk{m_};
        return value_;
    }

private:
    // mutable, damit const-Methoden (get) den Mutex locken dürfen.
    mutable std::mutex m_;
    int                value_ = 0;
};

void demo_mutex()
{
    std::cout << "--- Mutex ---\n";
    ThreadSafeCounter counter;

    // 10 Threads erhöhen den Counter je 1000-mal. Ohne Mutex hätten wir eine
    // Data Race — mit Mutex ist das Ergebnis deterministisch 10'000.
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter] {
            for (int k = 0; k < 1000; ++k) counter.increment();
        });
    }
    for (auto& t : threads) t.join();

    std::cout << "  final count = " << counter.get() << " (erwartet: 10000)\n";
}

// -----------------------------------------------------------------------------
// Abschnitt C: std::atomic — lock-free
// -----------------------------------------------------------------------------
// Für EINFACHE Werte (int, bool, Pointer) ist std::atomic schneller als Mutex.
// Operationen sind atomar, ohne Lock-Overhead.
void demo_atomic()
{
    std::cout << "--- std::atomic ---\n";
    std::atomic<int> counter{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter] {
            for (int k = 0; k < 1000; ++k) ++counter;   // atomar
        });
    }
    for (auto& t : threads) t.join();

    std::cout << "  atomic count = " << counter.load() << '\n';
}

// -----------------------------------------------------------------------------
// Abschnitt D: std::async und std::future
// -----------------------------------------------------------------------------
// Höhere Abstraktion als rohe threads. Liefert ein Ergebnis zurück
// und propagiert Exceptions automatisch.
//
// std::launch::async        → läuft GARANTIERT in eigenem Thread
// std::launch::deferred     → läuft erst beim get() (lazy, im Caller-Thread)
// default (beides)          → Implementierung entscheidet — oft unerwartet,
//                             daher meist besser explizit async angeben.

long long sum_range(long long from, long long to)
{
    long long s = 0;
    for (long long i = from; i < to; ++i) s += i;
    return s;
}

void demo_async()
{
    std::cout << "--- std::async ---\n";

    // Parallele Summation zweier Hälften.
    auto f1 = std::async(std::launch::async, sum_range, 0LL,      500'000LL);
    auto f2 = std::async(std::launch::async, sum_range, 500'000LL, 1'000'000LL);

    long long total = f1.get() + f2.get();   // blockiert bis fertig
    std::cout << "  Summe [0, 1'000'000) = " << total << '\n';
}

// -----------------------------------------------------------------------------
int main()
{
    demo_basic_threads();
    std::cout << '\n';
    demo_mutex();
    std::cout << '\n';
    demo_atomic();
    std::cout << '\n';
    demo_async();
}
