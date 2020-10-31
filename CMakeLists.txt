# Lots of this style is based off of noisepage from CMU
# - https://github.com/cmu-db/noisepage/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

include(ExternalProject)

project(demodb
    VERSION 1.0
    DESCRIPTION "Example of integratting rocksdb into a C++ project"
    LANGUAGES CXX
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =====================================================================================================================
# HEADER Options
# =====================================================================================================================

option(DEMODB_OPTION
    "Demo DB option example"
    OFF
)

message(STATUS "${CMAKE_MAKE_PROGRAM}")

# =====================================================================================================================
# HEADER Git Submodules
# =====================================================================================================================
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

# =====================================================================================================================
# HEADER RocksDB 
# =====================================================================================================================

# we use an external project and copy the sources to bin directory to ensure
# that object files are built outside of the source tree.
ExternalProject_Add(rocksdb_ext
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/rocksdb"
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${PROJECT_SOURCE_DIR}/extern/rocksdb static_lib
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

# add a imported library for librocksdb.a
add_library(rocksdb STATIC IMPORTED)
add_dependencies(rocksdb rocksdb_ext)
set_property(TARGET rocksdb PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/extern/rocksdb/librocksdb.a")
target_include_directories(rocksdb SYSTEM INTERFACE "${PROJECT_SOURCE_DIR}/extern/rocksdb/include")

# =====================================================================================================================
# HEADER demodb libraries
# =====================================================================================================================
file(GLOB_RECURSE
        DEMODB_SRCS
        CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.hpp
)

list(REMOVE_ITEM DEMODB_SRCS ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_library(demodb_objlib OBJECT "${DEMODB_SRCS}")

set_target_properties(demodb_objlib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
)

target_compile_options(demodb_objlib PRIVATE
    "-Werror"
    "-Wall"
)

target_include_directories(demodb_objlib SYSTEM PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(demodb_objlib PUBLIC
    rocksdb
)

add_library(demodb_static STATIC $<TARGET_OBJECTS:demodb_objlib>)
target_link_libraries(demodb_static PUBLIC demodb_objlib)

# =====================================================================================================================
# HEADER demodb libraries
# =====================================================================================================================

add_executable(demodb src/main.cpp)

target_compile_options(demodb PRIVATE
    "-Werror"
    "-Wall"
)

target_link_libraries(demodb demodb_static)

set_target_properties(demodb PROPERTIES
    CXX_EXTENSIONS OFF
    ENABLE_EXPORTS ON
)
