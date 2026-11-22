cmake_minimum_required(VERSION 3.18)
project(facet-rotation VERSION 0.1.0 LANGUAGES CXX)
set(ROT_VERSION_SUFFIX "alpha")  # pre-release tag; empty for final releases; keep manifest.json in sync

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(ROT_STATIC "Link fully static (portable release binaries)" OFF)

if(ROT_VERSION_SUFFIX)
    set(ROT_VERSION "${PROJECT_VERSION}-${ROT_VERSION_SUFFIX}")
else()
    set(ROT_VERSION "${PROJECT_VERSION}")
endif()

# The plugin SDK lives in facet-core: a local checkout next to this project
# (or FACET_CORE_DIR) is used when present, otherwise it is downloaded.
set(FACET_CORE_DIR "" CACHE PATH "facet-core checkout providing the SDK (default: ../facet-core, else download)")
set(FACET_CORE_REF "v0.9.0-alpha" CACHE STRING "facet-core git ref to download the SDK from (rotation needs 0.9+)")
if(NOT FACET_CORE_DIR AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../facet-core/sdk/CMakeLists.txt")
    set(FACET_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../facet-core")
endif()
if(FACET_CORE_DIR)
    add_subdirectory(${FACET_CORE_DIR}/sdk ${CMAKE_BINARY_DIR}/facet_sdk)
else()
    include(FetchContent)
    FetchContent_Declare(facet_core
        GIT_REPOSITORY https://github.com/siakinnik/facet-core.git
        GIT_TAG ${FACET_CORE_REF}
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR sdk)
    FetchContent_MakeAvailable(facet_core)
endif()

add_library(rot_logic STATIC src/orientation.cpp)
target_include_directories(rot_logic PUBLIC src)
target_compile_options(rot_logic PRIVATE -Wall -Wextra)

add_executable(rotation src/main.cpp src/sensor.cpp src/i18n/ru.cpp)
target_link_libraries(rotation PRIVATE rot_logic facet_sdk)
target_compile_options(rotation PRIVATE -Wall -Wextra)
target_compile_definitions(rotation PRIVATE ROT_VERSION="${ROT_VERSION}")
if(ROT_STATIC)
    target_link_options(rotation PRIVATE -static -s)
endif()

# The build dir doubles as a plugin dir: FACET_PLUGIN_PATH=<build>
set(PLUGIN_OUT ${CMAKE_BINARY_DIR}/rotation.plugin)
set_target_properties(rotation PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUT})
facet_plugin_manifest(${CMAKE_CURRENT_SOURCE_DIR}/manifest.json ${PLUGIN_OUT}/manifest.json)

enable_testing()
add_executable(orientation_test tests/orientation_test.cpp)
target_link_libraries(orientation_test PRIVATE rot_logic)
add_test(NAME orientation COMMAND orientation_test)

install(TARGETS rotation RUNTIME DESTINATION lib/facet/plugins/rotation)
install(FILES ${PLUGIN_OUT}/manifest.json DESTINATION lib/facet/plugins/rotation)
