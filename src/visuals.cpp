#include "visuals.hpp"
#include "raymap_config.hpp"
#include <cmath>

OrbitCamera::OrbitCamera(float radius, float height, float fov) 
    : m_radius(radius), m_height(height), m_angle(0.0f), m_autoRotate(true) {
    m_camera = { 0 };
    m_camera.position = (Vector3){ radius, height, radius };
    m_camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    m_camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    m_camera.fovy = fov;
    m_camera.projection = CAMERA_PERSPECTIVE;
}

void OrbitCamera::update(float dt) {
    // Rotar automáticamente si está activo
    if (m_autoRotate) {
        m_angle += cameraOrbitSpeed * dt;
    }

    // Controles de rotación interactiva con botón derecho del mouse
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mouseDelta = GetMouseDelta();
        m_angle += mouseDelta.x * 0.005f;
        m_height += mouseDelta.y * 0.2f;
        if (m_height < 10.0f) m_height = 10.0f;
        if (m_height > 180.0f) m_height = 180.0f;
    }

    // Zoom interactivo con la rueda del ratón
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        m_radius -= wheel * 5.0f;
        if (m_radius < 20.0f) m_radius = 20.0f;
        if (m_radius > 300.0f) m_radius = 300.0f;
    }

    m_camera.position.x = cos(m_angle) * m_radius;
    m_camera.position.z = sin(m_angle) * m_radius;
    m_camera.position.y = m_height;
}

void OrbitCamera::beginMode() {
    BeginMode3D(m_camera);
}

void OrbitCamera::endMode() {
    EndMode3D();
}

