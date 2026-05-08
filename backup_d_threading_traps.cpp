// =============================================================================
// backup_d_threading_traps.cpp
//
// Thema        : Multithreading-Fallstricke — Deadlocks, Condition-Variables.
// Lernziel     : Typische Fehler erkennen und richtig vermeiden.
// Kompilieren  : g++ -std=c++20 -Wall -Wextra -pthread \
//                    backup_d_threading_traps.cpp -o backup_d
//
// Hinweis: Die Deadlock-Demo ist standardmäßig AUSKOMMENTIERT, sonst hängt das
// Programm. Zum Ausprobieren #define DEMO_DEADLOCK unkommentieren.
// =============================================================================

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// -----------------------------------------------------------------------------
// Abschnitt A: Deadlock durch unterschiedliche Lock-Reihenfolge
// -----------------------------------------------------------------------------
// Zwei Threads locken zwei Mutexe in ENTGEGENGESETZTER Reihenfolge.
// Thread A: hat m1, wartet auf m2.
// Thread B: hat m2, wartet auf m1.
// → Keiner kommt weiter. Deadlock.
//
// Vermeidung:
//   1) KONSISTENTE Reihenfolge überall einhalten, oder
//   2) std::scoped_lock (C++17) — lockt MEHRERE Mutexe atomar und deadlock-frei.

// #define DEMO_DEADLOCK     // Unkommentieren, um es auszuprobieren (hängt!)

#ifdef DEMO_DEADLOCK
namespace deadlock_demo {

std::mutex m1, m2;

void thread_a()
{
    std::lock_guard lk1{m1};
    std::this_thread::sleep_for(50ms);       // Zeit geben, damit B auch m2 nimmt
    std::lock_guard lk2{m2};
    std::cout << "A: hat beide\n";
}

void thread_b()
{
    std::lock_guard lk2{m2};
    std::this_thread::sleep_for(50ms);
    std::lock_guard lk1{m1};                 // ⚠ DEADLOCK
    std::cout << "B: hat beide\n";
}

void run()
{
    std::thread a{thread_a};
    std::thread b{thread_b};
    a.join(); b.join();
}

} // namespace deadlock_demo
#endif

// -----------------------------------------------------------------------------
// Abschnitt B: Richtige Lösung — std::scoped_lock
// -----------------------------------------------------------------------------
// scoped_lock (C++17) nutzt intern std::lock, das einen Deadlock-freien
// Locking-Algorithmus implementiert. Die Reihenfolge der Argumente ist egal.

struct Account {
    std::mutex m;
    int        balance = 100;
};

void transfer(Account& from, Account& to, int amount)
{
    // Atomar BEIDE Mutexe locken — Deadlock-frei, egal in welcher Reihenfolge
    // wir from und to übergeben.
    std::scoped_lock lk{from.m, to.m};
    from.balance -= amount;
    to.balance   += amount;
}

void demo_scoped_lock()
{
    std::cout << "--- scoped_lock ---\n";
    Account alice, bob;

    // 1000 parallele Transfers in beide Richtungen — ohne Deadlock-Gefahr.
    std::vector<std::thread> threads;
    for (int i = 0; i < 500; ++i) {
        threads.emplace_back([&] { transfer(alice, bob, 1); });
        threads.emplace_back([&] { transfer(bob, alice, 1); });
    }
    for (auto& t : threads) t.join();

    std::cout << "  alice = " << alice.balance
              << "  bob = "   << bob.balance << "  (erwartet: 100 / 100)\n";
}

// -----------------------------------------------------------------------------
// Abschnitt C: condition_variable — RICHTIG verwenden
// -----------------------------------------------------------------------------
// Die zwei häufigsten Fehler:
//   (1) wait() ohne Prädikat — spurious wakeups führen zu falschem Verhalten
//   (2) notify vor State-Änderung — Consumer verpasst das Signal
//
// Korrektes Muster:
//     std::unique_lock lk{m};
//     cv.wait(lk, [&]{ return predicate; });   // Prädikat in WHILE-Schleife
//
//     // Producer:
//     { std::lock_guard lk{m}; update_state(); }
//     cv.notify_one();                          // notify NACH State-Update

class TaskQueue {
public:
    void push(int task)
    {
        {
            std::lock_guard lk{m_};
            queue_.push(task);
        }
        cv_.notify_one();                        // genau einen Consumer wecken
    }

    // Liefert eine Task, oder "nullopt"-Signal falls stop gesetzt wurde.
    // Rückgabe via optional wäre hier sauber — wir bleiben einfach.
    bool pop(int& out)
    {
        std::unique_lock lk{m_};
        // Prädikat-Form: wait kehrt nur zurück, wenn Prädikat WAHR ist.
        // Spurious Wakeups sind automatisch abgefangen.
        cv_.wait(lk, [this] { return !queue_.empty() || stop_; });

        if (stop_ && queue_.empty()) return false;
        out = queue_.front();
        queue_.pop();
        return true;
    }

    void stop()
    {
        {
            std::lock_guard lk{m_};
            stop_ = true;
        }
        cv_.notify_all();                        // alle weckt
    }

private:
    std::mutex              m_;
    std::condition_variable cv_;
    std::queue<int>         queue_;
    bool                    stop_ = false;
};

void demo_condition_variable()
{
    std::cout << "--- condition_variable ---\n";
    TaskQueue q;

    std::thread consumer{[&q] {
        int task;
        while (q.pop(task)) {
            std::cout << "  consumed " << task << '\n';
        }
        std::cout << "  consumer exit\n";
    }};

    for (int i = 1; i <= 5; ++i) {
        q.push(i);
        std::this_thread::sleep_for(10ms);
    }
    q.stop();
    consumer.join();
}

// -----------------------------------------------------------------------------
int main()
{
#ifdef DEMO_DEADLOCK
    // Diese Zeile hängt — zum Vorführen des Deadlocks.
    deadlock_demo::run();
#endif

    demo_scoped_lock();
    std::cout << '\n';
    demo_condition_variable();
}
