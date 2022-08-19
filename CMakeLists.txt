cmake_minimum_required(VERSION 3.10)
project(reverb LANGUAGES C)

add_library(reverb SHARED
    src/reverb.c
    inc/reverb.h
)

target_include_directories(reverb PRIVATE ./inc/)
