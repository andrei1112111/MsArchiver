cmake_minimum_required(VERSION 3.27)
project(mlz C)

set(CMAKE_C_STANDARD 23)

add_executable(mlz main.c
        algorithm/LZW.c
        algorithm/LZW.h
        algorithm/file.h
        algorithm/file.c
        algorithm/test.c
        algorithm/test.h)
