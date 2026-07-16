#include "visuals.hpp"
#include "raymap_config.hpp"
#include <cmath>

OrbitCamera::OrbitCamera(float radius, float height, float fov) 
    : m_radius(radius), m_height(height), m_angle(0.0f) {
    m_camera = { 0 };
    m_camera.position = (Vector3){ radius, height, radius };
    m_camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    m_camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    m_camera.fovy = fov;
    m_camera.projection = CAMERA_PERSPECTIVE;
}

void OrbitCamera::update(float dt) {
    m_angle += cameraOrbitSpeed * dt;
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

void DrawMetricsUI(const BenchmarkResults& results) {
    // Fondo del panel
    DrawRectangle(10, 10, 280, 380, uiPanelColor);
    DrawRectangleLines(10, 10, 280, 380, uiBorderColor);

    int y = 20;
    DrawText("Métricas de Rendimiento", 20, y, 15, DARKBLUE); y += 25;
    DrawText(TextFormat("Hilos:        %d", results.numThreads), 20, y, 18, DARKPURPLE); y += 20;
    DrawText(TextFormat("Secuencial:   %.2f ms", results.timeSequential), 20, y, 18, BLACK); y += 20;
    DrawText(TextFormat("Paralelo:     %.2f ms", results.timeParallel), 20, y, 18, BLACK); y += 20;
    DrawText(TextFormat("Speedup:      %.2fx", results.speedup), 20, y, 20, DARKGREEN); y += 22;
    DrawText(TextFormat("Eficiencia:   %.1f%%", results.efficiency), 20, y, 18, DARKGREEN); y += 20;
    DrawText(TextFormat("Costo (p*Tp):  %.2f ms", results.cost), 20, y, 18, MAROON); y += 30;

    DrawText("Métricas de Erosión", 20, y, 15, DARKBLUE); y += 25;
    const char* modeStr = (results.erosionMode == ErosionMode::SEQUENTIAL) ? "Secuencial" :
                          (results.erosionMode == ErosionMode::PARALLEL_ATOMIC) ? "Paralelo (Atómico)" :
                          "Paralelo (Buffers Locales)";
    DrawText(TextFormat("Modo:   %s", modeStr), 20, y, 15, DARKPURPLE); y += 20;
    DrawText(TextFormat("Tiempo: %.2f ms", results.timeErosion), 20, y, 16, MAROON); y += 30;

    DrawText("Parámetros del Terreno", 20, y, 15, DARKBLUE); y += 25;
    DrawText(TextFormat("Semilla:      %u", results.seed), 20, y, 16, DARKGRAY); y += 18;
    DrawText(TextFormat("Escala:       %.2f", results.scale), 20, y, 16, DARKGRAY); y += 18;
    DrawText(TextFormat("Octavas:      %d", results.octaves), 20, y, 16, DARKGRAY); y += 18;
    DrawText(TextFormat("Persistencia: %.2f", results.persistence), 20, y, 16, DARKGRAY); y += 18;
    DrawText(TextFormat("Lacunaridad:  %.2f", results.lacunarity), 20, y, 16, DARKGRAY);
}
