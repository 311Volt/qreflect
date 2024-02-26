cmake_minimum_required(VERSION 3.25)
project(qreflect)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(qreflect INTERFACE "include/qreflect.hpp")
target_include_directories(qreflect INTERFACE "include")

if (NOT DEFINED QREFLECT_MASTER_PROJECT)
    set(QREFLECT_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(QREFLECT_MASTER_PROJECT ON)
    endif ()
endif ()

option(QREFLECT_BUILD_EXAMPLES "Build example programs" ${QREFLECT_MASTER_PROJECT})

if (QREFLECT_BUILD_EXAMPLES)
    add_executable(qrexample "examples/example.cpp")
    target_link_libraries(qrexample PRIVATE qreflect)

    add_executable(qrsqlite_ddl "examples/sqlite_ddl.cpp")
    target_link_libraries(qrsqlite_ddl PRIVATE qreflect)

    find_package(nlohmann_json)
    if(TARGET nlohmann_json::nlohmann_json)
        add_executable(qrjson "examples/json.cpp")
        target_link_libraries(qrjson PRIVATE qreflect nlohmann_json::nlohmann_json)
    endif ()
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)