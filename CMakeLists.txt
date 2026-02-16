cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(YetMoreEditorTools VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})

CPMAddPackage("gh:eliaskosunen/scnlib#4b8a740")
CPMAddPackage("gh:lumia431/reaction#721f7f3")
CPMAddPackage("gh:ZXShady/enchantum#ea53e47")

messagE(STATUS reaction source ${reaction_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${reaction_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} reaction::reaction scn::scn enchantum::enchantum)

#enable colors on windows
#https://stackoverflow.com/a/77419222
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics)
    target_link_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics -Wno-nrvo)
endif()