#pragma once
#include "raylib.h"

// Configuración Global
const int screenWidth = 1280;
const int screenHeight = 720;

// Parámetros del Terreno
const int mapSize = 512;
const unsigned int terrainSeed = 12345;
const float terrainNoiseScale = 4.0f;
const int terrainOctaves = 6;
const float terrainPersistence = 0.5f;
const float terrainLacunarity = 2.0f;

// Dimensiones del Mesh
const float meshWidth = 120.0f;
const float meshHeight = 30.0f;
const float meshLength = 120.0f;

// Configuración de Cámara
const float cameraOrbitSpeed = 0.5f;
const float cameraRadius = 120.0f;
const float cameraHeight = 60.0f;
const float cameraFOV = 45.0f;

// Colores y UI
const Color uiPanelColor = (Color){ 135, 206, 235, 128 };
const Color uiBorderColor = BLUE;

