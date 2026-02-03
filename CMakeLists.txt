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


set(MACOS_SOURCES "")
if (APPLE)
  enable_language(OBJCXX)
  if (NOT IOS)
  set(MACOS_SOURCES src/macos.mm)
  endif()
endif()


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${MACOS_SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})

CPMAddPackage("gh:eliaskosunen/scnlib#4b8a740")

CPMAddPackage("gh:ZXShady/enchantum#ea53e47")
target_link_libraries(${PROJECT_NAME} scn::scn enchantum::enchantum)

#enable colors on windows
#https://stackoverflow.com/a/77419222
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics)
    target_link_options(${PROJECT_NAME} PUBLIC -fansi-escape-codes -fcolor-diagnostics)
endif()