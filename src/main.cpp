#include "raylib-cpp.hpp"

#include <iostream>
#include <omp.h>

int main() {
    std::cout << "OMP CPU Threads: " << omp_get_max_threads() << "\n";

    const int screenWidth = 800;
    const int screenHeight = 450;

    raylib::Window window(screenWidth, screenHeight, "raymap");
    window.SetTargetFPS(60);

    while (!window.ShouldClose()) {
        BeginDrawing();

        window.ClearBackground(RAYWHITE);
        DrawText("HOAAA!", 170, 200, 20, DARKGRAY);

        EndDrawing();
    }

    return 0;
}
