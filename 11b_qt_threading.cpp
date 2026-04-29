// =============================================================================
// 11b_qt_threading.cpp
//
// Thema        : Qt-Threading — vier Hauptmuster.
// Lernziel     : moveToThread + Signals/Slots, QtConcurrent::run, QThreadPool,
//                Data Race + QMutex/QMutexLocker.
// Kompilieren  : NUR über CMake mit Qt6 (AUTOMOC nötig wegen Q_OBJECT).
//                cmake -S . -B build
//                cmake --build build --target 11b_qt_threading
//                ./build/11b_qt_threading
// =============================================================================

#include <QAtomicInt>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QtConcurrent>

#include <chrono>
#include <thread>

// Hilfs-Funktion: simuliert eine teure Berechnung
static QString heavyComputation(const QString& input)
{
    qDebug() << "  [Worker-Thread:" << QThread::currentThreadId() << "] arbeite an" << input;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    return input.toUpper() + " (verarbeitet)";
}

// -----------------------------------------------------------------------------
// Abschnitt A: moveToThread + Signals/Slots — Qt-idiomatisches Worker-Pattern
// -----------------------------------------------------------------------------
// Pattern:
//   1) Worker als QObject-Subklasse mit Slot (Arbeit) und Signal (Ergebnis)
//   2) Eigenen QThread erzeugen, Worker mit moveToThread() dorthin verschieben
//   3) Cross-Thread-Connect: startWork() → doWork(), resultReady() → onResult()
//   4) Worker und Thread sauber aufräumen, wenn fertig
//
// Vorteile:
//   - Kein manuelles Locking — Kommunikation nur über Signals/Slots
//   - Cross-Thread-Signal kopiert Args automatisch (Qt::QueuedConnection)
//   - Worker hat seinen eigenen Event-Loop (langlaufende Tasks möglich)
class Worker : public QObject
{
    Q_OBJECT
public slots:
    void doWork(const QString& input)
    {
        auto result = heavyComputation(input);
        emit resultReady(result);
    }
signals:
    void resultReady(const QString& result);
};

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller()
    {
        worker_ = new Worker;
        thread_ = new QThread;
        worker_->moveToThread(thread_);

        // startWork (im Main-Thread emittiert) → doWork (läuft im Worker-Thread)
        connect(this,    &Controller::startWork,  worker_, &Worker::doWork);
        // resultReady (im Worker-Thread emittiert) → onResult (im Main-Thread)
        connect(worker_, &Worker::resultReady,    this,    &Controller::onResult);
        // Aufräumen — wenn der Thread endet, deleteLater den Worker
        connect(thread_, &QThread::finished,      worker_, &QObject::deleteLater);

        thread_->start();
    }

    ~Controller()
    {
        thread_->quit();        // Event-Loop beenden
        thread_->wait();        // auf Ende warten (blockierend)
        delete thread_;         // Worker wird via deleteLater zerstört
    }

signals:
    void startWork(const QString& input);
    void allDone();

public slots:
    void onResult(const QString& result)
    {
        qDebug() << "  [Main-Thread:  " << QThread::currentThreadId() << "] Ergebnis:" << result;
        emit allDone();
    }

private:
    Worker*  worker_ = nullptr;
    QThread* thread_ = nullptr;
};

