#pragma once

// Parámetros de Ruido / fBm del Terreno
inline const float terrainNoiseScale = 2.0f;
inline const float terrainPersistence = 0.5f;
inline const float terrainLacunarity = 2.0f;

// Umbrales del Mapa de Alturas (Rango [-1.0, 1.0])
inline const float heightDeepWater = -0.2f;
inline const float heightShallowWater = 0.0f;
inline const float heightSand = 0.1f;
inline const float heightGrass = 0.45f;
inline const float heightRock = 0.75f;

// Parámetros Físicos de la Erosión Hidráulica
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
