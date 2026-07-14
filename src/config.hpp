#pragma once
#include "raylib.h"

// Global
inline const int screenWidth = 1280;
inline const int screenHeight = 720;

// Parámetros del Terreno
inline const int mapSize = 512;
inline const float terrainNoiseScale = 4.0f;
inline const float terrainPersistence = 0.5f;
inline const float terrainLacunarity = 2.0f;


// Umbrales del Mapa (Rango de alturas [-1.0, 1.0])
inline const float heightDeepWater = -0.2f;
inline const float heightShallowWater = 0.0f;
inline const float heightSand = 0.1f;
inline const float heightGrass = 0.45f;
inline const float heightRock = 0.75f;

// Colores
inline const Color colorDeepWater = (Color){20, 50, 150, 255};
inline const Color colorShallowWater = (Color){40, 90, 200, 255};
inline const Color colorSand = (Color){230, 210, 160, 255};
inline const Color colorGrass = (Color){60, 150, 40, 255};
inline const Color colorRock = (Color){100, 90, 80, 255};
inline const Color colorSnow = (Color){250, 250, 255, 255};

// Dimensiones del Mesh
inline const float meshWidth = 120.0f;
inline const float meshLength = 120.0f;
inline const float meshHeight = 30.0f;

// Cámara
inline const float cameraOrbitSpeed = 0.5f;
inline const float cameraRadius = 120.0f;
inline const float cameraHeight = 60.0f;
inline const float cameraFOV = 45.0f;

// UI
inline const Color uiPanelColor = (Color){135, 206, 235, 128};
inline const Color uiBorderColor = BLUE;
inline const int uiIndicatorFontSize = 20;
inline const int uiIndicatorMarginBottom = 40;
inline const int uiIndicatorMarginLeft = 20;

// Parámetros de Erosión Hidráulica
inline const int erosionDroplets = 50000;
inline const int dropletMaxLifetime = 30;
inline const float gravity = 4.0f;
inline const float friction = 0.1f;
inline const float initialWater = 1.0f;
inline const float initialSpeed = 1.0f;
inline const float capacityFactor = 4.0f;
inline const float depositionSpeed = 0.1f;
inline const float erosionSpeed = 0.1f;
inline const float evaporationRate = 0.05f;
inline const float inertia = 0.05f; // Factor de inercia para la dirección de movimiento de la gota


