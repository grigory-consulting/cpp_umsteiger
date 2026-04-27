cmake_minimum_required(VERSION 3.16)
project(cpp_schulung LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Kern-Beispiele — jede .cpp wird zu einem eigenen Executable
# -----------------------------------------------------------------------------
set(EXAMPLES
    01_grundlagen
    02_stl_grundlagen
    03_stl_uebung
    04_funktionen
    05_vererbung
    06_speicher_raii
    07_smart_pointer
    08_templates
    09_exceptions
    10_modern_cpp
    11_multithreading
    backup_a_move
    backup_b_spezialisierung
    backup_d_threading_traps
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.cpp)
endforeach()

# Multithreading-Targets — portable Threads-Abstraktion:
#   Linux/macOS  -> linkt pthread (-pthread-Flag wird gesetzt)
#   Windows MSVC -> No-op, std::thread nutzt direkt die Win32-API
#   Windows MinGW -> linkt winpthread falls vorhanden
find_package(Threads REQUIRED)
target_link_libraries(11_multithreading       PRIVATE Threads::Threads)
target_link_libraries(backup_d_threading_traps PRIVATE Threads::Threads)

# -----------------------------------------------------------------------------
# Sanitizer-Beispiel — separat, da hier Laufzeitfehler ABSICHTLICH auftreten
# Build: cmake --build build --target backup_c_sanitizers
# Run  : ./build/backup_c_sanitizers  (ASan-Report erscheint)
# -----------------------------------------------------------------------------
add_executable(backup_c_sanitizers backup_c_sanitizers.cpp)
if(NOT MSVC)
    target_compile_options(backup_c_sanitizers PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1)
    target_link_options(backup_c_sanitizers PRIVATE
        -fsanitize=address,undefined)
endif()

# -----------------------------------------------------------------------------
# Optional: GoogleTest-Beispiel
# Aktivieren mit: cmake -DBUILD_GTEST=ON
# -----------------------------------------------------------------------------
option(BUILD_GTEST "Build GoogleTest example" OFF)
if(BUILD_GTEST)
    find_package(GTest REQUIRED)
    add_executable(12_tests_gtest 12_tests_gtest.cpp)
    target_link_libraries(12_tests_gtest PRIVATE GTest::gtest GTest::gtest_main)
    enable_testing()
    add_test(NAME gtest_example COMMAND 12_tests_gtest)
endif()

# -----------------------------------------------------------------------------
# Optional: QtTest-Beispiel
# Aktivieren mit: cmake -DBUILD_QTTEST=ON
# -----------------------------------------------------------------------------
option(BUILD_QTTEST "Build QtTest example" OFF)
if(BUILD_QTTEST)
    find_package(Qt6 COMPONENTS Core Test REQUIRED)
    qt_standard_project_setup()
    add_executable(13_tests_qttest 13_tests_qttest.cpp)
    target_link_libraries(13_tests_qttest PRIVATE Qt6::Core Qt6::Test)
    set_target_properties(13_tests_qttest PROPERTIES AUTOMOC ON)
endif()
