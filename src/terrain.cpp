#include "terrain.hpp"
#include "benchmark_config.hpp"
#include "raymap_config.hpp"
#include <iostream>
#include <cmath>
#include <omp.h>

using namespace std;

Terrain::Terrain(int size, unsigned int seed, int octaves) 
    : m_heightmap(size, seed, octaves), m_usingColor(true), m_texturesLoaded(false) {
}

Terrain::~Terrain() {
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }
}

Image Terrain::createHeightImage() {
    int size = m_heightmap.getSize();
    const auto& data = m_heightmap.getData();
    Image image = GenImageColor(size, size, BLACK);
    Color* pixels = (Color*)image.data;
    for (int i = 0; i < size * size; i++) {
        unsigned char val = (unsigned char)((data[i] + 1.0f) * 0.5f * 255.0f);
        pixels[i] = (Color){ val, val, val, 255 };
    }
    return image;
}

namespace {
    // Helper local para interpolación lineal de colores
    inline Color lerpColor(Color c1, Color c2, float t) {
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
        return (Color){
            (unsigned char)(c1.r + (c2.r - c1.r) * t),
            (unsigned char)(c1.g + (c2.g - c1.g) * t),
            (unsigned char)(c1.b + (c2.b - c1.b) * t),
            255
        };
    }
}

Image Terrain::createColorImage() {
    int size = m_heightmap.getSize();
    const auto& data = m_heightmap.getData();
    Image image = GenImageColor(size, size, BLACK);
    Color* pixels = (Color*)image.data;

    // Vector de luz solar direccional para el sombreado de colinas
    Vector3 lightDir = { 1.0f, 1.8f, 1.0f };
    float len = sqrt(lightDir.x*lightDir.x + lightDir.y*lightDir.y + lightDir.z*lightDir.z);
    lightDir.x /= len; lightDir.y /= len; lightDir.z /= len;

    // Paralelización del cálculo de textura usando OpenMP
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int i = y * size + x;
            float h = data[i];

            // 1. Calcular normal local por diferencias centrales
            float hL = (x > 0) ? data[y * size + (x - 1)] : h;
            float hR = (x < size - 1) ? data[y * size + (x + 1)] : h;
            float hD = (y > 0) ? data[(y - 1) * size + x] : h;
            float hU = (y < size - 1) ? data[(y + 1) * size + x] : h;

            float scaleX = meshWidth / size;
            float scaleZ = meshLength / size;
            
            float dx = (hR - hL) * meshHeight / (2.0f * scaleX);
            float dz = (hU - hD) * meshHeight / (2.0f * scaleZ);

            Vector3 normal = { -dx, 1.0f, -dz };
            float nLen = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            normal.x /= nLen; normal.y /= nLen; normal.z /= nLen;

            // Pendiente (normal.y es 1.0 en superficies completamente planas)
            float slope = normal.y;

            // 2. Interpolación suave de biomas basada en la altura
            Color baseColor;
            if (h < heightDeepWater) {
                baseColor = colorDeepWater;
            } else if (h < heightShallowWater) {
                float t = (h - heightDeepWater) / (heightShallowWater - heightDeepWater);
                baseColor = lerpColor(colorDeepWater, colorShallowWater, t);
            } else if (h < heightSand) {
                float t = (h - heightShallowWater) / (heightSand - heightShallowWater);
                baseColor = lerpColor(colorShallowWater, colorSand, t);
            } else if (h < heightGrass) {
                float t = (h - heightSand) / (heightGrass - heightSand);
                baseColor = lerpColor(colorSand, colorGrass, t);
            } else if (h < heightRock) {
                float t = (h - heightGrass) / (heightRock - heightGrass);
                baseColor = lerpColor(colorGrass, colorRock, t);
            } else {
                float t = (h - heightRock) / (1.0f - heightRock);
                baseColor = lerpColor(colorRock, colorSnow, t);
            }

            // Aplicar roca desnuda en pendientes inclinadas (acantilados) en tierra firme
            if (h >= heightShallowWater) {
                float steepness = 1.0f - slope; // 0 = plano, 1 = acantilado
                if (steepness > 0.20f) {
                    float t = (steepness - 0.20f) / 0.25f;
                    if (t > 1.0f) t = 1.0f;
                    baseColor = lerpColor(baseColor, colorRock, t);
                }
            }

            // 3. Sombreado de colinas (Hillshading)
            float dotProduct = normal.x*lightDir.x + normal.y*lightDir.y + normal.z*lightDir.z;
            float diffuse = (dotProduct > 0.0f) ? dotProduct : 0.0f;
            float shade = 0.40f + 0.60f * diffuse; // 40% ambiente, 60% difusa

            pixels[i] = (Color){
                (unsigned char)fmin(baseColor.r * shade, 255.0f),
                (unsigned char)fmin(baseColor.g * shade, 255.0f),
                (unsigned char)fmin(baseColor.b * shade, 255.0f),
                255
            };
        }
    }
    return image;
}

Model Terrain::createModel() {
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }

    Image imgHeight = createHeightImage();
    Image imgColor = createColorImage();

    m_texDebug = LoadTextureFromImage(imgHeight);
    m_texColor = LoadTextureFromImage(imgColor);
    m_texturesLoaded = true;

    Mesh mesh = GenMeshHeightmap(imgHeight, (Vector3){ meshWidth, meshHeight, meshLength });
    Model model = LoadModelFromMesh(mesh);

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_usingColor ? m_texColor : m_texDebug;

    UnloadImage(imgHeight);
    UnloadImage(imgColor);
    return model;
}

void Terrain::toggleTexture(Model& model) {
    m_usingColor = !m_usingColor;
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_usingColor ? m_texColor : m_texDebug;
    cout << "Modo de visualización: " << (m_usingColor ? "Realista" : "Debug") << endl;
}
