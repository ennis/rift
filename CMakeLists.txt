cmake_minimum_required(VERSION 2.6)

PROJECT(rift)

#
# Packages requis
FIND_PACKAGE(OpenGL REQUIRED)
include("cmake/FindGLEW.cmake")
include("cmake/FindGLFW.cmake")
set(GLM_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/local/include/")

set(RIFT_HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(RIFT_EXECUTABLE rift)

#
# Définitions du préprocesseur
#
ADD_DEFINITIONS(-DGLM_FORCE_RADIANS)

IF(WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

#
# includes pour nos headers
#
INCLUDE_DIRECTORIES(
	include/
	src/
)

#
# Fichiers sources
#
file(
    GLOB
    RIFT_SOURCES
    src/*.cpp
    src/renderer/*.cpp
    src/renderer/gl3/*.cpp
)

file(
    GLOB
    RIFT_HEADERS
    include/*.hpp
    include/renderer/*.hpp
    include/renderer/gl3/*.hpp
)

#
# Includes pour les headers systèmes
#
INCLUDE_DIRECTORIES(
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${GLM_INCLUDE_PATH}
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
        )
# Linux (PC imag)
ELSE()
    TARGET_LINK_LIBRARIES(
        ${RIFT_EXECUTABLE}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        -lX11
        -lXxf86vm
        -lXrandr
        -lpthread
        -lXi
        -lXcursor
        -lassimp
        )
ENDIF()
