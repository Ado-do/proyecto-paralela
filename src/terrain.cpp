#include "terrain.hpp"
#include "noise.hpp"
#include "config.hpp"
#include <chrono>
#include <omp.h>
#include <iostream>

using namespace std;

Terrain::Terrain(int size, unsigned int seed) 
    : m_size(size), m_seed(seed), m_data(size * size), m_textureLoaded(false) {
    m_pTable = noise::generatePermutationTable(m_seed);
}

Terrain::~Terrain() {
    if (m_textureLoaded) {
        UnloadTexture(m_texture);
    }
}

void Terrain::generateSequential() {
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                terrainOctaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

void Terrain::generateParallel() {
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                terrainOctaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

BenchmarkResults Terrain::runBenchmark() {
    cout << "Starting benchmark (Size: " << m_size << "x" << m_size << ")..." << endl;

    auto startSeq = chrono::high_resolution_clock::now();
    generateSequential();
    auto endSeq = chrono::high_resolution_clock::now();
    double tSeq = chrono::duration<double, milli>(endSeq - startSeq).count();

    auto startPar = chrono::high_resolution_clock::now();
    generateParallel();
    auto endPar = chrono::high_resolution_clock::now();
    double tPar = chrono::duration<double, milli>(endPar - startPar).count();

    return { tSeq, tPar, tSeq / tPar };
}

Image Terrain::createHeightImage() {
    Image image = GenImageColor(m_size, m_size, BLACK);
    Color* pixels = (Color*)image.data;
    for (int i = 0; i < m_size * m_size; i++) {
        unsigned char val = (unsigned char)((m_data[i] + 1.0f) * 0.5f * 255.0f);
        pixels[i] = (Color){ val, val, val, 255 };
    }
    return image;
}

Model Terrain::createModel() {
    Image img = createHeightImage();
    m_texture = LoadTextureFromImage(img);
    m_textureLoaded = true;

    Mesh mesh = GenMeshHeightmap(img, (Vector3){ meshWidth, meshHeight, meshLength });
    Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_texture;

    UnloadImage(img);
    return model;
}
