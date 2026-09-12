cmake_minimum_required(VERSION 3.20)
project(LinkSpanNativeDash LANGUAGES C)
# Build independente: NÃO faz add_subdirectory/link com shiplua ou o jogo.
set(LINKSPAN_SDK_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../../include" CACHE PATH "Headers do SDK")
set(DASH_VELOCITY "12.0" CACHE STRING "Velocidade definida pelo mod")
add_library(native_dash SHARED provider.c)
target_include_directories(native_dash PRIVATE "${LINKSPAN_SDK_INCLUDE}")
target_compile_definitions(native_dash PRIVATE DASH_VELOCITY=${DASH_VELOCITY})
set_target_properties(native_dash PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mod/provider"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mod/provider")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(native_dash PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider"
        LIBRARY_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
