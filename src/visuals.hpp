#pragma once
#include "raylib.h"
#include "terrain.hpp"

class OrbitCamera {
public:
    OrbitCamera(float radius, float height, float fov);
    void update(float dt);
    void beginMode();
    void endMode();

private:
    Camera3D m_camera;
    float m_angle;
    float m_radius;
    float m_height;
};

void DrawMetricsUI(const BenchmarkResults& results);
