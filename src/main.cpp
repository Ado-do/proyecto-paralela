#include "terrain_config.hpp"
#include "raymap_config.hpp"
#include "raylib.h"
#include "terrain.hpp"
#include "visuals.hpp"

#include <algorithm>
#include <omp.h>
#include <ctime>

#define MSF_GIF_IMPL
#include "msf_gif.h"

int main() {
    // Inicialización de Raylib con soporte para ventana redimensionable
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
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
    Terrain terrain(defaultMapSize, currentSeed, currentOctaves);
    BenchmarkResults results = terrain.runBenchmark(true);

    // Cargar modelo y cámara
    Model terrainModel = terrain.createModel();
    OrbitCamera camera(cameraRadius, cameraHeight, cameraFOV);

    ErosionMode currentErosionMode = ErosionMode::PARALLEL_ATOMIC;
    bool drawWireframe = false;
    bool showControls = true;
    bool showMetrics = true;
    bool is2DMode = false;
    float uiScaleModifier = 1.0f;

    // Variables para la grabacion de pantalla en GIF
    bool gifRecording = false;
    unsigned int gifFrameCounter = 0;
    MsfGifState gifState = { 0 };
    int gifRecordWidth = 0;
    int gifRecordHeight = 0;

    while (!WindowShouldClose()) {
        bool needsUpdate = false;

        // Calcular uiScale adaptativo al tamaño de la pantalla actual
        float screenW = (float)GetScreenWidth();
        float screenH = (float)GetScreenHeight();
        float baseScale = std::min(screenW / 1280.0f, screenH / 720.0f);
        float uiScale = baseScale * uiScaleModifier;

        // Alternar grabacion de pantalla en GIF (CTRL-R)
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_R)) {
            if (!gifRecording) {
                gifRecordHeight = 480;
                gifRecordWidth = (int)(((float)GetScreenWidth() / (float)GetScreenHeight()) * 480.0f);
                if (gifRecordWidth % 2 != 0) gifRecordWidth++; // Asegurar ancho par

                int beginRet = msf_gif_begin(&gifState, gifRecordWidth, gifRecordHeight);
                if (beginRet) {
                    gifRecording = true;
                    gifFrameCounter = 0;
                    TraceLog(LOG_INFO, "GIF: Grabacion iniciada (rescalado a 480p): %dx%d", gifRecordWidth, gifRecordHeight);
                } else {
                    TraceLog(LOG_ERROR, "GIF: Error al iniciar la grabacion");
                }
            } else {
                gifRecording = false;
                MsfGifResult result = msf_gif_end(&gifState);
                if (result.data) {
                    time_t rawTime;
                    time(&rawTime);
                    struct tm* timeInfo = localtime(&rawTime);
                    char fileName[64];
                    strftime(fileName, sizeof(fileName), "raymap_%Y%m%d_%H%M%S.gif", timeInfo);

                    SaveFileData(fileName, result.data, (int)result.dataSize);
                    TraceLog(LOG_INFO, "GIF: Grabacion guardada en %s (%d bytes)", fileName, (int)result.dataSize);
                    msf_gif_free(result);
                } else {
                    TraceLog(LOG_WARNING, "GIF: No hay datos grabados");
                }
            }
        }

        // Cambiar textura (C)
        if (IsKeyPressed(KEY_C)) {
            terrain.toggleTexture(terrainModel);
        }

        // Alternar entre vista 3D y vista 2D del ruido (M)
        if (IsKeyPressed(KEY_M)) {
            is2DMode = !is2DMode;
        }

        // Cambiar tamaño de la interfaz (+ / -)
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
            uiScaleModifier = std::min(uiScaleModifier + 0.1f, 2.0f);
        }
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
            uiScaleModifier = std::max(uiScaleModifier - 0.1f, 0.5f);
        }

        // Alternar malla de alambre (Z)
        if (IsKeyPressed(KEY_Z)) {
            drawWireframe = !drawWireframe;
        }

        // Alternar rotación automática de la cámara (P)
        if (IsKeyPressed(KEY_P)) {
            camera.toggleAutoRotate();
        }

        // Alternar visualización del panel de controles (TAB)
        if (IsKeyPressed(KEY_TAB)) {
            showControls = !showControls;
        }

        // Alternar visualización del panel de métricas (`)
        if (IsKeyPressed(KEY_GRAVE)) {
            showMetrics = !showMetrics;
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

        if (!is2DMode) {
            camera.update(GetFrameTime());
        }

        // Render
        BeginDrawing();
            ClearBackground(colorBackground); // Fondo oscuro técnico

            if (!is2DMode) {
                camera.beginMode();
                    // Dibujar terreno sólido
                    DrawModel(terrainModel, (Vector3){-meshWidth / 2.0f, 0, -meshLength / 2.0f}, 1.0f, WHITE);

                    // Superponer malla de alambre si está activa (con pequeño Y offset para evitar Z-fighting)
                    if (drawWireframe) {
                        float y_offset = 0.1f;
                        DrawModelWires(terrainModel, (Vector3){-meshWidth / 2.0f, y_offset, -meshLength / 2.0f}, 1.0f, colorWireframe);
                    }

                    // Dibujar plano de agua semi-transparente solo en modo color/realista
                    if (terrain.isUsingColor()) {
                        Vector3 waterPos = { 0.0f, (heightShallowWater + 1.0f) * 0.5f * meshHeight, 0.0f };
                        DrawCube(waterPos, meshWidth, 0.1f, meshLength, colorWaterVolume);
                    }

                    DrawGrid(20, 10.0f);
                camera.endMode();
            } else {
                // Dibujar vista 2D del mapa de alturas
                int screenW = GetScreenWidth();
                int screenH = GetScreenHeight();

                // Calcular dimensiones adaptativas para no solaparse con paneles superior, inferior o laterales
                float margin = 10.0f * baseScale;
                float titleH = 40.0f * baseScale;
                float bottomH = showControls ? (115.0f * baseScale) : 0.0f;
                float spacing = 40.0f * baseScale;

                float maxTexW = (screenW - 2.0f * margin - spacing) / 2.0f;
                float maxTexH = screenH - 2.0f * margin - titleH - bottomH - 40.0f * baseScale; // 40.0f para margen de texto
                float texSize = std::min(maxTexW, maxTexH);
                if (texSize < 100.0f) texSize = 100.0f;

                int totalWidth = (int)(texSize * 2.0f + spacing);
                int startX = (screenW - totalWidth) / 2;
                int startY = (int)(margin + titleH + (screenH - margin - titleH - bottomH - texSize) / 2.0f);

                // 1. Escala de grises (Dibujado escalado)
                Rectangle srcRec = { 0.0f, 0.0f, (float)terrain.getTexDebug().width, (float)terrain.getTexDebug().height };
                Rectangle destRec1 = { (float)startX, (float)startY, texSize, texSize };
                DrawTexturePro(terrain.getTexDebug(), srcRec, destRec1, (Vector2){0,0}, 0.0f, WHITE);
                DrawRectangleLines(startX - 2, startY - 2, (int)texSize + 4, (int)texSize + 4, uiBorderColor);
                DrawText("Ruido / Alturas (Escala de Grises)", startX, startY - (int)(25.0f * baseScale), (int)(16.0f * baseScale), SKYBLUE);

                // 2. Colorizado (Dibujado escalado)
                Rectangle destRec2 = { (float)(startX + texSize + spacing), (float)startY, texSize, texSize };
                DrawTexturePro(terrain.getTexColor(), srcRec, destRec2, (Vector2){0,0}, 0.0f, WHITE);
                DrawRectangleLines(startX + (int)texSize + (int)spacing - 2, startY - 2, (int)texSize + 4, (int)texSize + 4, uiBorderColor);
                DrawText("Biomas / Terreno (Colorizado)", startX + (int)texSize + (int)spacing, startY - (int)(25.0f * baseScale), (int)(16.0f * baseScale), SKYBLUE);
            }

            // Interfaz
            DrawInterface(results, terrain.isUsingColor(), drawWireframe, camera.isAutoRotate(), showControls, showMetrics, is2DMode, uiScale);

            // Dibujar indicador de grabacion GIF si esta activo
            if (gifRecording) {
                float titlePanelW = 200.0f * uiScale;
                float titlePanelH = 40.0f * uiScale;
                float titlePanelX = (screenW - titlePanelW) / 2.0f;
                float titlePanelY = 4.0f;

                float recX = titlePanelX + titlePanelW + 10.0f * uiScale;
                float recY = titlePanelY + (titlePanelH - 16 * uiScale) / 2.0f;

                bool blink = ((int)(GetTime() * 2) % 2 == 0);
                if (blink) {
                    DrawCircle((int)recX, (int)(recY + 8 * uiScale), (int)(6 * uiScale), RED);
                } else {
                    DrawCircle((int)recX, (int)(recY + 8 * uiScale), (int)(6 * uiScale), MAROON);
                }
                DrawText("REC", (int)(recX + 12 * uiScale), (int)recY, (int)(16 * uiScale), RED);
            }

            // Dibujar FPS dinámicamente si las métricas están ocultas
            if (!showMetrics) {
                DrawFPS((int)(GetScreenWidth() - 80 * uiScale), (int)(10 * uiScale));
            }
        EndDrawing();

        // Capturar frame para el GIF (cada 6 frames, aprox. 10 FPS a 60 FPS objetivo)
        if (gifRecording) {
            gifFrameCounter++;
            if ((gifFrameCounter % 6) == 0) {
                Image imScreen = LoadImageFromScreen();
                // Redimensionar el frame a 480p de alto antes de enviarlo al codificador
                ImageResize(&imScreen, gifRecordWidth, gifRecordHeight);
                msf_gif_frame(&gifState, (uint8_t*)imScreen.data, 10, 16, imScreen.width * 4); // 10cs = 100ms
                UnloadImage(imScreen);
            }
        }
    }

    UnloadModel(terrainModel);
    CloseWindow();

    return 0;
}
