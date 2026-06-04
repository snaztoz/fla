@echo off

cmake -G Ninja -B build/ -S . && cmake --build build/ -j
