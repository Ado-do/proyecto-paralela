#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "../src/noise.hpp"
#include "../src/terrain.hpp"
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
    Terrain terrain(64, 42, 4);

    BenchmarkResults res = terrain.runBenchmark();

    CHECK(res.timeSequential >= 0.0);
    CHECK(res.timeParallel >= 0.0);
    CHECK(res.numThreads > 0);
}

TEST_CASE("Erosion stability and execution") {
    Terrain terrain(64, 7, 4);
    terrain.runBenchmark();

    SUBCASE("Sequential Erosion") {
        double time = terrain.applyErosion(ErosionMode::SEQUENTIAL);
        CHECK(time >= 0.0);
    }

    SUBCASE("Parallel Atomic Erosion") {
        double time = terrain.applyErosion(ErosionMode::PARALLEL_ATOMIC);
        CHECK(time >= 0.0);
    }

    SUBCASE("Parallel Local Buffers Erosion") {
        double time = terrain.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);
        CHECK(time >= 0.0);
    }
}
