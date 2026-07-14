#!/bin/sh
./build.sh && cd build && ctest --output-on-failure
