cmake_minimum_required(VERSION 3.7)

project(3d-demo VERSION 1.0) 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/") 

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
# find_package(assimp REQUIRED HINTS ${ASSIMP_DIR}) 

add_subdirectory(external/SOIL2)

add_definitions(
  -DGLEW_STATIC
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

file(GLOB DEMO_GL_H demo/opengl/*.h)
file(GLOB DEMO_GL_C demo/opengl/*.cpp)
file(GLOB DEMO_H demo/*.h)
file(GLOB DEMO_C demo/*.cpp)

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
  demo/opengl/
  demo/
  external/glew-2.1.0/include/
  external/glm/
  external/stb/
  external/SOIL2/src/SOIL2/
  external/tinygltf/
  ${SDL2_INCLUDE_DIR}
)

link_directories(
#  ${ASSIMP_LIBRARY_DIRS}
)

file(GLOB SHADERS assets/shaders/*.glsl)
file(GLOB SHADERS_INC assets/shaders/*.inc)

add_executable(demo	
  ${DEMO_H}
  ${DEMO_C}
  ${DEMO_GL_H}
  ${DEMO_GL_C}
  ${SHADERS}
)

target_link_libraries(demo
  glew
  stb_image
  tinygltf
  soil2
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

set(DEMO_GL
  ${DEMO_GL_H}
  ${DEMO_GL_C}
)
source_group("Shaders" FILES ${SHADERS} )
source_group("OpenGL" FILES ${DEMO_GL} )
