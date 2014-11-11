cmake_minimum_required(VERSION 2.6)
project(modelimporter)
include(FindYAMLCPP)
include(cotire)
set(MODELIMPORTER_EXECUTABLE modelimporter)
set(BOOST_DEBUG ON)
set(BOOST_ROOT "C:\\local\\boost_1_55_0")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}\\lib64-msvc-12.0")
set(Boost_USE_STATIC_LIBS ON)
# 'Boost' package name is case-sensitive? what. the. actual. fuck.
find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
find_package(assimp REQUIRED)
add_definitions(-DGLM_FORCE_RADIANS)
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RIFT_HOME}/bin")
set(MODELIMPORTER_HOME ${RIFT_HOME}/tools/modelimporter)

include_directories(
    ${YAMLCPP_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${MODELIMPORTER_HOME}
    ${ASSIMP_INCLUDE_DIR}
    ${RIFT_HOME}/include
    ${RIFT_HOME}/include/log
    ${RIFT_HOME}/include/renderer/gl3
)

file(
    GLOB
    MODELIMPORTER_SOURCES
    ${MODELIMPORTER_HOME}/*.cpp
    ${RIFT_HOME}/src/log/*.cpp
)

file(
    GLOB
    MODELIMPORTER_HEADERS
    ${MODELIMPORTER_HOME}/*.hpp
    ${RIFT_HOME}/include/log/*.hpp
)

add_executable(
    ${MODELIMPORTER_EXECUTABLE}
    ${MODELIMPORTER_SOURCES}
    ${MODELIMPORTER_HEADERS}
)

link_directories(${BOOST_LIBRARYDIR})
set_target_properties(${MODELIMPORTER_EXECUTABLE} PROPERTIES
    COTIRE_PREFIX_HEADER_INCLUDE_PATH  "${GLM_INCLUDE_PATH};${YAMLCPP_INCLUDE_DIR};${Boost_INCLUDE_DIRS};${ASSIMP_INCLUDE_DIR}")
target_link_libraries(${MODELIMPORTER_EXECUTABLE} 
    debug ${YAMLCPP_LIBRARY_DEBUG} 
    debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG} 
    debug ${Boost_FILESYSTEM_LIBRARY_DEBUG} 
    debug ${Boost_SYSTEM_LIBRARY_DEBUG} 
    debug ${ASSIMP_LIBRARY_RELEASE} 
    optimized ${YAMLCPP_LIBRARY_RELEASE} 
    optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE} 
    optimized ${Boost_FILESYSTEM_LIBRARY_RELEASE} 
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE} 
    optimized ${ASSIMP_LIBRARY_RELEASE} )

cotire(${MODELIMPORTER_EXECUTABLE})
