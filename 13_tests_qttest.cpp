// =============================================================================
// 13_tests_qttest.cpp
//
// Thema        : Unit Tests mit QtTest — nativ in Qt Creator integriert.
// Lernziel     : Test-Slot-Mechanik, QCOMPARE, data-driven Tests.
// Voraussetzung: Qt 6 (Core + Test).
// Kompilieren  : cmake -DBUILD_QTTEST=ON -S . -B build && cmake --build build
// =============================================================================

#include <QtTest>
#include <QObject>

#include <stdexcept>

// -----------------------------------------------------------------------------
// System under test
// -----------------------------------------------------------------------------
class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int sub(int a, int b) const { return a - b; }
    int div(int a, int b) const
    {
        if (b == 0) throw std::invalid_argument{"div by zero"};
        return a / b;
    }
};

// -----------------------------------------------------------------------------
// Test-Klasse
// -----------------------------------------------------------------------------
// WICHTIG:
//   - Muss QObject sein und Q_OBJECT deklarieren (→ MOC-Generierung durch AUTOMOC)
//   - Jede Test-Methode ist ein private Q_SLOT (private slots:)
//   - Spezielle Slots:
//       initTestCase()    — einmal vor allen Tests
//       cleanupTestCase() — einmal nach allen Tests
//       init()            — vor JEDEM Test
//       cleanup()         — nach JEDEM Test
//
// Wichtige Makros:
//   QCOMPARE(actual, expected) — ähnlich EXPECT_EQ, gibt Differenz bei Fehler
//   QVERIFY(cond)              — boolesche Bedingung
//   QVERIFY_EXCEPTION_THROWN   — erwartete Exception
//   QFETCH / QTest::addColumn  — data-driven Tests

class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    // -------------------------------------------------------------------------
    // Per-Test-Setup
    // -------------------------------------------------------------------------
    void init()    { calc_ = new Calculator(); }
    void cleanup() { delete calc_; calc_ = nullptr; }

    // -------------------------------------------------------------------------
    // Einfacher Test
    // -------------------------------------------------------------------------
    void add_positive()
    {
        QCOMPARE(calc_->add(2, 3), 5);
        QCOMPARE(calc_->add(0, 0), 0);
    }

    void sub_negative()
    {
        QCOMPARE(calc_->sub(1, 3), -2);
    }

    // -------------------------------------------------------------------------
    // Exception-Test
    // -------------------------------------------------------------------------
    void div_by_zero_throws()
    {
        QVERIFY_EXCEPTION_THROWN(calc_->div(10, 0), std::invalid_argument);
    }

    // -------------------------------------------------------------------------
    // Data-driven: _data() liefert die Zeilen, die eigentliche Test-Methode
    // wird je Zeile einmal ausgeführt.
    // -------------------------------------------------------------------------
    void add_data()
    {
        QTest::addColumn<int>("a");
        QTest::addColumn<int>("b");
        QTest::addColumn<int>("expected");

        QTest::newRow("pos+pos") <<  2 <<  3 <<  5;
        QTest::newRow("neg+neg") << -1 << -2 << -3;
        QTest::newRow("zero")    <<  0 <<  0 <<  0;
        QTest::newRow("large")   << 100 << 200 << 300;
    }

    void add()
    {
        QFETCH(int, a);
        QFETCH(int, b);
        QFETCH(int, expected);

        QCOMPARE(calc_->add(a, b), expected);
    }

private:
    Calculator* calc_ = nullptr;
};

// -----------------------------------------------------------------------------
// QTEST_MAIN erzeugt eine main()-Funktion, die alle Test-Slots ausführt und
// einen XML- oder Text-Report produziert.
//
// Der #include "13_tests_qttest.moc" ist beim Single-File-Setup nötig. Mit
// CMake + AUTOMOC: nicht erforderlich.
// -----------------------------------------------------------------------------
QTEST_MAIN(TestCalculator)
#include "13_tests_qttest.moc"
