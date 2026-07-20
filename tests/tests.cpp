#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "../src/noise.hpp"
#include "../src/heightmap.hpp"
#include <vector>
#include <cmath>
#include <set>

TEST_CASE("Noise primitives and permutation properties") {
    SUBCASE("Fade function interpolation boundaries") {
        CHECK(noise::fade(0.0f) == doctest::Approx(0.0f));
        CHECK(noise::fade(1.0f) == doctest::Approx(1.0f));
        CHECK(noise::fade(0.5f) == doctest::Approx(0.5f));
    }

    SUBCASE("Linear interpolation (lerp)") {
        CHECK(noise::lerp(0.0f, 2.0f, 8.0f) == doctest::Approx(2.0f));
        CHECK(noise::lerp(1.0f, 2.0f, 8.0f) == doctest::Approx(8.0f));
        CHECK(noise::lerp(0.5f, 2.0f, 8.0f) == doctest::Approx(5.0f));
    }

    SUBCASE("Permutation table generation and properties") {
        auto table1 = noise::generatePermutationTable(42);
        auto table2 = noise::generatePermutationTable(42);
        auto table3 = noise::generatePermutationTable(24);

        REQUIRE(table1.size() == 512);
        REQUIRE(table1 == table2);
        REQUIRE(table1 != table3);
    }

    SUBCASE("fBm output bounds") {
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

TEST_CASE("Heightmap sequential and parallel equivalence") {
    Heightmap heightmap(64, 42, 4);

    SUBCASE("Exact heightmap array equivalence between sequential and parallel") {
        heightmap.generateSequential();
        std::vector<float> seqData = heightmap.getData();

        heightmap.generateParallel();
        std::vector<float> parData = heightmap.getData();

        REQUIRE(seqData.size() == parData.size());
        for (size_t i = 0; i < seqData.size(); ++i) {
            CHECK(seqData[i] == doctest::Approx(parData[i]).epsilon(1e-5f));
        }
    }

    SUBCASE("Benchmark results sanity") {
        BenchmarkResults res = heightmap.runBenchmark();
        CHECK(res.timeSequential >= 0.0);
        CHECK(res.timeParallel >= 0.0);
        CHECK(res.numThreads > 0);
    }
}

TEST_CASE("Heightmap state management and determinism") {
    Heightmap heightmap(32, 100, 5);

    CHECK(heightmap.getSize() == 32);
    CHECK(heightmap.getSeed() == 100);
    CHECK(heightmap.getOctaves() == 5);

    SUBCASE("Regeneration with new seed changes terrain deterministically") {
        heightmap.generateParallel();
        auto originalData = heightmap.getData();

        // Cambiar semilla y regenerar
        heightmap.regenerate(999);
        CHECK(heightmap.getSeed() == 999);
        heightmap.resetGrid();
        auto newSeedData = heightmap.getData();

        // Verificar que los datos cambien
        CHECK(originalData != newSeedData);

        // Volver a la semilla original
        heightmap.regenerate(100);
        heightmap.resetGrid();
        auto restoredData = heightmap.getData();

        // Restaurar semilla debe producir exactamente el mismo terreno inicial
        REQUIRE(originalData.size() == restoredData.size());
        for (size_t i = 0; i < originalData.size(); ++i) {
            CHECK(originalData[i] == doctest::Approx(restoredData[i]).epsilon(1e-5f));
        }
    }

    SUBCASE("Set octaves updates internal configuration") {
        heightmap.setOctaves(12);
        CHECK(heightmap.getOctaves() == 12);
    }
}

TEST_CASE("Erosion correctness, profile metrics and non-NaN check") {
    Heightmap heightmap(64, 7, 4);
    heightmap.resetGrid();

    SUBCASE("Sequential Erosion profile and NaN check") {
        ErosionProfile profile;
        double time = heightmap.applyErosion(ErosionMode::SEQUENTIAL, &profile);
        CHECK(time >= 0.0);
        CHECK(profile.totalTime >= 0.0);
        CHECK(profile.simulationTime >= 0.0);

        for (float val : heightmap.getData()) {
            CHECK_FALSE(std::isnan(val));
            CHECK_FALSE(std::isinf(val));
        }
    }

    SUBCASE("Parallel Atomic Erosion profile and NaN check") {
        ErosionProfile profile;
        double time = heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC, &profile);
        CHECK(time >= 0.0);
        CHECK(profile.totalTime >= 0.0);
        CHECK(profile.simulationTime >= 0.0);

        for (float val : heightmap.getData()) {
            CHECK_FALSE(std::isnan(val));
            CHECK_FALSE(std::isinf(val));
        }
    }

    SUBCASE("Parallel Local Buffers Erosion profile and NaN check") {
        ErosionProfile profile;
        double time = heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS, &profile);
        CHECK(time >= 0.0);
        CHECK(profile.totalTime >= 0.0);
        CHECK(profile.allocationTime >= 0.0);
        CHECK(profile.simulationTime >= 0.0);
        CHECK(profile.reductionTime >= 0.0);

        for (float val : heightmap.getData()) {
            CHECK_FALSE(std::isnan(val));
            CHECK_FALSE(std::isinf(val));
        }
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
        CHECK_FALSE(std::isnan(val));
        CHECK_FALSE(std::isinf(val));
    }

    // Esperamos que la erosión se auto-limite y los valores no excedan límites razonables
    CHECK(maxVal < 3.0f);
    CHECK(minVal > -3.0f);
}
