cmake_minimum_required(VERSION 3.7)

project(3d-demo VERSION 1.0) 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/") 

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
# find_package(assimp REQUIRED HINTS ${ASSIMP_DIR}) 

add_definitions(
  -DGLEW_STATIC
  -DUSE_INTRINSICS_SSE
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

file(GLOB DEMO_H demo/*.h)
file(GLOB DEMO_C demo/*.cpp)

file(GLOB ENGINE2_I engine2/*.h)
file(GLOB ENGINE2_S engine2/*.cpp)
file(GLOB GFX_OPENGL_H engine2/OpenGL/*.h)
file(GLOB GFX_OPENGL_C engine2/OpenGL/*.cpp)

set(ENGINE2_FILES
  ${ENGINE2_I}
  ${ENGINE2_S}
  ${GFX_OPENGL_H}
  ${GFX_OPENGL_C}
)


set(DEMO_FILES
  ${DEMO_H}
  ${DEMO_C}
)

add_library(engine2 STATIC
  ${ENGINE2_FILES}
)

add_library(tinyobjloader STATIC
  external/tinyobjloader/tiny_obj_loader.h
  external/tinyobjloader/tiny_obj_loader.cc
)

add_library(nv_dds STATIC
  external/nv_dds/nv_dds.h
  external/nv_dds/nv_dds.cpp
)

add_library(glew STATIC
  external/glew-2.1.0/src/glew.c
  external/glew-2.1.0/include/GL/glew.h
)

add_library(stb_image STATIC
  external/stb/stb_image.h
  external/stb/stb_image.c
  external/stb/stb_image_resize.h
  external/stb/stb_image_resize.c
)

add_library(tinygltf STATIC
  external/tinygltf/tiny_gltf.h
  external/tinygltf/tiny_gltf.cpp
)

file(GLOB IMGGUI_H external/imgui/*.h)
file(GLOB IMGGUI_C external/imgui/*.cpp)
file(GLOB IMGGUI_BACKENDS_H external/imgui/backends/*.h)
file(GLOB IMGGUI_BACKENDS_C external/imgui/backends/*.cpp)

set(IMGUI_FILES 
${IMGGUI_H}
${IMGGUI_C}
${IMGGUI_BACKENDS_H}
${IMGGUI_BACKENDS_C}
)

add_library(imgui STATIC
${IMGUI_FILES}
)

include_directories(
  ./
  demo/
  engine2/include/
  external/glew-2.1.0/include/
  external/glm/
  external/stb/
  external/half/
  external/tinygltf/
  external/nv_dds/
  external/tinyobjloader/
  external/imgui/
  external/imgui/backends
  lib/ktx/include/
  ${SDL2_INCLUDE_DIR}
)

link_directories(
#  ${ASSIMP_LIBRARY_DIRS}
)

file(GLOB SHADERS_G assets/shaders/*.glsl)
file(GLOB SHADERS_I assets/shaders/*.inc)
file(GLOB SHADERS_B assets/shaders/builtin/*.glsl)
file(GLOB SHADERS_BI assets/shaders/builtin/*.inc)

set(SHADERS_BUILTIN
${SHADERS_B}
${SHADERS_BI}
)

set(SHADERS_DEV
${SHADERS_G}
${SHADERS_I}
)

set(SHADERS
${SHADERS_G}
${SHADERS_I}
${SHADERS_B}
${SHADERS_BI}
)

add_executable(demo
  ${DEMO_FILES}
  ${SHADERS}
)

target_link_libraries(demo
  glew
  stb_image
  tinygltf
  engine2
  nv_dds
  tinyobjloader
  imgui
  ../lib/ktx/ktx
  ${OPENGL_LIBRARY}
  ${SDL2_LIBRARIES}
)

if(WIN32)

  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
  else()
    set(ARCH "x86")
  endif()

#  add_custom_command(TARGET demo POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#     "${assimp_DIR}/../../../bin/assimp${ASSIMP_LIBRARY_SUFFIX}.dll"
#     $<TARGET_FILE_DIR:demo>)

  add_custom_command(TARGET demo POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy
     "${SDL2_INCLUDE_DIR}/../lib/${ARCH}/SDL2.dll"
     $<TARGET_FILE_DIR:demo>)
     add_custom_command(TARGET demo POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy
     "../lib/ktx/ktx.dll"
     $<TARGET_FILE_DIR:demo>)
endif()

set(GFX_OPENGL
  ${GFX_OPENGL_H}
  ${GFX_OPENGL_C}
)

source_group("renderer/OpenGL" FILES ${GFX_OPENGL} )
source_group("Shaders" FILES ${SHADERS_DEV} )
source_group("Shaders/builtin" FILES ${SHADERS_BUILTIN} )
