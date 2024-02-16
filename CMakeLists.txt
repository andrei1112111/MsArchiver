cmake_minimum_required(VERSION 3.27)
project(mlza C)

set(CMAKE_C_STANDARD 23)

add_executable(mlza main.c
        algorithm/LZW.c
        algorithm/LZW.h)
