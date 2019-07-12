cmake_minimum_required(VERSION 3.8)
project(PersistentStorage DESCRIPTION "mylib description" VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/submodules/cmake/FindBerkeleyDB")

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin
        CACHE PATH
        "Single Directory for all"
    )

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin
        CACHE PATH
        "Single Directory for all"
    )

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/lib
        CACHE PATH
        "Single Directory for all"
    )

SET (DOCUMENTS_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/documents
        CACHE PATH
        "Single Directory for all"
    )


# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

ENABLE_TESTING()

add_subdirectory( src )

if (BUILD_TESTING)
  add_subdirectory( test )
endif()

set_target_properties(PersistentStorage PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(PersistentStorage PROPERTIES SOVERSION 1)

target_include_directories(PersistentStorage PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/src submodules/eventpp/include)





