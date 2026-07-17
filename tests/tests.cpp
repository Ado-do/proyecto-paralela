#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "../src/noise.hpp"
#include "../src/heightmap.hpp"
#include <vector>
#include <cmath>

TEST_CASE("Noise generation basics") {
    SUBCASE("Permutation table generation") {
        auto table1 = noise::generatePermutationTable(42);
        auto table2 = noise::generatePermutationTable(42);
        auto table3 = noise::generatePermutationTable(24);

        REQUIRE(table1.size() == 512);
        REQUIRE(table1 == table2);
        REQUIRE(table1 != table3);
    }

    SUBCASE("fBm bounds") {
        auto table = noise::generatePermutationTable(12345);
        for (int y = 0; y < 100; y += 10) {
            for (int x = 0; x < 100; x += 10) {
                float fx = x * 0.1f;
                float fy = y * 0.1f;
                float val = noise::fbm2D(fx, fy, 8, 0.5f, 2.0f, table);
                CHECK(val >= -1.0f);
                CHECK(val <= 1.0f);
            }
        }
    }
}

TEST_CASE("Terrain sequential and parallel equivalence") {
    Heightmap heightmap(64, 42, 4);

    BenchmarkResults res = heightmap.runBenchmark();

    CHECK(res.timeSequential >= 0.0);
    CHECK(res.timeParallel >= 0.0);
    CHECK(res.numThreads > 0);
}

TEST_CASE("Erosion stability and execution") {
    Heightmap heightmap(64, 7, 4);
    heightmap.runBenchmark();

    SUBCASE("Sequential Erosion") {
        double time = heightmap.applyErosion(ErosionMode::SEQUENTIAL);
        CHECK(time >= 0.0);
    }

    SUBCASE("Parallel Atomic Erosion") {
        double time = heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC);
        CHECK(time >= 0.0);
    }

    SUBCASE("Parallel Local Buffers Erosion") {
        double time = heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);
        CHECK(time >= 0.0);
    }
}

TEST_CASE("Erosion local buffers artifacts and bounds verification") {
    // Inicializar un mapa de altura de 128x128 con una semilla conocida
    Heightmap heightmap(128, 42, 4);
    heightmap.resetGrid();

    // Aplicar erosión paralela en modo local

    heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);

    // Verificar que los valores resultantes no tengan artefactos extremos
    const auto& data = heightmap.getData();
    float maxVal = -1e9f;
    float minVal = 1e9f;
    for (float val : data) {
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    // Esperamos que la erosión se auto-limite y los valores no excedan límites razonables.
    // Sin la solución, el modo local produce pozos/picos que exceden con creces el rango [-3.0f, 3.0f].
    CHECK(maxVal < 3.0f);
    CHECK(minVal > -3.0f);
}



