cmake_minimum_required(VERSION 3.7)

project(3d-demo VERSION 1.0) 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/") 

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
# find_package(assimp REQUIRED HINTS ${ASSIMP_DIR}) 

add_definitions(
  -DGLEW_STATIC
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

file(GLOB DEMO_H demo/*.h)
file(GLOB DEMO_C demo/*.cpp)
file(GLOB DEMO_OGL_H demo/opengl/*.h)
file(GLOB DEMO_OGL_C demo/opengl/*.cpp)

file(GLOB ENGINE_GFX_H engine/gfx/*.h)
file(GLOB ENGINE_GFX_C engine/gfx/*.cpp)
file(GLOB ENGINE_INPUT_H engine/input/*.h)
file(GLOB ENGINE_INPUT_C engine/input/*.cpp)
file(GLOB ENGINE_RSRC_H engine/resource/*.h)
file(GLOB ENGINE_RSRC_C engine/resource/*.cpp)
file(GLOB ENGINE_SCENE_H engine/scene/*.h)
file(GLOB ENGINE_SCENE_C engine/scene/*.cpp)
file(GLOB ENGINE_MATH_H engine/math/*.h)
file(GLOB ENGINE_MATH_C engine/math/*.cpp)

set(ENGINE_GFX
  ${ENGINE_GFX_H}
  ${ENGINE_GFX_C}
)

set(ENGINE_INPUT
  ${ENGINE_INPUT_H}
  ${ENGINE_INPUT_C}
)

set(ENGINE_RSRC
  ${ENGINE_RSRC_H}
  ${ENGINE_RSRC_C}
)

set(ENGINE_SCENE
  ${ENGINE_SCENE_H}
  ${ENGINE_SCENE_C}
)

set(ENGINE_MATH
  ${ENGINE_MATH_H}
  ${ENGINE_MATH_C}
)

set(ENGINE_FILES
  ${ENGINE_GFX_H}
  ${ENGINE_GFX_C}
  ${ENGINE_INPUT_H}
  ${ENGINE_INPUT_C}
  ${ENGINE_RSRC_H}
  ${ENGINE_RSRC_C}
  ${ENGINE_SCENE_H}
  ${ENGINE_SCENE_C}
  ${ENGINE_MATH_H}
  ${ENGINE_MATH_C}
  engine/engine.h
  engine/common.h
)

set(DEMO_FILES
  ${DEMO_H}
  ${DEMO_C}
  ${DEMO_OGL_H}
  ${DEMO_OGL_C}
)

add_library(engine STATIC
  ${ENGINE_FILES}
)

add_library(glew STATIC
  external/glew-2.1.0/src/glew.c
  external/glew-2.1.0/include/GL/glew.h
)

add_library(stb_image STATIC
  external/stb/stb_image.h
  external/stb/stb_image.c
)

add_library(tinygltf STATIC
  external/tinygltf/tiny_gltf.h
  external/tinygltf/tiny_gltf.cpp
)


include_directories(
  ./
  demo/
  demo/opengl/
  engine/
  external/glew-2.1.0/include/
  external/glm/
  external/stb/
  external/half/
  external/tinygltf/
  ${SDL2_INCLUDE_DIR}
)

link_directories(
#  ${ASSIMP_LIBRARY_DIRS}
)

file(GLOB SHADERS assets/shaders/*.glsl)
file(GLOB SHADERS_INC assets/shaders/*.inc)

add_executable(demo
  ${DEMO_FILES}
  ${SHADERS}
)

target_link_libraries(demo
  glew
  stb_image
  tinygltf
  engine
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
endif()

source_group("Shaders" FILES ${SHADERS} )
source_group("gfx" FILES ${ENGINE_GFX} )
source_group("input" FILES ${ENGINE_INPUT} )
source_group("scene" FILES ${ENGINE_SCENE} )
source_group("resource" FILES ${ENGINE_RSRC} )
source_group("math" FILES ${ENGINE_MATH} )