// -----------------------------------------------------------------------------
// Abschnitt B: QtConcurrent::run — einmalige Berechnung mit QFuture
// -----------------------------------------------------------------------------
// Wenn du nur EINE Aufgabe in einen Hintergrund-Thread auslagern willst, ohne
// einen eigenen QThread und Worker zu basteln. Liefert ein QFuture<T> zurück.
//
// Vorteile:
//   - Eine Zeile — kein Worker, kein moveToThread
//   - Thread kommt aus dem globalen QThreadPool
//   - QFuture lässt sich mit QFutureWatcher per Signal/Slot beobachten
static void demo_qtconcurrent()
{
    qDebug() << "\n--- Abschnitt B: QtConcurrent::run ---";

    // Variante 1 — synchron warten (blockiert):
    {
        QFuture<QString> future = QtConcurrent::run(heavyComputation, QString{"task-A"});
        qDebug() << "  [Variante 1, blocking] Ergebnis:" << future.result();
    }

    // Variante 2 — asynchron via QFutureWatcher:
    //   - Watcher emittiert finished()-Signal, sobald der QFuture fertig ist.
    //   - Slot/Lambda läuft im Thread, in dem der Watcher lebt (hier: Main-Thread).
    //   - In einer GUI-App würdest du KEINEN lokalen QEventLoop brauchen — der
    //     globale QApplication-Loop dispatcht das finished()-Signal automatisch.
    //   - Hier nutzen wir einen lokalen QEventLoop, damit die Demo-Funktion
    //     blockiert, bis das Signal kommt — sonst kehrt sie sofort zurück.
    {
        QFuture<QString> future = QtConcurrent::run(heavyComputation, QString{"task-B"});

        QFutureWatcher<QString> watcher;
        QEventLoop              loop;

        QObject::connect(&watcher, &QFutureWatcher<QString>::finished,
                         &loop, [&] {
                             qDebug() << "  [Variante 2, async   ] Ergebnis:"
                                      << watcher.result();
                             loop.quit();
                         });

        watcher.setFuture(future);   // ab hier kann finished() jederzeit feuern
        loop.exec();                 // wartet im Event-Loop auf finished()
    }
}

// -----------------------------------------------------------------------------
// Abschnitt C: QThreadPool + QRunnable — viele kurze, gleichartige Tasks
// -----------------------------------------------------------------------------
// QThreadPool verwaltet einen Pool von Threads (Default: Hardware-Threads).
// Aufgaben werden als QRunnable eingereicht und automatisch verteilt.
//
// Vorteile:
//   - Kein Thread-Erzeugen pro Task (Pool wiederverwendet)
//   - Skaliert automatisch zur CPU-Anzahl
//   - Ideal für eine Liste gleichartiger Aufgaben (z.B. Bild-Verarbeitung)
class MyTask : public QRunnable
{
public:
    explicit MyTask(int id) : id_{id}
    {
        // QRunnable wird vom Pool automatisch zerstört (Default).
        // Mit setAutoDelete(false) kann man es zurückbekommen.
    }

