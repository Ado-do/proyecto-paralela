#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <omp.h>
#include "heightmap.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Error: Missing output directory argument." << endl;
        cerr << "Usage: " << argv[0] << " <output_directory_path>" << endl;
        return 1;
    }
    
    string outDir = argv[1];
    // Asegurar que la ruta termine en barra diagonal
    if (!outDir.empty() && outDir.back() != '/') {
        outDir += '/';
    }

    cout << "Starting Raymap CPU Benchmarks (Output directory: " << outDir << ")..." << endl;
    
    int maxThreads = omp_get_max_threads();
    vector<int> threadCounts;
    for (int t = 1; t <= maxThreads; t *= 2) {
        threadCounts.push_back(t);
    }
    if (threadCounts.back() != maxThreads) {
        threadCounts.push_back(maxThreads);
    }
    
    vector<int> sizes = {256, 512, 1024};

    for (int size : sizes) {
        cout << "Benchmarking map size: " << size << "x" << size << endl;
        
        string noisePath = outDir + "benchmark_noise_" + to_string(size) + ".csv";
        string erosionPath = outDir + "benchmark_erosion_" + to_string(size) + ".csv";
        
        ofstream noiseFile(noisePath);
        if (!noiseFile.is_open()) {
            cerr << "Error: Could not open noise benchmark file: " << noisePath << endl;
            return 1;
        }
        noiseFile << "Threads,Size,SequentialTime_ms,ParallelTime_ms,Speedup,Efficiency\n";

        ofstream erosionFile(erosionPath);
        if (!erosionFile.is_open()) {
            cerr << "Error: Could not open erosion benchmark file: " << erosionPath << endl;
            return 1;
        }
        erosionFile << "Threads,Size,Sequential_ms,Atomic_ms,LocalBuffers_ms,SpeedupAtomic,SpeedupLocal\n";

        for (int threads : threadCounts) {
            omp_set_num_threads(threads);
            Heightmap heightmap(size, 42, 8);
            
            // Benchmark Noise
            BenchmarkResults noiseRes = heightmap.runBenchmark();
            noiseFile << threads << "," 
                      << size << "," 
                      << noiseRes.timeSequential << "," 
                      << noiseRes.timeParallel << "," 
                      << noiseRes.speedup << "," 
                      << noiseRes.efficiency << "\n";

            // Benchmark Erosion
            heightmap.runBenchmark(); 
            
            double tSeq = heightmap.applyErosion(ErosionMode::SEQUENTIAL);
            
            heightmap.runBenchmark();
            double tAtomic = heightmap.applyErosion(ErosionMode::PARALLEL_ATOMIC);
            
            heightmap.runBenchmark();
            double tLocal = heightmap.applyErosion(ErosionMode::PARALLEL_LOCAL_BUFFERS);

            double speedupAtomic = tSeq / tAtomic;
            double speedupLocal = tSeq / tLocal;

            erosionFile << threads << ","
                        << size << ","
                        << tSeq << ","
                        << tAtomic << ","
                        << tLocal << ","
                        << speedupAtomic << ","
                        << speedupLocal << "\n";
            
            cout << "  Threads: " << threads 
                 << " | Noise Speedup: " << noiseRes.speedup << "x"
                 << " | Erosion Atomic: " << tAtomic << "ms"
                 << " | Erosion Local: " << tLocal << "ms" << endl;
        }
        noiseFile.close();
        erosionFile.close();
    }

    cout << "Benchmarks completed successfully! Data saved to " << outDir << endl;
    return 0;
}
