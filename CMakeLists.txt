cmake_minimum_required(VERSION 2.6)
PROJECT(rift)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# Packages requis
FIND_PACKAGE(OpenGL REQUIRED)
include(FindGLEW)
include(FindGLFW)
include(FindYAMLCPP)
include(FindAnttweakBar)
include(cotire)
find_package(assimp REQUIRED)

set(RIFT_HOME ${CMAKE_SOURCE_DIR})
set(RIFT_EXECUTABLE rift)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RIFT_HOME}/bin")

# librift
add_subdirectory(src/rift)
# main
add_subdirectory(src/main)
# model_converter
add_subdirectory(src/model_converter)

# Définitions du préprocesseur
ADD_DEFINITIONS(-DGLM_FORCE_RADIANS)

IF(WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
ENDIF()

#
# includes pour nos headers
#
INCLUDE_DIRECTORIES(
    include/
    include/renderer
    include/log
    include/renderer/gl3
    include/renderer/dummy
    include/utils
    deps/include
    src/
)

#
# Fichiers sources
#
file(
    GLOB
    RIFT_SOURCES
    src/*.cpp
    src/log/*.cpp
    src/renderer/*.cpp
    src/renderer/gl3/*.cpp
    src/renderer/dummy/*.cpp
    src/utils/*.cpp
)

file(
    GLOB
    RIFT_HEADERS
    include/*.hpp
    include/log/*.hpp
    include/renderer/*.hpp
    include/renderer/gl3/*.hpp
    include/renderer/dummy/*.hpp
    include/utils/*.hpp
)

#
# Includes pour les headers systèmes
#
INCLUDE_DIRECTORIES(
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${GLM_INCLUDE_PATH}
    ${YAMLCPP_INCLUDE_DIR}
    ${ANTTWEAKBAR_INCLUDE_DIR}
)

LINK_DIRECTORIES(
    ${GLFW_LIBRARY_DIR}
)

#
# Executable
ADD_EXECUTABLE(
    ${RIFT_EXECUTABLE}
    ${RIFT_SOURCES}
    ${RIFT_HEADERS}
)

set_target_properties(${RIFT_EXECUTABLE} PROPERTIES
    COTIRE_PREFIX_HEADER_INCLUDE_PATH  "${GLEW_INCLUDE_PATH};${GLFW_INCLUDE_DIR};${GLM_INCLUDE_PATH};${YAMLCPP_INCLUDE_DIR};${ANTTWEAKBAR_INCLUDE_DIR}")

cotire(${RIFT_EXECUTABLE})

# 'make run'
ADD_CUSTOM_TARGET(run
    COMMAND ${RIFT_EXECUTABLE}
    DEPENDS ${RIFT_EXECUTABLE}
    WORKING_DIRECTORY ${RIFT_HOME}
)

# MacOS (Jocelyn)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    TARGET_LINK_LIBRARIES(
        ${RIFT_EXECUTABLE}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        )
# Windows
ELSEIF(WIN32)
TARGET_LINK_LIBRARIES(
    ${RIFT_EXECUTABLE}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY}
    ${GLEW_LIBRARY}
    ${ANTTWEAKBAR_LIBRARY}
)

target_link_libraries(
    ${RIFT_EXECUTABLE} 
    debug 
    ${YAMLCPP_LIBRARY_DEBUG}  
    ${ASSIMP_LIBRARY_RELEASE} 
    optimized 
    ${YAMLCPP_LIBRARY_RELEASE} 
    ${ASSIMP_LIBRARY_RELEASE} )

# Linux (PC imag)
ELSE()
    TARGET_LINK_LIBRARIES(
        ${RIFT_EXECUTABLE}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        ${YAMLCPP_LIBRARY}
        -lX11
        -lXxf86vm
        -lXrandr
        -lpthread
        -lXi
        -lXcursor
        -lassimp
        )
ENDIF()
