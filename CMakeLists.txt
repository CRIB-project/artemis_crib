# General settings
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(
  artemis-crib
  VERSION 2025.1.0
  LANGUAGES CXX)

# Default installation dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      ${CMAKE_SOURCE_DIR}/install
      CACHE PATH "..." FORCE)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# path definitions
find_package(artemis HINTS ${ARTPATH} REQUIRED)
find_package(yaml-cpp HINTS ${ARTEMIS_YAML_CPP_LIB_DIR}/../ REQUIRED)

find_package(TSrim)
if(TSrim_FOUND)
  message(STATUS "Found TSrim library")
else()
  message(STATUS "Not Found TSrim library")
endif()

# ROOT settings
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

add_subdirectory(src-crib)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/.thisartemis-crib.sh.in"
               "${CMAKE_SOURCE_DIR}/thisartemis-crib.sh" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/.artemislogon.C.in"
               "${CMAKE_SOURCE_DIR}/artemislogon.C" @ONLY)

# make uninstall
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
  TARGET "uninstall"
  POST_BUILD
  COMMENT "Uninstall files with install_manifest.txt"
  COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
          install_manifest.txt to be uninstalled!)
