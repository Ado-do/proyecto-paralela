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

void DrawInterface(const BenchmarkResults& results, bool usingColor, bool drawWireframe, bool autoRotate) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // 1. PANEL IZQUIERDO: Métricas de Rendimiento y Erosión
    DrawRectangleRounded((Rectangle){ 10, 10, 290, 310 }, 0.05f, 4, uiPanelColor);
    DrawRectangleRoundedLines((Rectangle){ 10, 10, 290, 310 }, 0.05f, 4, uiBorderColor);

    int y = 25;
    DrawText("Métricas de Rendimiento", 25, y, 16, SKYBLUE); y += 30;
    DrawText(TextFormat("Hilos Activos: %d", results.numThreads), 25, y, 14, WHITE); y += 20;
    DrawText(TextFormat("Secuencial:   %.2f ms", results.timeSequential), 25, y, 14, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Paralelo:     %.2f ms", results.timeParallel), 25, y, 14, LIGHTGRAY); y += 22;
    
    DrawText("Aceleración (Speedup):", 25, y, 14, LIME);
    DrawText(TextFormat("%.2fx", results.speedup), 195, y, 15, LIME); y += 20;
    DrawText("Eficiencia:", 25, y, 14, LIME);
    DrawText(TextFormat("%.1f%%", results.efficiency), 195, y, 14, LIME); y += 20;
    DrawText(TextFormat("Costo (p * Tp): %.2f ms", results.cost), 25, y, 13, ORANGE); y += 35;

    DrawText("Métricas de Erosión", 25, y, 16, SKYBLUE); y += 30;
    const char* modeStr = (results.erosionMode == ErosionMode::SEQUENTIAL) ? "Secuencial" :
                          (results.erosionMode == ErosionMode::PARALLEL_ATOMIC) ? "Paralelo (Atómico)" :
                          "Paralelo (Locales)";
    DrawText(TextFormat("Modo:   %s", modeStr), 25, y, 13, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Tiempo: %.2f ms", results.timeErosion), 25, y, 14, (results.timeErosion > 0) ? GOLD : LIGHTGRAY);

    // 2. PANEL DERECHO: Parámetros del Terreno y Visualización
    int rightPanelX = screenW - 300;
    DrawRectangleRounded((Rectangle){ (float)rightPanelX, 10, 290, 310 }, 0.05f, 4, uiPanelColor);
    DrawRectangleRoundedLines((Rectangle){ (float)rightPanelX, 10, 290, 310 }, 0.05f, 4, uiBorderColor);

    y = 25;
    DrawText("Parámetros del Terreno", rightPanelX + 25, y, 16, SKYBLUE); y += 30;
    DrawText(TextFormat("Semilla:      %u", results.seed), rightPanelX + 25, y, 14, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Escala:       %.2f", results.scale), rightPanelX + 25, y, 14, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Octavas:      %d", results.octaves), rightPanelX + 25, y, 14, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Persistencia: %.2f", results.persistence), rightPanelX + 25, y, 14, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Lacunaridad:  %.2f", results.lacunarity), rightPanelX + 25, y, 14, LIGHTGRAY); y += 35;

    DrawText("Estado Visual", rightPanelX + 25, y, 16, SKYBLUE); y += 30;
    DrawText(TextFormat("Textura: %s", usingColor ? "Realista (Color)" : "Depuración (Grises)"), rightPanelX + 25, y, 13, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Malla:   %s", drawWireframe ? "Sólido + Alambre" : "Sólido"), rightPanelX + 25, y, 13, LIGHTGRAY); y += 20;
    DrawText(TextFormat("Cámara:  %s", autoRotate ? "Rotación Activa" : "Cámara Manual"), rightPanelX + 25, y, 13, LIGHTGRAY);

    // 3. PANEL INFERIOR: Controles agrupados
    DrawRectangleRounded((Rectangle){ 10, (float)(screenH - 110), (float)(screenW - 20), 100 }, 0.15f, 4, uiPanelColor);
    DrawRectangleRoundedLines((Rectangle){ 10, (float)(screenH - 110), (float)(screenW - 20), 100 }, 0.15f, 4, uiBorderColor);

    int colY = screenH - 100;
    int colWidth = (screenW - 40) / 4;

    // Columna 1: Terreno
    int colX = 25;
    DrawText("[TERRENO]", colX, colY, 13, SKYBLUE);
    DrawText("ESPACIO: Semilla aleatoria", colX, colY + 20, 12, LIGHTGRAY);
    DrawText("H / L: Semilla (- / +)", colX, colY + 38, 12, LIGHTGRAY);
    DrawText("J / K: Cant. Octavas", colX, colY + 56, 12, LIGHTGRAY);

    // Columna 2: Erosión
    colX += colWidth;
    DrawText("[EROSIÓN]", colX, colY, 13, SKYBLUE);
    DrawText("E: Aplicar Erosión", colX, colY + 20, 12, LIGHTGRAY);
    DrawText("V: Cambiar modo de erosión", colX, colY + 38, 12, LIGHTGRAY);

    // Columna 3: Visualización
    colX += colWidth;
    DrawText("[VISUALIZACIÓN]", colX, colY, 13, SKYBLUE);
    DrawText("C: Alternar textura (Color/Grises)", colX, colY + 20, 12, LIGHTGRAY);
    DrawText("Z: Alternar malla de alambre", colX, colY + 38, 12, LIGHTGRAY);
    DrawText("P: Pausar/Reanudar cámara", colX, colY + 56, 12, LIGHTGRAY);

    // Columna 4: Navegación e Hilos
    colX += colWidth;
    DrawText("[NAVEGACIÓN E HILOS]", colX, colY, 13, SKYBLUE);
    DrawText("Clic-Der + Arrastrar: Orbitar", colX, colY + 20, 12, LIGHTGRAY);
    DrawText("Rueda de Mouse: Zoom cámara", colX, colY + 38, 12, LIGHTGRAY);
    DrawText("X: Alternar cantidad de hilos", colX, colY + 56, 12, LIGHTGRAY);
}
