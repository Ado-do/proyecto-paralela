#pragma once
#include "raylib.h"

// Global
inline const int screenWidth = 1280;
inline const int screenHeight = 720;

// Colores
inline const Color colorDeepWater = (Color){20, 50, 150, 255};
inline const Color colorShallowWater = (Color){40, 90, 200, 255};
inline const Color colorSand = (Color){230, 210, 160, 255};
inline const Color colorGrass = (Color){60, 150, 40, 255};
inline const Color colorRock = (Color){100, 90, 80, 255};
inline const Color colorSnow = (Color){250, 250, 255, 255};

// Dimensiones del Mesh
inline const float meshWidth = 180.0f;
inline const float meshLength = meshWidth;
inline const float meshHeight = 60.0f;

// Cámara
inline const float cameraOrbitSpeed = 0.5f;
inline const float cameraRadius = 120.0f;
inline const float cameraHeight = 90.0f;
inline const float cameraFOV = 60.0f;

// UI
inline const Color uiPanelColor = (Color){135, 206, 235, 128};
inline const Color uiBorderColor = BLUE;
inline const int uiIndicatorFontSize = 20;
inline const int uiIndicatorMarginBottom = 40;
inline const int uiIndicatorMarginLeft = 20;
