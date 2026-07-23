cmake_minimum_required(VERSION 4.1)
project(HighPerf_RayTracer)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(HighPerf_RayTracer
        src/main.cpp
        src/main.h
        src/hittable.h
        src/sphere.h
        src/hittable_list.h
        src/material.h
        src/lambertian.h
        src/metal.h
        src/dielectric.h
        src/aabb.h
        src/bvh.h
        src/plane.h)
