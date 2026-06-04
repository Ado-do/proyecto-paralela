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
    Terrain terrain(mapSize, terrainSeed);
    BenchmarkResults results = terrain.runBenchmark();

    // Preparar Visuales
    Model terrainModel = terrain.createModel();
    OrbitCamera camera(cameraRadius, cameraHeight, cameraFOV);

    // Main loop
    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_C)) {
            terrain.toggleTexture(terrainModel);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            // Nueva semilla
            terrain.regenerate(GetRandomValue(0, 100000));
            
            // Recalcular métricas
            results = terrain.runBenchmark();
            
            // Actualizar modelo 3D
            UnloadModel(terrainModel);
            terrainModel = terrain.createModel();
        }

        camera.update(GetFrameTime());

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            camera.beginMode();
                DrawModel(terrainModel, (Vector3){ -meshWidth/2.0f, 0, -meshLength/2.0f }, 1.0f, WHITE);
                DrawGrid(20, 10.0f);
            camera.endMode();

            // Interfaz
            DrawMetricsUI(results);
            DrawText("ESPACIO: Randomizar | C: Visualización", 
                     uiIndicatorMarginLeft, 
                     screenHeight - uiIndicatorMarginBottom, 
                     uiIndicatorFontSize, 
                     DARKGRAY);
        EndDrawing();
    }

    UnloadModel(terrainModel);
    CloseWindow();

    return 0;
}
