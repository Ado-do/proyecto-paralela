#include "visuals.hpp"
#include "config.hpp"
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
    DrawRectangle(10, 10, 280, 110, uiPanelColor);
    DrawRectangleLines(10, 10, 280, 110, uiBorderColor);

    DrawText("Terrain Generation Metrics", 20, 20, 15, DARKBLUE);
    DrawText(TextFormat("Sequential: %.2f ms", results.timeSequential), 20, 45, 18, BLACK);
    DrawText(TextFormat("Parallel:   %.2f ms", results.timeParallel), 20, 65, 18, BLACK);
    DrawText(TextFormat("Speedup:    %.2fx", results.speedup), 20, 85, 20, DARKGREEN);
}
