#include "benchmark_config.hpp"
#include "raymap_config.hpp"
#include "raylib.h"
#include "terrain.hpp"
#include "visuals.hpp"
#include <omp.h>

int main() {
    // Inicialización de Raylib
    InitWindow(screenWidth, screenHeight, "raymap");
    SetTargetFPS(60);

    // Configuración de hilos
    int threadOptions[] = {4, 8, 12};
    int threadIndex = 2;
    omp_set_num_threads(threadOptions[threadIndex]);

    // Crear objeto terreno y correr benchmark inicial
    unsigned int currentSeed = 0;
    int currentOctaves = 8;
    Terrain terrain(mapSize, currentSeed, currentOctaves);
    BenchmarkResults results = terrain.runBenchmark();

    // Cargar modelo y cámara
    Model terrainModel = terrain.createModel();
    OrbitCamera camera(cameraRadius, cameraHeight, cameraFOV);

    ErosionMode currentErosionMode = ErosionMode::SEQUENTIAL;

    while (!WindowShouldClose()) {
        bool needsUpdate = false;

        // Cambiar textura (C)
        if (IsKeyPressed(KEY_C)) {
            terrain.toggleTexture(terrainModel);
        }

        // Cambiar modo de erosión (V)
        if (IsKeyPressed(KEY_V)) {
            currentErosionMode = static_cast<ErosionMode>((static_cast<int>(currentErosionMode) + 1) % 3);
            results.erosionMode = currentErosionMode;
        }

        // Aplicar erosión (E)
        if (IsKeyPressed(KEY_E)) {
            double tErosion = terrain.applyErosion(currentErosionMode);
            results.timeErosion = tErosion;
            results.erosionMode = currentErosionMode;
            UnloadModel(terrainModel);
            terrainModel = terrain.createModel();
        }

        // Rotar hilos (X)
        if (IsKeyPressed(KEY_X)) {
            threadIndex = (threadIndex + 1) % 3;
            omp_set_num_threads(threadOptions[threadIndex]);
            needsUpdate = true;
        }

        // Cambiar semilla (Espacio - random, H - restar, L - sumar)
        if (IsKeyPressed(KEY_SPACE)) {
            currentSeed = GetRandomValue(0, 100);
            terrain.regenerate(currentSeed);
            needsUpdate = true;
        }
        if (IsKeyPressed(KEY_H)) {
            if (currentSeed > 0) {
                currentSeed--;
                terrain.regenerate(currentSeed);
                needsUpdate = true;
            }
        }
        if (IsKeyPressed(KEY_L)) {
            if (currentSeed < 100) {
                currentSeed++;
                terrain.regenerate(currentSeed);
                needsUpdate = true;
            }
        }

        // Cambiar Octavas (J - restar, K - sumar)
        if (IsKeyPressed(KEY_J)) {
            if (currentOctaves > 1) {
                currentOctaves /= 2;
                terrain.setOctaves(currentOctaves);
                needsUpdate = true;
            }
        }
        if (IsKeyPressed(KEY_K)) {
            if (currentOctaves < 8) {
                currentOctaves *= 2;
                terrain.setOctaves(currentOctaves);
                needsUpdate = true;
            }
        }

        if (needsUpdate) {
            results = terrain.runBenchmark();
            results.erosionMode = currentErosionMode;
            UnloadModel(terrainModel);
            terrainModel = terrain.createModel();
        }

        camera.update(GetFrameTime());

        // Render
        BeginDrawing();
            ClearBackground(RAYWHITE);

            camera.beginMode();
                DrawModel(terrainModel, (Vector3){-meshWidth / 2.0f, 0, -meshLength / 2.0f}, 1.0f, WHITE);
                DrawGrid(20, 10.0f);
            camera.endMode();

            // Interfaz
            DrawMetricsUI(results);
            DrawText("ESPACIO: Rand | H/L: Semilla (-/+) | J/K: Octavas (1-8) | X: Hilos | C: Color", 10,
                    screenHeight - 55, 18, DARKGRAY);
            DrawText("E: Aplicar Erosión | V: Cambiar Modo de Erosión", 10,
                    screenHeight - 30, 18, DARKGRAY);

        EndDrawing();
    }

    UnloadModel(terrainModel);
    CloseWindow();

    return 0;
}
