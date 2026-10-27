cmake_minimum_required(VERSION 3.25)

if(NOT CMAKE_TOOLCHAIN_FILE)
    execute_process(
        COMMAND em-config EMSCRIPTEN_ROOT
        OUTPUT_VARIABLE emscriptenRoot
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    set(CMAKE_TOOLCHAIN_FILE "${emscriptenRoot}/cmake/Modules/Platform/Emscripten.cmake")
endif()

project(SubWeb LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt" pluginCMake)
string(REGEX MATCH "juce_add_plugin[^)]*[ \t\r\n]VERSION[ \t\r\n]+([0-9]+\\.[0-9]+\\.[0-9]+)" _ "${pluginCMake}")
set(pluginVersion "${CMAKE_MATCH_1}")

if(NOT pluginVersion)
    message(FATAL_ERROR "No VERSION found in juce_add_plugin in ../CMakeLists.txt")
endif()

set(CPM_DOWNLOAD_VERSION 0.43.1)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Otherwise a reconfigure drops the patch
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# The patch fixes JUCE's wasm support; delete it once JUCE has the fix
CPMAddPackage(
  NAME
    JUCE
  GIT_TAG
    9.0.2
  GITHUB_REPOSITORY
    juce-framework/JUCE
  OPTIONS
    "JUCE_MODULES_ONLY ON"
  PATCHES
    "${CMAKE_CURRENT_SOURCE_DIR}/patches/juce-wasm.patch"
)

add_executable(SubWeb
    SubEngine.cpp
    ../Source/dsp/SubBassVoice.cpp
    ../Source/dsp/SubSynth.cpp)

target_include_directories(SubWeb PRIVATE ../Source)

target_compile_definitions(SubWeb PRIVATE JUCE_STANDALONE_APPLICATION=1 SUB_PLUGIN_VERSION="${pluginVersion}")

target_link_libraries(SubWeb PRIVATE juce::juce_audio_basics)

# A bare .wasm, no Emscripten JS
target_link_options(SubWeb PRIVATE --no-entry -sSTANDALONE_WASM)

set_target_properties(SubWeb PROPERTIES
    OUTPUT_NAME "SampleRealm-Sub-${pluginVersion}"
    SUFFIX ".wasm")

find_program(NODE_EXECUTABLE node REQUIRED)
find_program(NPM_EXECUTABLE npm REQUIRED)

set(uiDir "${CMAKE_CURRENT_SOURCE_DIR}/../webui")

# Uploaded as demos/<this folder>/
set(demoDir "${CMAKE_BINARY_DIR}/SampleRealm-Sub-${pluginVersion}")

file(GLOB_RECURSE uiInputs CONFIGURE_DEPENDS
    "${uiDir}/src/*" "${uiDir}/public/demo/*" "${uiDir}/package*.json" "${uiDir}/vite.config.ts")

# npm writes this on every install
set(npmInstalled "${uiDir}/node_modules/.package-lock.json")

add_custom_command(OUTPUT "${npmInstalled}"
    COMMAND ${NPM_EXECUTABLE} ci --no-audit --no-fund
    WORKING_DIRECTORY "${uiDir}"
    DEPENDS "${uiDir}/package.json" "${uiDir}/package-lock.json"
    VERBATIM)

add_custom_command(OUTPUT "${demoDir}/index.html"
    COMMAND ${NPM_EXECUTABLE} run typecheck
    COMMAND ${NPM_EXECUTABLE} run build:demo
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${demoDir}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${uiDir}/dist/demo" "${demoDir}"
    WORKING_DIRECTORY "${uiDir}"
    DEPENDS ${uiInputs} "${npmInstalled}"
    VERBATIM)

add_custom_command(OUTPUT "${demoDir}/engine.wasm"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:SubWeb>" "${demoDir}/engine.wasm"
    DEPENDS SubWeb "${demoDir}/index.html"
    VERBATIM)

add_custom_target(SubWebDemo ALL DEPENDS "${demoDir}/engine.wasm")

enable_testing()

add_test(NAME engine
    COMMAND ${NODE_EXECUTABLE} --test "${CMAKE_CURRENT_SOURCE_DIR}/engine.test.mjs"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME ui
    COMMAND ${NPM_EXECUTABLE} test
    WORKING_DIRECTORY "${uiDir}")

set_tests_properties(engine ui PROPERTIES ENVIRONMENT "SUB_WASM=$<TARGET_FILE:SubWeb>")
