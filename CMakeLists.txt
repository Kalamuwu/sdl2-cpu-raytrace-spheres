cmake_minimum_required(VERSION 3.0)

project(sdl2-cpu-raytrace-spheres)

add_executable(sdl2-cpu-raytrace-spheres main.cpp)
target_link_libraries(sdl2-cpu-raytrace-spheres SDL2 SDL2_image -pthread)

install(TARGETS sdl2-cpu-raytrace-spheres RUNTIME DESTINATION bin)
