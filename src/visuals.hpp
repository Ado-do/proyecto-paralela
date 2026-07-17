#pragma once
#include "raylib.h"
#include "terrain.hpp"

class OrbitCamera {
public:
    OrbitCamera(float radius, float height, float fov);
    void update(float dt);
    void beginMode();
    void endMode();
    void toggleAutoRotate() { m_autoRotate = !m_autoRotate; }
    bool isAutoRotate() const { return m_autoRotate; }

private:
    Camera3D m_camera;
    float m_angle;
    float m_radius;
    float m_height;
    bool m_autoRotate;
};

void DrawInterface(const BenchmarkResults& results, bool usingColor, bool drawWireframe, bool autoRotate, bool showControls = true, bool is2DMode = false, float uiScale = 1.2f);
