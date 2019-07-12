cmake_minimum_required(VERSION 3.8)
project(persistent-storage)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/submodules/cmake/FindBerkeleyDB")


set(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


ENABLE_TESTING()

SET(LIBRARY_NAME "persistent-storage")

include(${CMAKE_SOURCE_DIR}/cmake/SetEnv.cmake)

add_subdirectory( src )

if (BUILD_TESTING)
  add_subdirectory( test )
endif()

include(${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake)



