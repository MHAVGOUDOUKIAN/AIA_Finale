cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME App)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "-no-pie")
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/../src)

include_directories(${PROJECT_SOURCE_DIR}/../src)

set( SOURCES
    ${SOURCE_DIR}/glad.c
    ${SOURCE_DIR}/main.cpp 
    ${SOURCE_DIR}/OpenGL_App.cpp
    ${SOURCE_DIR}/OpenGL_Camera.cpp
    ${SOURCE_DIR}/OpenGL_Shader.cpp
    ${SOURCE_DIR}/OpenGL_Texture.cpp
    ${SOURCE_DIR}/OpenCV_Main.cpp
)

# //   headers file //
set( HEADERS
    ${SOURCE_DIR}/OpenGL_App.hpp
    ${SOURCE_DIR}/OpenGL_Camera.hpp
    ${SOURCE_DIR}/OpenGL_Shader.hpp
    ${SOURCE_DIR}/OpenGL_Texture.hpp
    ${SOURCE_DIR}/stb_image.h
    ${SOURCE_DIR}/glad.h
    ${SOURCE_DIR}/OpenCV_Main.hpp
)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(${PROJECT_NAME} # Name of the executable
    ${SOURCES}
    ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    glfw
    dl
)
