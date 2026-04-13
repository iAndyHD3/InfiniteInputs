cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

#enable this to reduce build size a lot
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

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
CPMAddPackage("gh:ZXShady/enchantum#2f7fff7")


CPMAddPackage("gh:boostorg/predef#a2a5010")
CPMAddPackage("gh:boostorg/mp11#48019a0")
CPMAddPackage("gh:boostorg/config#423423e")
CPMAddPackage("gh:boostorg/assert#af15ced")
CPMAddPackage("gh:boostorg/throw_exception#0924b53")
CPMAddPackage("gh:boostorg/core#a90a319")
CPMAddPackage("gh:boostorg/describe#4959611")
CPMAddPackage("gh:boostorg/container_hash#e3cbbeb")
CPMAddPackage("gh:boostorg/unordered#6699184")


target_include_directories(${PROJECT_NAME} PRIVATE ${reaction_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} reaction::reaction scn::scn enchantum::enchantum Boost::unordered)

#enable colors on windows
#https://stackoverflow.com/a/77419222
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics)
    target_link_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics -Wnrvo)
endif()

