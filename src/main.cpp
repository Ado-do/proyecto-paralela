#include "benchmark_config.hpp"
#include "raymap_config.hpp"
#include "raylib.h"
#include "terrain.hpp"
#include "visuals.hpp"

#include <algorithm>
#include <omp.h>

int main() {
    // Inicialización de Raylib
    InitWindow(screenWidth, screenHeight, "raymap");
    SetTargetFPS(60);

    // Configuración de hilos dinámica
    int maxThreads = omp_get_max_threads();
    std::vector<int> threadOptions;
    if (maxThreads <= 4) {
        threadOptions = {1, 2, maxThreads};
    } else if (maxThreads <= 8) {
        threadOptions = {2, 4, maxThreads};
    } else {
        threadOptions = {4, maxThreads / 2, maxThreads};
    }
    // Eliminar duplicados si los hay
    threadOptions.erase(std::unique(threadOptions.begin(), threadOptions.end()), threadOptions.end());
    int threadIndex = threadOptions.size() - 1;
    omp_set_num_threads(threadOptions[threadIndex]);

    // Crear objeto terreno y correr benchmark inicial en modo rápido
    unsigned int currentSeed = 0;
    int currentOctaves = 8;
    Terrain terrain(mapSize, currentSeed, currentOctaves);
    BenchmarkResults results = terrain.runBenchmark(true);

    // Cargar modelo y cámara
    Model terrainModel = terrain.createModel();
    OrbitCamera camera(cameraRadius, cameraHeight, cameraFOV);

    ErosionMode currentErosionMode = ErosionMode::SEQUENTIAL;
    bool drawWireframe = false;

    while (!WindowShouldClose()) {
        bool needsUpdate = false;

        // Cambiar textura (C)
        if (IsKeyPressed(KEY_C)) {
            terrain.toggleTexture(terrainModel);
        }

        // Alternar malla de alambre (Z)
        if (IsKeyPressed(KEY_Z)) {
            drawWireframe = !drawWireframe;
        }

        // Alternar rotación automática de la cámara (P)
        if (IsKeyPressed(KEY_P)) {
            camera.toggleAutoRotate();
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
            threadIndex = (threadIndex + 1) % threadOptions.size();
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
            results = terrain.runBenchmark(true);
            results.erosionMode = currentErosionMode;
            UnloadModel(terrainModel);
            terrainModel = terrain.createModel();
        }

        camera.update(GetFrameTime());

        // Render
        BeginDrawing();
            ClearBackground((Color){ 18, 18, 24, 255 }); // Fondo oscuro técnico

            camera.beginMode();
                // Dibujar terreno sólido
                DrawModel(terrainModel, (Vector3){-meshWidth / 2.0f, 0, -meshLength / 2.0f}, 1.0f, WHITE);

                // Superponer malla de alambre si está activa (con pequeño Y offset para evitar Z-fighting)
                if (drawWireframe) {
                    DrawModelWires(terrainModel, (Vector3){-meshWidth / 2.0f, 0.1f, -meshLength / 2.0f}, 1.0f, (Color){0, 0, 0, 90});
                }

                // Dibujar plano de agua semi-transparente
                Vector3 waterPos = { 0.0f, (heightShallowWater + 1.0f) * 0.5f * meshHeight, 0.0f };
                DrawCube(waterPos, meshWidth, 0.1f, meshLength, (Color){40, 90, 220, 140});

                DrawGrid(20, 10.0f);
            camera.endMode();

            // Interfaz
            DrawInterface(results, terrain.isUsingColor(), drawWireframe, camera.isAutoRotate());

            DrawFPS(GetScreenWidth() - 80, 10);
        EndDrawing();
    }

    UnloadModel(terrainModel);
    CloseWindow();

    return 0;
}
