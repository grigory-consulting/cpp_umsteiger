cmake_minimum_required(VERSION 3.16)
project(cpp_schulung LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wimplicit-fallthrough)
endif()

# -----------------------------------------------------------------------------
# Kern-Beispiele — jede .cpp wird zu einem eigenen Executable
# -----------------------------------------------------------------------------
set(EXAMPLES
    01_grundlagen
    02_stl_grundlagen
    03_stl_uebung
    #04_funktionen
    05_vererbung
    #06_speicher_raii
    #06b_raii_uebung
    07_smart_pointer
    #07b_smart_pointer_uebung
    08_templates
    09_exceptions
    #10_modern_cpp
    11_multithreading
    #backup_a_move
    #backup_b_spezialisierung
    #backup_d_threading_traps
)

foreach(example ${EXAMPLES})

add_executable(${example} ${example}.cpp)
endforeach()

# -----------------------------------------------------------------------------
# Qt-Beispiele — brauchen Qt6 (Core, Concurrent, Test) + AUTOMOC (wegen Q_OBJECT).
# -----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Test)
qt_standard_project_setup()

# 11b_qt_threading.cpp
qt_add_executable(11b_qt_threading 11b_qt_threading.cpp)
target_link_libraries(11b_qt_threading PRIVATE Qt6::Core Qt6::Concurrent)
set_target_properties(11b_qt_threading PROPERTIES AUTOMOC ON)

# 13_tests_qttest.cpp — QtTest-Unit-Tests
qt_add_executable(13_tests_qttest 13_tests_qttest.cpp)
target_link_libraries(13_tests_qttest PRIVATE Qt6::Core Qt6::Test)
set_target_properties(13_tests_qttest PROPERTIES AUTOMOC ON)

enable_testing()
add_test(NAME 13_tests_qttest COMMAND 13_tests_qttest)

