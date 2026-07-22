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

double Heightmap::applyErosion(ErosionMode mode, ErosionProfile* profile) {
    auto start = chrono::high_resolution_clock::now();

    int numThreads = omp_get_max_threads();
    double tAlloc = 0.0;
    double tSim = 0.0;
    double tRed = 0.0;

    if (mode == ErosionMode::SEQUENTIAL) {
        auto startSim = chrono::high_resolution_clock::now();

        for (int i = 0; i < erosionDroplets; i++) {
            float posX, posY;
            getDropletInitialPosition(i, m_seed, m_size, posX, posY);
            float velX = 0.0f;
            float velY = 0.0f;
            float speed = initialSpeed;
            float water = initialWater;
            float sediment = 0.0f;

            for (int step = 0; step < dropletMaxLifetime; step++) {
                int ipx = static_cast<int>(posX);
                int ipy = static_cast<int>(posY);

                if (ipx < 0 || ipx >= m_size - 1 || ipy < 0 || ipy >= m_size - 1) break;

                float xOffset = posX - ipx;
                float yOffset = posY - ipy;

                int indexNW = ipy * m_size + ipx;
                int indexNE = indexNW + 1;
                int indexSW = (ipy + 1) * m_size + ipx;
                int indexSE = indexSW + 1;

                float hNW = m_data[indexNW];
                float hNE = m_data[indexNE];
                float hSW = m_data[indexSW];
                float hSE = m_data[indexSE];

                // Calculate gradient
                float gradX = (hNE - hNW) * (1.0f - yOffset) + (hSE - hSW) * yOffset;
                float gradY = (hSW - hNW) * (1.0f - xOffset) + (hSE - hNE) * xOffset;

                // Update droplet's direction and position
                velX = velX * inertia - gradX * (1.0f - inertia);
                velY = velY * inertia - gradY * (1.0f - inertia);

                float len = sqrt(velX * velX + velY * velY);
                if (len != 0.0f) {
                    velX /= len;
                    velY /= len;
                }

                float nextX = posX + velX;
                float nextY = posY + velY;

                if (nextX < 0.0f || nextX >= m_size - 1 || nextY < 0.0f || nextY >= m_size - 1) break;

                // Bilinear height calculation
                float hOld = hNW * (1.0f - xOffset) * (1.0f - yOffset) +
                             hNE * xOffset * (1.0f - yOffset) +
                             hSW * (1.0f - xOffset) * yOffset +
                             hSE * xOffset * yOffset;

                int nipx = static_cast<int>(nextX);
                int nipy = static_cast<int>(nextY);
                float nxOffset = nextX - nipx;
                float nyOffset = nextY - nipy;
                int nindexNW = nipy * m_size + nipx;
                int nindexNE = nindexNW + 1;
                int nindexSW = (nipy + 1) * m_size + nipx;
                int nindexSE = nindexSW + 1;

                float hNew = m_data[nindexNW] * (1.0f - nxOffset) * (1.0f - nyOffset) +
                             m_data[nindexNE] * nxOffset * (1.0f - nyOffset) +
                             m_data[nindexSW] * (1.0f - nxOffset) * nyOffset +
                             m_data[nindexSE] * nxOffset * nyOffset;

                float deltaH = hNew - hOld;

                // Update speed
                if (deltaH < 0.0f) {
                    speed = sqrt(speed * speed - deltaH * gravity);
                } else {
                    speed = max(0.0f, speed - deltaH * gravity);
                }

                if (speed == 0.0f) break;

                // Calculate capacity
                float slope = max(-deltaH, 0.0f);
                float capacity = max(0.0f, speed * water * slope * capacityFactor);

                if (sediment > capacity || deltaH > 0.0f) {
                    float depositAmount = (deltaH > 0.0f) ? min(deltaH, sediment) : (sediment - capacity) * depositionSpeed;
                    sediment -= depositAmount;

                    m_data[indexNW] += (1.0f - xOffset) * (1.0f - yOffset) * depositAmount;
                    m_data[indexNE] += xOffset * (1.0f - yOffset) * depositAmount;
                    m_data[indexSW] += (1.0f - xOffset) * yOffset * depositAmount;
                    m_data[indexSE] += xOffset * yOffset * depositAmount;
                } else {
                    float erodeAmount = min((capacity - sediment) * erosionSpeed, -deltaH);
                    sediment += erodeAmount;

                    m_data[indexNW] -= (1.0f - xOffset) * (1.0f - yOffset) * erodeAmount;
                    m_data[indexNE] -= xOffset * (1.0f - yOffset) * erodeAmount;
                    m_data[indexSW] -= (1.0f - xOffset) * yOffset * erodeAmount;
                    m_data[indexSE] -= xOffset * yOffset * erodeAmount;
                }

                water *= (1.0f - evaporationRate);
                posX = nextX;
                posY = nextY;
            }
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
                float posX, posY;
                getDropletInitialPosition(i, m_seed, m_size, posX, posY);
                float velX = 0.0f;
                float velY = 0.0f;
                float speed = initialSpeed;
                float water = initialWater;
                float sediment = 0.0f;

                for (int step = 0; step < dropletMaxLifetime; step++) {
                    int ipx = static_cast<int>(posX);
                    int ipy = static_cast<int>(posY);

                    if (ipx < 0 || ipx >= m_size - 1 || ipy < 0 || ipy >= m_size - 1) break;

                    float xOffset = posX - ipx;
                    float yOffset = posY - ipy;

                    int indexNW = ipy * m_size + ipx;
                    int indexNE = indexNW + 1;
                    int indexSW = (ipy + 1) * m_size + ipx;
                    int indexSE = indexSW + 1;

                    float hNW = m_data[indexNW];
                    float hNE = m_data[indexNE];
                    float hSW = m_data[indexSW];
                    float hSE = m_data[indexSE];

                    // Calculate gradient
                    float gradX = (hNE - hNW) * (1.0f - yOffset) + (hSE - hSW) * yOffset;
                    float gradY = (hSW - hNW) * (1.0f - xOffset) + (hSE - hNE) * xOffset;

                    // Update droplet's direction and position
                    velX = velX * inertia - gradX * (1.0f - inertia);
                    velY = velY * inertia - gradY * (1.0f - inertia);

                    float len = sqrt(velX * velX + velY * velY);
                    if (len != 0.0f) {
                        velX /= len;
                        velY /= len;
                    }

                    float nextX = posX + velX;
                    float nextY = posY + velY;

                    if (nextX < 0.0f || nextX >= m_size - 1 || nextY < 0.0f || nextY >= m_size - 1) break;

                    // Bilinear height calculation
                    float hOld = hNW * (1.0f - xOffset) * (1.0f - yOffset) +
                                 hNE * xOffset * (1.0f - yOffset) +
                                 hSW * (1.0f - xOffset) * yOffset +
                                 hSE * xOffset * yOffset;

                    int nipx = static_cast<int>(nextX);
                    int nipy = static_cast<int>(nextY);
                    float nxOffset = nextX - nipx;
                    float nyOffset = nextY - nipy;
                    int nindexNW = nipy * m_size + nipx;
                    int nindexNE = nindexNW + 1;
                    int nindexSW = (nipy + 1) * m_size + nipx;
                    int nindexSE = nindexSW + 1;

                    float hNew = m_data[nindexNW] * (1.0f - nxOffset) * (1.0f - nyOffset) +
                                 m_data[nindexNE] * nxOffset * (1.0f - nyOffset) +
                                 m_data[nindexSW] * (1.0f - nxOffset) * nyOffset +
                                 m_data[nindexSE] * nxOffset * nyOffset;

                    float deltaH = hNew - hOld;

                    // Update speed
                    if (deltaH < 0.0f) {
                        speed = sqrt(speed * speed - deltaH * gravity);
                    } else {
                        speed = max(0.0f, speed - deltaH * gravity);
                    }

                    if (speed == 0.0f) break;

                    // Calculate capacity
                    float slope = max(-deltaH, 0.0f);
                    float capacity = max(0.0f, speed * water * slope * capacityFactor);

                    if (sediment > capacity || deltaH > 0.0f) {
                        float depositAmount = (deltaH > 0.0f) ? min(deltaH, sediment) : (sediment - capacity) * depositionSpeed;
                        sediment -= depositAmount;

                        float dNW = (1.0f - xOffset) * (1.0f - yOffset) * depositAmount;
                        float dNE = xOffset * (1.0f - yOffset) * depositAmount;
                        float dSW = (1.0f - xOffset) * yOffset * depositAmount;
                        float dSE = xOffset * yOffset * depositAmount;

                        #pragma omp atomic
                        m_data[indexNW] += dNW;
                        #pragma omp atomic
                        m_data[indexNE] += dNE;
                        #pragma omp atomic
                        m_data[indexSW] += dSW;
                        #pragma omp atomic
                        m_data[indexSE] += dSE;
                    } else {
                        float erodeAmount = min((capacity - sediment) * erosionSpeed, -deltaH);
                        sediment += erodeAmount;

                        float eNW = (1.0f - xOffset) * (1.0f - yOffset) * erodeAmount;
                        float eNE = xOffset * (1.0f - yOffset) * erodeAmount;
                        float eSW = (1.0f - xOffset) * yOffset * erodeAmount;
                        float eSE = xOffset * yOffset * erodeAmount;

                        #pragma omp atomic
                        m_data[indexNW] -= eNW;
                        #pragma omp atomic
                        m_data[indexNE] -= eNE;
                        #pragma omp atomic
                        m_data[indexSW] -= eSW;
                        #pragma omp atomic
                        m_data[indexSE] -= eSE;
                    }

                    water *= (1.0f - evaporationRate);
                    posX = nextX;
                    posY = nextY;
                }
            }
        }
        auto endSim = chrono::high_resolution_clock::now();
        tSim = chrono::duration<double, milli>(endSim - startSim).count();
    } 
    else if (mode == ErosionMode::PARALLEL_LOCAL_BUFFERS) {
        auto startAlloc = chrono::high_resolution_clock::now();
        vector<float> localDelta(numThreads * m_size * m_size, 0.0f);
        auto endAlloc = chrono::high_resolution_clock::now();
        tAlloc = chrono::duration<double, milli>(endAlloc - startAlloc).count();

        auto startSim = chrono::high_resolution_clock::now();
        #pragma omp parallel
        {
            int threadId = omp_get_thread_num();
            float* myDelta = &localDelta[threadId * m_size * m_size];

            #pragma omp for schedule(runtime)
            for (int i = 0; i < erosionDroplets; i++) {
                float posX, posY;
                getDropletInitialPosition(i, m_seed, m_size, posX, posY);
                float velX = 0.0f;
                float velY = 0.0f;
                float speed = initialSpeed;
                float water = initialWater;
                float sediment = 0.0f;

                for (int step = 0; step < dropletMaxLifetime; step++) {
                    int ipx = static_cast<int>(posX);
                    int ipy = static_cast<int>(posY);

                    if (ipx < 0 || ipx >= m_size - 1 || ipy < 0 || ipy >= m_size - 1) break;

                    float xOffset = posX - ipx;
                    float yOffset = posY - ipy;

                    int indexNW = ipy * m_size + ipx;
                    int indexNE = indexNW + 1;
                    int indexSW = (ipy + 1) * m_size + ipx;
                    int indexSE = indexSW + 1;

                    float hNW = m_data[indexNW] + myDelta[indexNW];
                    float hNE = m_data[indexNE] + myDelta[indexNE];
                    float hSW = m_data[indexSW] + myDelta[indexSW];
                    float hSE = m_data[indexSE] + myDelta[indexSE];

                    // Calculate gradient
                    float gradX = (hNE - hNW) * (1.0f - yOffset) + (hSE - hSW) * yOffset;
                    float gradY = (hSW - hNW) * (1.0f - xOffset) + (hSE - hNE) * xOffset;

                    // Update droplet's direction and position
                    velX = velX * inertia - gradX * (1.0f - inertia);
                    velY = velY * inertia - gradY * (1.0f - inertia);

                    float len = sqrt(velX * velX + velY * velY);
                    if (len != 0.0f) {
                        velX /= len;
                        velY /= len;
                    }

                    float nextX = posX + velX;
                    float nextY = posY + velY;

                    if (nextX < 0.0f || nextX >= m_size - 1 || nextY < 0.0f || nextY >= m_size - 1) break;

                    // Bilinear height calculation
                    float hOld = hNW * (1.0f - xOffset) * (1.0f - yOffset) +
                                 hNE * xOffset * (1.0f - yOffset) +
                                 hSW * (1.0f - xOffset) * yOffset +
                                 hSE * xOffset * yOffset;

                    int nipx = static_cast<int>(nextX);
                    int nipy = static_cast<int>(nextY);
                    float nxOffset = nextX - nipx;
                    float nyOffset = nextY - nipy;
                    int nindexNW = nipy * m_size + nipx;
                    int nindexNE = nindexNW + 1;
                    int nindexSW = (nipy + 1) * m_size + nipx;
                    int nindexSE = nindexSW + 1;

                    float hNew = (m_data[nindexNW] + myDelta[nindexNW]) * (1.0f - nxOffset) * (1.0f - nyOffset) +
                                 (m_data[nindexNE] + myDelta[nindexNE]) * nxOffset * (1.0f - nyOffset) +
                                 (m_data[nindexSW] + myDelta[nindexSW]) * (1.0f - nxOffset) * nyOffset +
                                 (m_data[nindexSE] + myDelta[nindexSE]) * nxOffset * nyOffset;


                    float deltaH = hNew - hOld;

                    // Update speed
                    if (deltaH < 0.0f) {
                        speed = sqrt(speed * speed - deltaH * gravity);
                    } else {
                        speed = max(0.0f, speed - deltaH * gravity);
                    }

                    if (speed == 0.0f) break;

                    // Calculate capacity
                    float slope = max(-deltaH, 0.0f);
                    float capacity = max(0.0f, speed * water * slope * capacityFactor);

                    if (sediment > capacity || deltaH > 0.0f) {
                        float depositAmount = (deltaH > 0.0f) ? min(deltaH, sediment) : (sediment - capacity) * depositionSpeed;
                        sediment -= depositAmount;

                        myDelta[indexNW] += (1.0f - xOffset) * (1.0f - yOffset) * depositAmount;
                        myDelta[indexNE] += xOffset * (1.0f - yOffset) * depositAmount;
                        myDelta[indexSW] += (1.0f - xOffset) * yOffset * depositAmount;
                        myDelta[indexSE] += xOffset * yOffset * depositAmount;
                    } else {
                        float erodeAmount = min((capacity - sediment) * erosionSpeed, -deltaH);
                        sediment += erodeAmount;

                        myDelta[indexNW] -= (1.0f - xOffset) * (1.0f - yOffset) * erodeAmount;
                        myDelta[indexNE] -= xOffset * (1.0f - yOffset) * erodeAmount;
                        myDelta[indexSW] -= (1.0f - xOffset) * yOffset * erodeAmount;
                        myDelta[indexSE] -= xOffset * yOffset * erodeAmount;
                    }

                    water *= (1.0f - evaporationRate);
                    posX = nextX;
                    posY = nextY;
                }
            }
        }
        auto endSim = chrono::high_resolution_clock::now();
        tSim = chrono::duration<double, milli>(endSim - startSim).count();

        // Parallel reduction back to m_data
        auto startRed = chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(runtime)
        for (int idx = 0; idx < m_size * m_size; idx++) {
            float sum = 0.0f;
            for (int t = 0; t < numThreads; t++) {
                sum += localDelta[t * m_size * m_size + idx];
            }
            m_data[idx] += sum;
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
