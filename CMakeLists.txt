cmake_minimum_required(VERSION 3.21)
project(RG_project)

include_directories(include/)


find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(external/glad)
add_subdirectory(external/imgui)
set(CMAKE_CXX_STANDARD 17)

add_executable(RG_project main.cpp include/classes_impl/Shader.h include/stb_image.cpp include/classes_impl/Camera.h include/classes_impl/Model.h include/classes_impl/Mesh.h include/classes_impl/model.h)

target_link_libraries(RG_project glfw glad OpenGL::GL pthread Xrandr X11 Xi dl assimp imgui)