    void run() override
    {
        qDebug() << "  [Pool-Thread:" << QThread::currentThreadId() << "] task" << id_;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

private:
    int id_;
};

static void demo_threadpool()
{
    qDebug() << "\n--- Abschnitt C: QThreadPool + QRunnable ---";

    QThreadPool* pool = QThreadPool::globalInstance();
    qDebug() << "  Pool-Größe:" << pool->maxThreadCount() << "Threads";

    for (int i = 0; i < 8; ++i) {
        pool->start(new MyTask{i});            // Eigentum geht an den Pool
    }
    pool->waitForDone();                       // wartet, bis alle Tasks fertig sind
}

// -----------------------------------------------------------------------------
// Abschnitt D: Data Race — wenn die Synchronisation fehlt
// -----------------------------------------------------------------------------
// Wenn zwei oder mehr Threads denselben Speicher LESEN UND SCHREIBEN ohne
// Synchronisation, ist das eine DATA RACE — Undefined Behavior.
//
// Demonstration:
//   1) Counter ohne Schutz → Ergebnis irgendwo unter 1'000'000 (Lost Updates)
//   2) Counter mit QMutex/QMutexLocker → garantiert genau 1'000'000
//   3) Counter mit QAtomicInt → lock-free, gleich schnell wie Mutex bei int
//
// Symptom in der Praxis: nicht reproduzierbar, hängt von Scheduling, Cache,
// Compiler-Optimierungen ab. ASan/TSan helfen, sie zur Laufzeit zu finden.

constexpr int kThreads     = 10;
constexpr int kIncPerThread = 100'000;

// (1) UNSICHER — wird je nach Lauf ein anderes Ergebnis liefern
static int g_unsafe_counter = 0;

static void increment_unsafe()
{
    for (int i = 0; i < kIncPerThread; ++i) {
        ++g_unsafe_counter;                    // ⚠ Read-Modify-Write — nicht atomar
    }
}

// (2) SICHER — QMutex schützt den kritischen Abschnitt
static int      g_mutex_counter = 0;
static QMutex   g_counter_mutex;

static void increment_with_mutex()
{
    for (int i = 0; i < kIncPerThread; ++i) {
        QMutexLocker lk{&g_counter_mutex};     // RAII — unlock im Destruktor
        ++g_mutex_counter;
    }                                          // (Mutex wird hier freigegeben)
}

// (3) LOCK-FREE — QAtomicInt für einfache Werte
static QAtomicInt g_atomic_counter{0};

static void increment_atomic()
{
    for (int i = 0; i < kIncPerThread; ++i) {
        g_atomic_counter.fetchAndAddOrdered(1); // atomar, ohne Lock
    }
}

template <typename Fn>
static int run_threads_and_measure(Fn fn, const char* label)
{
    QList<QThread*> threads;
    QElapsedTimer   timer;
    timer.start();

    for (int i = 0; i < kThreads; ++i) {
        threads.append(QThread::create(fn));
    }
    for (auto* t : threads) t->start();
    for (auto* t : threads) { t->wait(); delete t; }

    qDebug().noquote() << QString{"  %1: %2 ms"}.arg(label).arg(timer.elapsed());
    return timer.elapsed();
}

static void demo_data_race()
{
    qDebug() << "\n--- Abschnitt D: Data Race & Synchronisation ---";
    qDebug().nospace() << "  Erwartet: " << kThreads * kIncPerThread
                       << " (= " << kThreads << " × " << kIncPerThread << ")";

    // (1) Ohne Schutz → Lost Updates, nicht-deterministisch
    g_unsafe_counter = 0;
    run_threads_and_measure(increment_unsafe, "unsafe (data race) ");
    qDebug() << "    →" << g_unsafe_counter
             << "(meist DEUTLICH unter Erwartung)";

    // (2) Mit QMutex/QMutexLocker
    g_mutex_counter = 0;
    run_threads_and_measure(increment_with_mutex, "QMutex/QMutexLocker");
    qDebug() << "    →" << g_mutex_counter << "(garantiert korrekt)";

    // (3) Mit QAtomicInt — lock-free
    g_atomic_counter.storeRelaxed(0);
    run_threads_and_measure(increment_atomic, "QAtomicInt          ");
    qDebug() << "    →" << g_atomic_counter.loadRelaxed() << "(garantiert korrekt)";
}

// -----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QCoreApplication app{argc, argv};

    qDebug() << "[Main-Thread:  " << QThread::currentThreadId() << "] start";

    // Abschnitt A: moveToThread-Pattern
    qDebug() << "\n--- Abschnitt A: Worker + moveToThread ---";
    Controller controller;
    QObject::connect(&controller, &Controller::allDone, &app, &QCoreApplication::quit);
    emit controller.startWork(QString{"hallo welt"});

    // Abschnitt B + C + D laufen synchron — werden NACH Beenden der Event-Loop
    // direkt im Main-Thread aufgerufen.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] {
        demo_qtconcurrent();
        demo_threadpool();
        demo_data_race();
    });

    return app.exec();
}

// AUTOMOC erzeugt die Datei automatisch — ohne CMake-AUTOMOC manuell:
//   moc 11b_qt_threading.cpp -o 11b_qt_threading.moc
//   und dann:  #include "11b_qt_threading.moc"
#include "11b_qt_threading.moc"