void DrawInterface(const BenchmarkResults& results, bool usingColor, bool drawWireframe, bool autoRotate, bool showControls, bool is2DMode, float uiScale) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // 1. PANEL IZQUIERDO: Métricas de Rendimiento y Erosión
    float panelW = 230.0f * uiScale;
    float panelH = 280.0f * uiScale;
    DrawRectangleRounded((Rectangle){ 10, 10, panelW, panelH }, 0.05f, 4, uiPanelColor);
    DrawRectangleRoundedLines((Rectangle){ 10, 10, panelW, panelH }, 0.05f, 4, uiBorderColor);

    float startX = 10.0f + 15.0f * uiScale;
    float y = 10.0f + 8.0f * uiScale;

    int fontSizeTitle = (int)(16.0f * uiScale);
    int fontSizeStandard = (int)(14.0f * uiScale);
    int fontSizeSmall = (int)(13.0f * uiScale);
    int fontSizeTiny = (int)(12.0f * uiScale);
    int fontSizeLarge = (int)(15.0f * uiScale);

    DrawText("Métricas de Rendimiento", (int)startX, (int)y, fontSizeTitle, SKYBLUE); y += 30.0f * uiScale;
    DrawText(TextFormat("Hilos Activos: %d", results.numThreads), (int)startX, (int)y, fontSizeStandard, WHITE); y += 20.0f * uiScale;
    DrawText(TextFormat("Secuencial:   %.2f ms", results.timeSequential), (int)startX, (int)y, fontSizeStandard, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Paralelo:     %.2f ms", results.timeParallel), (int)startX, (int)y, fontSizeStandard, LIGHTGRAY); y += 22.0f * uiScale;
    
    DrawText("Aceleración (Speedup):", (int)startX, (int)y, fontSizeStandard, LIME);
    DrawText(TextFormat("%.2fx", results.speedup), (int)(startX + 170.0f * uiScale), (int)y, fontSizeLarge, LIME); y += 20.0f * uiScale;
    DrawText("Eficiencia:", (int)startX, (int)y, fontSizeStandard, LIME);
    DrawText(TextFormat("%.1f%%", results.efficiency), (int)(startX + 170.0f * uiScale), (int)y, fontSizeStandard, LIME); y += 20.0f * uiScale;
    DrawText(TextFormat("Costo (p * Tp): %.2f ms", results.cost), (int)startX, (int)y, fontSizeSmall, ORANGE); y += 35.0f * uiScale;

    DrawText("Métricas de Erosión", (int)startX, (int)y, fontSizeTitle, SKYBLUE); y += 30.0f * uiScale;
    const char* modeStr = (results.erosionMode == ErosionMode::SEQUENTIAL) ? "Secuencial" :
                          (results.erosionMode == ErosionMode::PARALLEL_ATOMIC) ? "Paralelo (Atómico)" :
                          "Paralelo (Locales)";
    DrawText(TextFormat("Modo:   %s", modeStr), (int)startX, (int)y, fontSizeSmall, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Tiempo: %.2f ms", results.timeErosion), (int)startX, (int)y, fontSizeStandard, (results.timeErosion > 0) ? GOLD : LIGHTGRAY);

    // 2. PANEL DERECHO: Parámetros del Terreno y Visualización
    float rightPanelX = (float)screenW - panelW - 10.0f;
    DrawRectangleRounded((Rectangle){ rightPanelX, 10, panelW, panelH }, 0.05f, 4, uiPanelColor);
    DrawRectangleRoundedLines((Rectangle){ rightPanelX, 10, panelW, panelH }, 0.05f, 4, uiBorderColor);

    float startRightX = rightPanelX + 15.0f * uiScale;
    y = 10.0f + 15.0f * uiScale;
    DrawText("Parámetros del Terreno", (int)startRightX, (int)y, fontSizeTitle, SKYBLUE); y += 30.0f * uiScale;
    DrawText(TextFormat("Semilla:      %u", results.seed), (int)startRightX, (int)y, fontSizeStandard, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Escala:       %.2f", results.scale), (int)startRightX, (int)y, fontSizeStandard, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Octavas:      %d", results.octaves), (int)startRightX, (int)y, fontSizeStandard, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Persistencia: %.2f", results.persistence), (int)startRightX, (int)y, fontSizeStandard, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Lacunaridad:  %.2f", results.lacunarity), (int)startRightX, (int)y, fontSizeStandard, LIGHTGRAY); y += 35.0f * uiScale;

    DrawText("Estado Visual", (int)startRightX, (int)y, fontSizeTitle, SKYBLUE); y += 30.0f * uiScale;
    DrawText(TextFormat("Vista:   %s", is2DMode ? "2D Ruido" : "3D Terreno"), (int)startRightX, (int)y, fontSizeSmall, SKYBLUE); y += 20.0f * uiScale;
    DrawText(TextFormat("Textura: %s", usingColor ? "Realista (Color)" : "Depuración (Grises)"), (int)startRightX, (int)y, fontSizeSmall, LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Malla (3D): %s", drawWireframe ? "Sólido + Alambre" : "Sólido"), (int)startRightX, (int)y, fontSizeSmall, is2DMode ? DARKGRAY : LIGHTGRAY); y += 20.0f * uiScale;
    DrawText(TextFormat("Cámara (3D): %s", autoRotate ? "Rotación Activa" : "Cámara Manual"), (int)startRightX, (int)y, fontSizeSmall, is2DMode ? DARKGRAY : LIGHTGRAY);

    // 3. PANEL INFERIOR: Controles agrupados
    if (showControls) {
        float bottomH = 100.0f * uiScale;
        if (uiScale > 1.2f) {
            bottomH = 110.0f * uiScale;
        }
        DrawRectangleRounded((Rectangle){ 10, (float)(screenH - bottomH - 10.0f), (float)(screenW - 20), bottomH }, 0.15f, 4, uiPanelColor);
        DrawRectangleRoundedLines((Rectangle){ 10, (float)(screenH - bottomH - 10.0f), (float)(screenW - 20), bottomH }, 0.15f, 4, uiBorderColor);

        float colY = (float)screenH - bottomH + 1.0f * uiScale;
        float colWidth = (float)(screenW - 40) / 4.0f;

        // Columna 1: Terreno
        float colX = 25.0f;
        DrawText("[TERRENO]", (int)colX, (int)colY, fontSizeSmall, SKYBLUE);
        DrawText("ESPACIO: Semilla aleatoria", (int)colX, (int)(colY + 20.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("H / L: Semilla (- / +)", (int)colX, (int)(colY + 38.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("J / K: Cant. Octavas", (int)colX, (int)(colY + 56.0f * uiScale), fontSizeTiny, LIGHTGRAY);

        // Columna 2: Erosión e Hilos
        colX += colWidth;
        DrawText("[EROSIÓN E HILOS]", (int)colX, (int)colY, fontSizeSmall, SKYBLUE);
        DrawText("E: Aplicar Erosión", (int)colX, (int)(colY + 20.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("V: Cambiar modo de erosión", (int)colX, (int)(colY + 38.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("X: Alternar cantidad de hilos", (int)colX, (int)(colY + 56.0f * uiScale), fontSizeTiny, LIGHTGRAY);

        // Columna 3: Visualización
        colX += colWidth;
        DrawText("[VISUALIZACIÓN]", (int)colX, (int)colY, fontSizeSmall, SKYBLUE);
        DrawText("M: Alternar Vista 2D/3D", (int)colX, (int)(colY + 20.0f * uiScale), fontSizeTiny, SKYBLUE);
        DrawText("C: Alternar textura (3D)", (int)colX, (int)(colY + 38.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("Z: Alternar malla (3D)", (int)colX, (int)(colY + 56.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("P: Pausar cámara (3D)", (int)colX, (int)(colY + 74.0f * uiScale), fontSizeTiny, LIGHTGRAY);

        // Columna 4: Navegación e Interfaz
        colX += colWidth;
        DrawText("[NAVEGACIÓN E INTERFAZ]", (int)colX, (int)colY, fontSizeSmall, SKYBLUE);
        DrawText("Clic-Der: Orbitar (3D)", (int)colX, (int)(colY + 20.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("Rueda: Zoom (3D)", (int)colX, (int)(colY + 38.0f * uiScale), fontSizeTiny, LIGHTGRAY);
        DrawText("+ / -: Escalar Interfaz", (int)colX, (int)(colY + 56.0f * uiScale), fontSizeTiny, SKYBLUE);
        DrawText("TAB: Ocultar panel ayuda", (int)colX, (int)(colY + 74.0f * uiScale), fontSizeTiny, SKYBLUE);
    } else {
        // Pequeño indicador para volver a abrir el panel
        float indicatorW = 180.0f * uiScale;
        float indicatorH = 30.0f * uiScale;
        DrawRectangleRounded((Rectangle){ 10, (float)(screenH - indicatorH - 10.0f), indicatorW, indicatorH }, 0.15f, 4, uiPanelColor);
        DrawRectangleRoundedLines((Rectangle){ 10, (float)(screenH - indicatorH - 10.0f), indicatorW, indicatorH }, 0.15f, 4, uiBorderColor);
        DrawText("[TAB] Mostrar Controles", 20, (int)(screenH - indicatorH + 8.0f * uiScale), fontSizeTiny, SKYBLUE);
    }
}
