# C++ Schulung — Code-Beispiele

Begleitmaterial zur Präsentation [../cpp_schulung.qmd](../cpp_schulung.qmd).
Jede Datei ist **in sich geschlossen**, ausführlich kommentiert und kompilierbar.

## Build

### Mit CMake (empfohlen, auch in Qt Creator)

```bash
cmake -S . -B build -DCMAKE_CXX_STANDARD=20
cmake --build build
```

Danach liegen alle Beispiele als ausführbare Dateien in `build/`.

### Einzelne Datei direkt kompilieren

```bash
g++ -std=c++20 -Wall -Wextra -pedantic 01_grundlagen.cpp -o 01_grundlagen
./01_grundlagen
```

Für Multithreading-Beispiele zusätzlich `-pthread`, für Ranges ggf. `-std=c++20`.

## Struktur

| Datei                             | Thema (Präsentation)                    |
|-----------------------------------|-----------------------------------------|
| `01_grundlagen.cpp`               | Variablen, Kontrollstrukturen, Klassen  |
| `02_stl_grundlagen.cpp`           | Container, Algorithmen, Iteratoren      |
| `03_stl_uebung.cpp`               | STL-Übung (Sensor-Daten)                |
| `04_funktionen.cpp`               | Parameterübergabe, Lambdas, RVO         |
| `05_vererbung.cpp`                | Klassen, Polymorphismus, Slicing        |
| `06_speicher_raii.cpp`            | new/delete, RAII, Rule of 5             |
| `06b_raii_uebung.cpp`             | RAII-Übung (ScopeGuard, Timer, DynArray) |
| `07_smart_pointer.cpp`            | unique/shared/weak_ptr                  |
| `07b_smart_pointer_uebung.cpp`    | Smart-Pointer-Übung (Factory, Zyklus)   |
| `08_templates.cpp`                | Templates, Concepts                     |
| `09_exceptions.cpp`               | Exception Handling                      |
| `10_modern_cpp.cpp`               | C++11/14/17/20 Features                 |
| `11_multithreading.cpp`           | std::thread, Mutex, async, atomic       |
| `11b_qt_threading.cpp`            | Qt: moveToThread, QtConcurrent, QThreadPool *(opt-in via `-DBUILD_QT_THREADING=ON`)* |
| `12_tests_gtest.cpp`              | Unit Tests mit GoogleTest               |
| `13_tests_qttest.cpp`             | Unit Tests mit QtTest                   |
| `backup_a_move.cpp`               | Move-Semantik (Rule of 5, forwarding)   |
| `backup_b_spezialisierung.cpp`    | Template-Spezialisierung                |
| `backup_c_sanitizers.cpp`         | Fehlerbeispiele für ASan/UBSan/TSan     |
| `backup_d_threading_traps.cpp`    | Deadlocks, condition_variable korrekt   |

## Voraussetzungen

- Compiler mit C++20-Unterstützung (gcc ≥ 10, clang ≥ 12, MSVC 2019+)
- Optional: GoogleTest, Qt (für `12_tests_gtest.cpp` / `13_tests_qttest.cpp`)
- Optional: Sanitizer-Unterstützung im Compiler (gcc/clang)
