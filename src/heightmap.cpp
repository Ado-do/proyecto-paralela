#include "heightmap.hpp"
#include "noise.hpp"
#include "terrain_config.hpp"
#include <chrono>
#include <omp.h>
#include <iostream>
#include <random>
#include <cmath>

using namespace std;

Heightmap::Heightmap(int size, unsigned int seed, int octaves) 
    : m_size(size), m_seed(seed), m_octaves(octaves), m_data(size * size) {
    m_pTable = noise::generatePermutationTable(m_seed);
}

void Heightmap::generateSequential() {
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                m_octaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

void Heightmap::generateParallel() {
    #pragma omp parallel for collapse(2) schedule(runtime)
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                m_octaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

void Heightmap::regenerate(unsigned int newSeed) {
    m_seed = newSeed;
    m_pTable = noise::generatePermutationTable(m_seed);
}

void Heightmap::setOctaves(int octaves) {
    m_octaves = octaves;
}

void Heightmap::resetGrid() {
    generateParallel();
}

BenchmarkResults Heightmap::runBenchmark(bool quick) {
    int numThreads = omp_get_max_threads();
    double tSeq = 0.0;
    double tPar = 0.0;

    if (quick) {
        // Modo rápido para el visualizador interactivo (sin warmups ni repeticiones)
        auto startSeq = chrono::high_resolution_clock::now();
        generateSequential();
        auto endSeq = chrono::high_resolution_clock::now();
        tSeq = chrono::duration<double, milli>(endSeq - startSeq).count();

        auto startPar = chrono::high_resolution_clock::now();
        generateParallel();
        auto endPar = chrono::high_resolution_clock::now();
        tPar = chrono::duration<double, milli>(endPar - startPar).count();
    } else {
        // Modo riguroso para benchmarks offline y tests
        cout << "Iniciando benchmark (Tamaño: " << m_size << "x" << m_size << ", Hilos: " << numThreads << ", Octavas: " << m_octaves << ")..." << endl;

        // Warmup Secuencial (2 pasadas)
        for (int w = 0; w < 2; w++) {
            generateSequential();
        }
        // Repeticiones Secuencial (5 pasadas, tomamos el promedio)
        double sumSeq = 0.0;
        for (int r = 0; r < 5; r++) {
            auto startSeq = chrono::high_resolution_clock::now();
            generateSequential();
            auto endSeq = chrono::high_resolution_clock::now();
            sumSeq += chrono::duration<double, milli>(endSeq - startSeq).count();
        }
        tSeq = sumSeq / 5.0;

        // Warmup Paralelo (2 pasadas)
        for (int w = 0; w < 2; w++) {
            generateParallel();
        }
        // Repeticiones Paralelo (5 pasadas, tomamos el promedio)
        double sumPar = 0.0;
        for (int r = 0; r < 5; r++) {
            auto startPar = chrono::high_resolution_clock::now();
            generateParallel();
            auto endPar = chrono::high_resolution_clock::now();
            sumPar += chrono::duration<double, milli>(endPar - startPar).count();
        }
        tPar = sumPar / 5.0;
    }

    double speedup = tSeq / tPar;
    double efficiency = (speedup / numThreads) * 100.0;
    double cost = numThreads * tPar;

    return { 
        tSeq, tPar, speedup, efficiency, cost, numThreads,
        m_seed, terrainNoiseScale, m_octaves, terrainPersistence, terrainLacunarity,
        0.0, ErosionMode::SEQUENTIAL
    };
}

namespace {
template <typename GetHeightFunc, typename ModifyHeightFunc>
inline void simulateSingleDroplet(int dropletIndex, unsigned int seed, int size,
                                  GetHeightFunc&& getHeight,
                                  ModifyHeightFunc&& modifyHeight) {
    float posX, posY;
    getDropletInitialPosition(dropletIndex, seed, size, posX, posY);
    float velX = 0.0f;
    float velY = 0.0f;
    float speed = initialSpeed;
    float water = initialWater;
    float sediment = 0.0f;

    for (int step = 0; step < dropletMaxLifetime; step++) {
        int ipx = static_cast<int>(posX);
        int ipy = static_cast<int>(posY);

        if (ipx < 0 || ipx >= size - 1 || ipy < 0 || ipy >= size - 1) break;

        float xOffset = posX - ipx;
        float yOffset = posY - ipy;

        int indexNW = ipy * size + ipx;
        int indexNE = indexNW + 1;
        int indexSW = (ipy + 1) * size + ipx;
        int indexSE = indexSW + 1;

        float hNW = getHeight(indexNW);
        float hNE = getHeight(indexNE);
        float hSW = getHeight(indexSW);
        float hSE = getHeight(indexSE);

        // Cálculo del gradiente
        float gradX = (hNE - hNW) * (1.0f - yOffset) + (hSE - hSW) * yOffset;
        float gradY = (hSW - hNW) * (1.0f - xOffset) + (hSE - hNE) * xOffset;

        // Actualización de dirección e inercia de la gota
        velX = velX * inertia - gradX * (1.0f - inertia);
        velY = velY * inertia - gradY * (1.0f - inertia);

        float len = sqrt(velX * velX + velY * velY);
        if (len != 0.0f) {
            velX /= len;
            velY /= len;
        }

        float nextX = posX + velX;
        float nextY = posY + velY;

        if (nextX < 0.0f || nextX >= size - 1 || nextY < 0.0f || nextY >= size - 1) break;

        // Cálculo de altura bilineal
        float hOld = hNW * (1.0f - xOffset) * (1.0f - yOffset) +
                     hNE * xOffset * (1.0f - yOffset) +
                     hSW * (1.0f - xOffset) * yOffset +
                     hSE * xOffset * yOffset;

        int nipx = static_cast<int>(nextX);
        int nipy = static_cast<int>(nextY);
        float nxOffset = nextX - nipx;
        float nyOffset = nextY - nipy;
        int nindexNW = nipy * size + nipx;
        int nindexNE = nindexNW + 1;
        int nindexSW = (nipy + 1) * size + nipx;
        int nindexSE = nindexSW + 1;

        float hNew = getHeight(nindexNW) * (1.0f - nxOffset) * (1.0f - nyOffset) +
                     getHeight(nindexNE) * nxOffset * (1.0f - nyOffset) +
                     getHeight(nindexSW) * (1.0f - nxOffset) * nyOffset +
                     getHeight(nindexSE) * nxOffset * nyOffset;

        float deltaH = hNew - hOld;

        // Actualización de velocidad
        if (deltaH < 0.0f) {
            speed = sqrt(speed * speed - deltaH * gravity);
        } else {
            speed = max(0.0f, speed - deltaH * gravity);
        }

        if (speed == 0.0f) break;

        // Capacidad de sedimento
        float slope = max(-deltaH, 0.0f);
        float capacity = max(0.0f, speed * water * slope * capacityFactor);

        if (sediment > capacity || deltaH > 0.0f) {
            float depositAmount = (deltaH > 0.0f) ? min(deltaH, sediment) : (sediment - capacity) * depositionSpeed;
            sediment -= depositAmount;

            modifyHeight(indexNW, (1.0f - xOffset) * (1.0f - yOffset) * depositAmount);
            modifyHeight(indexNE, xOffset * (1.0f - yOffset) * depositAmount);
            modifyHeight(indexSW, (1.0f - xOffset) * yOffset * depositAmount);
            modifyHeight(indexSE, xOffset * yOffset * depositAmount);
        } else {
            float erodeAmount = min((capacity - sediment) * erosionSpeed, -deltaH);
            sediment += erodeAmount;

            modifyHeight(indexNW, -(1.0f - xOffset) * (1.0f - yOffset) * erodeAmount);
            modifyHeight(indexNE, -xOffset * (1.0f - yOffset) * erodeAmount);
            modifyHeight(indexSW, -(1.0f - xOffset) * yOffset * erodeAmount);
            modifyHeight(indexSE, -xOffset * yOffset * erodeAmount);
        }

        water *= (1.0f - evaporationRate);
        posX = nextX;
        posY = nextY;
    }
}
} // namespace anónimo

double Heightmap::applyErosion(ErosionMode mode, ErosionProfile* profile) {
    auto start = chrono::high_resolution_clock::now();

    int numThreads = omp_get_max_threads();
    double tAlloc = 0.0;
    double tSim = 0.0;
    double tRed = 0.0;

    if (mode == ErosionMode::SEQUENTIAL) {
        auto startSim = chrono::high_resolution_clock::now();

        for (int i = 0; i < erosionDroplets; i++) {
            simulateSingleDroplet(i, m_seed, m_size,
                [this](int idx) { return m_data[idx]; },
                [this](int idx, float delta) { m_data[idx] += delta; }
            );
        }
        auto endSim = chrono::high_resolution_clock::now();
        tSim = chrono::duration<double, milli>(endSim - startSim).count();
    } 
    else if (mode == ErosionMode::PARALLEL_ATOMIC) {
        auto startSim = chrono::high_resolution_clock::now();
        #pragma omp parallel
        {
            #pragma omp for schedule(runtime)
            for (int i = 0; i < erosionDroplets; i++) {
                simulateSingleDroplet(i, m_seed, m_size,
                    [this](int idx) { return m_data[idx]; },
                    [this](int idx, float delta) {
                        #pragma omp atomic
                        m_data[idx] += delta;
                    }
                );
            }
        }
        auto endSim = chrono::high_resolution_clock::now();
        tSim = chrono::duration<double, milli>(endSim - startSim).count();
    } 
    else if (mode == ErosionMode::PARALLEL_LOCAL_BUFFERS) {
        auto startAlloc = chrono::high_resolution_clock::now();
        // Asignar memoria de deltas locales
        size_t totalDeltaSize = static_cast<size_t>(numThreads) * m_size * m_size;
        vector<float> localDelta(totalDeltaSize);

        // Inicialización First-Touch paralela con OpenMP para distribuir páginas en nodos NUMA
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < totalDeltaSize; ++i) {
            localDelta[i] = 0.0f;
        }
        auto endAlloc = chrono::high_resolution_clock::now();
        tAlloc = chrono::duration<double, milli>(endAlloc - startAlloc).count();

        auto startSim = chrono::high_resolution_clock::now();
        #pragma omp parallel
        {
            int threadId = omp_get_thread_num();
            float* myDelta = &localDelta[threadId * m_size * m_size];

            #pragma omp for schedule(runtime)
            for (int i = 0; i < erosionDroplets; i++) {
                simulateSingleDroplet(i, m_seed, m_size,
                    [this, myDelta](int idx) { return m_data[idx] + myDelta[idx]; },
                    [myDelta](int idx, float delta) { myDelta[idx] += delta; }
                );
            }
        }
        auto endSim = chrono::high_resolution_clock::now();
        tSim = chrono::duration<double, milli>(endSim - startSim).count();

        // Reducción paralela optimizada por bloques contiguos y vectorización SIMD
        auto startRed = chrono::high_resolution_clock::now();
        const int totalGridSize = m_size * m_size;
        const int BLOCK_SIZE = 4096;

        #pragma omp parallel for schedule(static)
        for (int b = 0; b < totalGridSize; b += BLOCK_SIZE) {
            int chunkSize = min(BLOCK_SIZE, totalGridSize - b);
            for (int t = 0; t < numThreads; t++) {
                const float* src = &localDelta[t * totalGridSize + b];
                float* dst = &m_data[b];
                #pragma omp simd
                for (int k = 0; k < chunkSize; k++) {
                    dst[k] += src[k];
                }
            }
        }
        auto endRed = chrono::high_resolution_clock::now();
        tRed = chrono::duration<double, milli>(endRed - startRed).count();
    }

    auto end = chrono::high_resolution_clock::now();
    double tTotal = chrono::duration<double, milli>(end - start).count();

    if (profile) {
        profile->totalTime = tTotal;
        profile->allocationTime = tAlloc;
        profile->simulationTime = tSim;
        profile->reductionTime = tRed;
    }

    return tTotal;
}
