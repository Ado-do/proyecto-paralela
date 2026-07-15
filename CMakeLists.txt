cmake_minimum_required(VERSION 3.28)
project(raymap CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opción para habilitar el perfilamiento con herramientas como gprof o valgrind (-g -pg)
option(ENABLE_PROFILING "Habilitar compilación con flags de perfilamiento" OFF)
if(ENABLE_PROFILING)
    add_compile_options(-g -pg)
    add_link_options(-pg)
endif()

# Librerias
find_package(OpenMP REQUIRED)

# Opción para construir o no el visualizador 3D (útil para servidores sin entorno gráfico)
option(BUILD_VISUALIZER "Construir el visualizador 3D (requiere dependencias X11/Raylib)" ON)

if(BUILD_VISUALIZER)
    find_package(raylib QUIET)
    if (NOT raylib_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 6.0
            GIT_SHALLOW 1
        )
        FetchContent_MakeAvailable(raylib)
    endif()

    # --- Raymap (Visualizador)
    add_executable(${PROJECT_NAME}
        src/main.cpp
        src/noise.cpp
        src/terrain.cpp
        src/visuals.cpp
        src/heightmap.cpp
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# --- Benchmarks
add_executable(raymap_benchmark
    src/benchmark.cpp
    src/noise.cpp
    src/heightmap.cpp
)
target_link_libraries(raymap_benchmark PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(raymap_benchmark PRIVATE $<$<CONFIG:Release>:-O3>)

# --- Testing
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

add_executable(raymap_tests
    tests/tests.cpp
    src/noise.cpp
    src/heightmap.cpp
)

target_link_libraries(raymap_tests PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(raymap_tests PRIVATE doctest::doctest)

add_test(NAME CoreTests COMMAND raymap_tests)
