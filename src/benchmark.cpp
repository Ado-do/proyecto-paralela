#include "heightmap.hpp"
#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Error: Falta el argumento del directorio de salida." << endl;
        cerr << "Uso: " << argv[0] << " <ruta_directorio_salida>" << endl;
        return 1;
    }

    string outDir = argv[1];
    // Asegurar que la ruta termine en barra diagonal
    if (!outDir.empty() && outDir.back() != '/') {
        outDir += '/';
    }

    cout << "Iniciando Raymap CPU Benchmarks (Directorio de salida: " << outDir << ")..." << endl;

    int maxThreads = omp_get_max_threads();
    vector<int> threadCounts;
    for (int t = 1; t <= maxThreads; t *= 2) {
        threadCounts.push_back(t);
    }
    if (threadCounts.back() != maxThreads) {
        threadCounts.push_back(maxThreads);
    }

    vector<int> sizes = {256, 512, 1024, 2048, 4096};

    for (int size : sizes) {
        cout << "Evaluando rendimiento para tamaño de mapa: " << size << "x" << size << endl;

        string noisePath = outDir + "benchmark_noise_" + to_string(size) + ".csv";
        string erosionPath = outDir + "benchmark_erosion_" + to_string(size) + ".csv";

        ofstream noiseFile(noisePath);
        if (!noiseFile.is_open()) {
            cerr << "Error: No se pudo abrir el archivo del benchmark de ruido: " << noisePath << endl;
            return 1;
        }
        noiseFile << "Threads,Size,SequentialTime_ms,ParallelTime_ms,Speedup,Efficiency\n";

        ofstream erosionFile(erosionPath);
        if (!erosionFile.is_open()) {
            cerr << "Error: No se pudo abrir el archivo del benchmark de erosión: " << erosionPath << endl;
            return 1;
        }
        erosionFile << "Threads,Size,Sequential_ms,Atomic_ms,LocalBuffers_ms,SpeedupAtomic,SpeedupLocal,LocalAlloc_ms,"
                       "LocalSim_ms,LocalReduc_ms\n";

        for (int threads : threadCounts) {
            omp_set_num_threads(threads);
            Heightmap heightmap(size, 42, 8);

            // Benchmark Noise
            BenchmarkResults noiseRes = heightmap.runBenchmark();
            noiseFile << threads << "," << size << "," << noiseRes.timeSequential << "," << noiseRes.timeParallel << ","
                      << noiseRes.speedup << "," << noiseRes.efficiency << "\n";

            // Benchmark Erosion
            // --- Erosión Secuencial
            // Warmup (2 pasadas)
            for (int w = 0; w < 2; w++) {
                heightmap.resetGrid();
                heightmap.applyErosion(ErosionMode::SEQUENTIAL);
            }
            // Repeticiones (5 pasadas, tomamos el promedio)
            double tSeq = 0.0;
            ErosionProfile profSeq = {0.0, 0.0, 0.0, 0.0};
            for (int r = 0; r < 5; r++) {
                heightmap.resetGrid();
                ErosionProfile prof;
                double t = heightmap.applyErosion(ErosionMode::SEQUENTIAL, &prof);
                tSeq += t;
                profSeq.totalTime += prof.totalTime;
                profSeq.allocationTime += prof.allocationTime;
                profSeq.simulationTime += prof.simulationTime;
                profSeq.reductionTime += prof.reductionTime;
            }
            tSeq /= 5.0;
            profSeq.totalTime /= 5.0;
            profSeq.allocationTime /= 5.0;
            profSeq.simulationTime /= 5.0;
            profSeq.reductionTime /= 5.0;

            // --- Erosión Paralela Atómica
            // Warmup (2 pasadas)
            for (int w = 0; w < 2; w++) {
                heightmap.resetGrid();
                heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC);
            }
            // Repeticiones (5 pasadas, tomamos el promedio)
            double tAtomic = 0.0;
            ErosionProfile profAtomic = {0.0, 0.0, 0.0, 0.0};
            for (int r = 0; r < 5; r++) {
                heightmap.resetGrid();
                ErosionProfile prof;
                double t = heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC, &prof);
                tAtomic += t;
                profAtomic.totalTime += prof.totalTime;
                profAtomic.allocationTime += prof.allocationTime;
                profAtomic.simulationTime += prof.simulationTime;
                profAtomic.reductionTime += prof.reductionTime;
            }
            tAtomic /= 5.0;
            profAtomic.totalTime /= 5.0;
            profAtomic.allocationTime /= 5.0;
            profAtomic.simulationTime /= 5.0;
            profAtomic.reductionTime /= 5.0;

            // --- Erosión Paralela Local Buffers
            // Warmup (2 pasadas)
            for (int w = 0; w < 2; w++) {
                heightmap.resetGrid();
                heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);
            }
            // Repeticiones (5 pasadas, tomamos el promedio)
            double tLocal = 0.0;
            ErosionProfile profLocal = {0.0, 0.0, 0.0, 0.0};
            for (int r = 0; r < 5; r++) {
                heightmap.resetGrid();
                ErosionProfile prof;
                double t = heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS, &prof);
                tLocal += t;
                profLocal.totalTime += prof.totalTime;
                profLocal.allocationTime += prof.allocationTime;
                profLocal.simulationTime += prof.simulationTime;
                profLocal.reductionTime += prof.reductionTime;
            }
            tLocal /= 5.0;
            profLocal.totalTime /= 5.0;
            profLocal.allocationTime /= 5.0;
            profLocal.simulationTime /= 5.0;
            profLocal.reductionTime /= 5.0;

            double speedupAtomic = tSeq / tAtomic;
            double speedupLocal = tSeq / tLocal;

            erosionFile << threads << "," << size << "," << tSeq << "," << tAtomic << "," << tLocal << ","
                        << speedupAtomic << "," << speedupLocal << "," << profLocal.allocationTime << ","
                        << profLocal.simulationTime << "," << profLocal.reductionTime << "\n";

            cout << "  Hilos: " << threads << " | Speedup de Ruido: " << noiseRes.speedup << "x\n"
                 << "    Erosión (Atómico): Total " << tAtomic << " ms (Sim: " << profAtomic.simulationTime << " ms)\n"
                 << "    Erosión (Local):   Total " << tLocal << " ms (Asig: " << profLocal.allocationTime
                 << " ms, Sim: " << profLocal.simulationTime << " ms, Reduc: " << profLocal.reductionTime << " ms)"
                 << endl;
        }
        noiseFile.close();
        erosionFile.close();
    }

    cout << "¡Benchmarks finalizados con éxito! Datos guardados en " << outDir << endl;
    return 0;
}
