cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(main)
# project(RayTracingDemo)

add_executable(${PROJECT_NAME}  
./src/main.cpp
# ./src/material.hpp
# ./src/OBJ_Loader.hpp
# ./src/utils.hpp
)

# aux_source_directory(./src main_srcs)

# add_executable(RayTracingDemo 
# src/main.cpp
# # src/Utils.hpp
# src/material.hpp
# src/utils.hpp
# src/OBJ_Loader.hpp)