// =============================================================================
// 12_tests_gtest.cpp
//
// Thema        : Unit Tests mit GoogleTest.
// Lernziel     : TEST/TEST_F-Makros, Fixtures, parametrisierte Tests.
// Voraussetzung: libgtest installiert (apt install libgtest-dev / brew install gtest)
// Kompilieren  : cmake -DBUILD_GTEST=ON -S . -B build && cmake --build build
//                oder: g++ -std=c++20 12_tests_gtest.cpp -lgtest -lgtest_main -pthread
// =============================================================================

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

// -----------------------------------------------------------------------------
// System under test — eine kleine Calculator-Klasse
// -----------------------------------------------------------------------------
class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int sub(int a, int b) const { return a - b; }

    // Division mit expliziter Fehlersignalisierung durch Exception.
    int div(int a, int b) const
    {
        if (b == 0) throw std::invalid_argument{"div by zero"};
        return a / b;
    }
};

// -----------------------------------------------------------------------------
// Abschnitt A: TEST — frei stehende Test-Funktionen
// -----------------------------------------------------------------------------
// Syntax: TEST(TestSuiteName, TestName) { /* assertions */ }
//
// Wichtige Assertions:
//   EXPECT_EQ (a, b)   — Gleichheit, NICHT-fatal (Test läuft weiter)
//   ASSERT_EQ (a, b)   — Gleichheit, fatal (Test bricht sofort ab)
//   EXPECT_TRUE / FALSE
//   EXPECT_NEAR(a, b, eps) — für Fließkomma
//   EXPECT_THROW(stmt, ExcType) — Erwartete Exception
//   EXPECT_NO_THROW
//
// Faustregel: EXPECT_* ist Default, ASSERT_* nur wenn das weitere Ausführen
// des Tests nach Fehler keinen Sinn hätte (z. B. nach nullptr-Check).

TEST(CalculatorTest, AddPositive)
{
    Calculator c;
    EXPECT_EQ(c.add(2, 3), 5);
    EXPECT_EQ(c.add(0, 0), 0);
}

TEST(CalculatorTest, SubNegative)
{
    Calculator c;
    EXPECT_EQ(c.sub(1, 3), -2);
}

TEST(CalculatorTest, DivByZeroThrows)
{
    Calculator c;
    EXPECT_THROW(c.div(10, 0), std::invalid_argument);
    EXPECT_NO_THROW(c.div(10, 2));
}

// -----------------------------------------------------------------------------
// Abschnitt B: TEST_F — Test-Fixture mit gemeinsamer Vorbedingung
// -----------------------------------------------------------------------------
// Eine Fixture-Klasse erbt von ::testing::Test. SetUp()/TearDown() werden
// FÜR JEDEN Test frisch aufgerufen — jeder Test bekommt einen sauberen Zustand.

class CalculatorFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        calc_ = std::make_unique<Calculator>();
    }

    // TearDown ist implizit durch unique_ptr-Destruktor erledigt.

    std::unique_ptr<Calculator> calc_;
};

// Syntax: TEST_F(FixtureName, TestName) — hat Zugriff auf Fixture-Member.
TEST_F(CalculatorFixture, AddUsesFixture)
{
    EXPECT_EQ(calc_->add(10, 20), 30);
}

TEST_F(CalculatorFixture, SubUsesFixture)
{
    EXPECT_EQ(calc_->sub(50, 30), 20);
}

// -----------------------------------------------------------------------------
// Abschnitt C: Parametrisierte Tests
// -----------------------------------------------------------------------------
// Ein und derselbe Test-Körper wird mit mehreren Parameter-Sätzen ausgeführt —
// spart Copy-Paste-Code und macht Randfälle kompakt lesbar.

struct AddCase { int a; int b; int expected; };

class AddParamTest : public ::testing::TestWithParam<AddCase> {};

TEST_P(AddParamTest, ComputesExpectedSum)
{
    Calculator c;
    const auto& p = GetParam();
    EXPECT_EQ(c.add(p.a, p.b), p.expected);
}

INSTANTIATE_TEST_SUITE_P(
    AddVariants, AddParamTest,
    ::testing::Values(
        AddCase{ 0,  0,  0},
        AddCase{ 1,  1,  2},
        AddCase{-1,  1,  0},
        AddCase{100, 200, 300}
    )
);

// -----------------------------------------------------------------------------
// Hinweis zu main():
// -----------------------------------------------------------------------------
// Wenn wir gegen gtest_main linken, wird main() automatisch bereitgestellt.
// Falls man eigene Initialisierung braucht:
//
//   int main(int argc, char** argv) {
//       ::testing::InitGoogleTest(&argc, argv);
//       return RUN_ALL_TESTS();
//   }
