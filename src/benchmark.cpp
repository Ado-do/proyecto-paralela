#include "heightmap.hpp"
#include "benchmark_config.hpp"
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

    for (int size : benchmarkSizes) {
        cout << "Evaluando rendimiento para tamaño de mapa: " << size << "x" << size << endl;

        string noisePath = outDir + "benchmark_noise_" + to_string(size) + ".csv";
        string erosionPath = outDir + "benchmark_erosion_" + to_string(size) + ".csv";

        ofstream noiseFile(noisePath);
        if (!noiseFile.is_open()) {
            cerr << "Error: No se pudo abrir el archivo del benchmark de ruido: " << noisePath << endl;
            return 1;
        }
        noiseFile << "Threads,Size,Scheduler,SequentialTime_ms,ParallelTime_ms,Speedup,Efficiency\n";
 
        ofstream erosionFile(erosionPath);
        if (!erosionFile.is_open()) {
            cerr << "Error: No se pudo abrir el archivo del benchmark de erosión: " << erosionPath << endl;
            return 1;
        }
        erosionFile << "Threads,Size,Scheduler,Sequential_ms,Atomic_ms,LocalBuffers_ms,SpeedupAtomic,SpeedupLocal,LocalAlloc_ms,"
                       "LocalSim_ms,LocalReduc_ms\n";
 
        struct SchedConfig {
            omp_sched_t type;
            string name;
        };
        vector<SchedConfig> schedulers = {
            {omp_sched_static, "static"},
            {omp_sched_dynamic, "dynamic"}
        };

        for (int threads : threadCounts) {
            omp_set_num_threads(threads);

            for (const auto& sched : schedulers) {
                omp_set_schedule(sched.type, 0);
                Heightmap heightmap(size, 42, 8);

                // Benchmark Noise
                BenchmarkResults noiseRes = heightmap.runBenchmark();
                noiseFile << threads << "," << size << "," << sched.name << "," << noiseRes.timeSequential << "," << noiseRes.timeParallel << ","
                          << noiseRes.speedup << "," << noiseRes.efficiency << "\n";

                // Benchmark Erosion
                // --- Erosión Secuencial
                // Warmup
                for (int w = 0; w < benchmarkWarmups; w++) {
                    heightmap.resetGrid();
                    heightmap.applyErosion(ErosionMode::SEQUENTIAL);
                }
                // Repeticiones (promedio)
                double tSeq = 0.0;
                ErosionProfile profSeq = {0.0, 0.0, 0.0, 0.0};
                for (int r = 0; r < benchmarkRuns; r++) {
                    heightmap.resetGrid();
                    ErosionProfile prof;
                    double t = heightmap.applyErosion(ErosionMode::SEQUENTIAL, &prof);
                    tSeq += t;
                    profSeq.totalTime += prof.totalTime;
                    profSeq.allocationTime += prof.allocationTime;
                    profSeq.simulationTime += prof.simulationTime;
                    profSeq.reductionTime += prof.reductionTime;
                }
                tSeq /= (double)benchmarkRuns;
                profSeq.totalTime /= (double)benchmarkRuns;
                profSeq.allocationTime /= (double)benchmarkRuns;
                profSeq.simulationTime /= (double)benchmarkRuns;
                profSeq.reductionTime /= (double)benchmarkRuns;

                // --- Erosión Paralela Atómica
                // Warmup
                for (int w = 0; w < benchmarkWarmups; w++) {
                    heightmap.resetGrid();
                    heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC);
                }
                // Repeticiones (promedio)
                double tAtomic = 0.0;
                ErosionProfile profAtomic = {0.0, 0.0, 0.0, 0.0};
                for (int r = 0; r < benchmarkRuns; r++) {
                    heightmap.resetGrid();
                    ErosionProfile prof;
                    double t = heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC, &prof);
                    tAtomic += t;
                    profAtomic.totalTime += prof.totalTime;
                    profAtomic.allocationTime += prof.allocationTime;
                    profAtomic.simulationTime += prof.simulationTime;
                    profAtomic.reductionTime += prof.reductionTime;
                }
                tAtomic /= (double)benchmarkRuns;
                profAtomic.totalTime /= (double)benchmarkRuns;
                profAtomic.allocationTime /= (double)benchmarkRuns;
                profAtomic.simulationTime /= (double)benchmarkRuns;
                profAtomic.reductionTime /= (double)benchmarkRuns;

                // --- Erosión Paralela Local Buffers
                // Warmup
                for (int w = 0; w < benchmarkWarmups; w++) {
                    heightmap.resetGrid();
                    heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);
                }
                // Repeticiones (promedio)
                double tLocal = 0.0;
                ErosionProfile profLocal = {0.0, 0.0, 0.0, 0.0};
                for (int r = 0; r < benchmarkRuns; r++) {
                    heightmap.resetGrid();
                    ErosionProfile prof;
                    double t = heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS, &prof);
                    tLocal += t;
                    profLocal.totalTime += prof.totalTime;
                    profLocal.allocationTime += prof.allocationTime;
                    profLocal.simulationTime += prof.simulationTime;
                    profLocal.reductionTime += prof.reductionTime;
                }
                tLocal /= (double)benchmarkRuns;
                profLocal.totalTime /= (double)benchmarkRuns;
                profLocal.allocationTime /= (double)benchmarkRuns;
                profLocal.simulationTime /= (double)benchmarkRuns;
                profLocal.reductionTime /= (double)benchmarkRuns;

                double speedupAtomic = tSeq / tAtomic;
                double speedupLocal = tSeq / tLocal;

                erosionFile << threads << "," << size << "," << sched.name << "," << tSeq << "," << tAtomic << "," << tLocal << ","
                            << speedupAtomic << "," << speedupLocal << "," << profLocal.allocationTime << ","
                            << profLocal.simulationTime << "," << profLocal.reductionTime << "\n";

                cout << "  Hilos: " << threads << " | Planificador: " << sched.name << " | Speedup de Ruido: " << noiseRes.speedup << "x\n"
                     << "    Erosión (Atómico): Total " << tAtomic << " ms (Sim: " << profAtomic.simulationTime << " ms)\n"
                     << "    Erosión (Local):   Total " << tLocal << " ms (Asig: " << profLocal.allocationTime
                     << " ms, Sim: " << profLocal.simulationTime << " ms, Reduc: " << profLocal.reductionTime << " ms)"
                     << endl;
            }
        }
        noiseFile.close();
        erosionFile.close();
    }

    cout << "¡Benchmarks finalizados con éxito! Datos guardados en " << outDir << endl;
    return 0;
}
