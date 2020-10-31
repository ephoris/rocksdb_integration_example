# Lots of this style is based off of noisepage from CMUA
# - https://github.com/cmu-db/noisepage/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

include(ExternalProject)

project(demodb
    VERSION 1.0
    DESCRIPTION "Example of integratting rocksdb into a C++ project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)

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
ExternalProject_Add(rocksdb_static_lib
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/rocksdb"
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    #     USES_TERMINAL_CONFIGURE 1
    BUILD_COMMAND $(MAKE) CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${ DISABLE_WARNING_AS_ERROR=True -C ${PROJECT_SOURCE_DIR}/extern/rocksdb static_lib
)

# add a imported library for librocksdb.a
add_library(rocksdb STATIC IMPORTED)
add_dependencies(rocksdb rocksdb_static_lib)
set_target_properties(rocksdb PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/extern/rocksdb/librocksdb.a)
target_include_directories(rocksdb INTERFACE "${PROJECT_SOURCE_DIR}/extern/rocksdb/include")
# include_directories("${PROJECT_SOURCE_DIR}/extern/rocksdb/include")

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

# set_target_properties(demodb_objlib PROPERTIES
#     POSITION_INDEPENDENT_CODE ON
#     CXX_EXTENSIONS OFF
# )

target_compile_options(demodb_objlib PRIVATE
    "-Werror"
    "-Wall"
)

target_include_directories(demodb_objlib PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/extern"
)

target_link_libraries(demodb_objlib PUBLIC
    rocksdb
)

add_library(demodb_static STATIC $<TARGET_OBJECTS:demodb_objlib>)
target_link_libraries(demodb_static PUBLIC demodb_objlib)

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main PUBLIC demodb_objlib rocksdb)
