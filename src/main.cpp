#include "raylib.h"
#include "config.hpp"
#include "terrain.hpp"
#include "visuals.hpp"
#include <iostream>
#include <omp.h>

int main() {
    // Inicialización
    InitWindow(screenWidth, screenHeight, "raymap");
    SetTargetFPS(60);

    std::cout << "OMP Max Threads: " << omp_get_max_threads() << std::endl;

    // Lógica del Terreno y Experimentación
    // Se compara secuencial y paralelo
    Terrain terrain(mapSize, terrainSeed);
    BenchmarkResults results = terrain.runBenchmark();

    // Preparar de Visuales
    Model terrainModel = terrain.createModel();
    OrbitCamera camera(cameraRadius, cameraHeight, cameraFOV);

    // Main loop
    while (!WindowShouldClose()) {
        // Update
        camera.update(GetFrameTime());

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            camera.beginMode();
                // Terreno centrado
                DrawModel(terrainModel, (Vector3){ -meshWidth/2.0f, 0, -meshLength/2.0f }, 1.0f, WHITE);
                DrawGrid(20, 10.0f);
            camera.endMode();

            // Interfaz con resultados del benchmark
            DrawMetricsUI(results);
        EndDrawing();
    }

    UnloadModel(terrainModel);
    CloseWindow();

    return 0;
}
