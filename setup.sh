#!/bin/sh
# Configura CMake para el proyecto raymap (admite flags adicionales del usuario como -DBUILD_VISUALIZER=OFF)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "$@"
