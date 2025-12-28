cmake_minimum_required(VERSION 3.16)
project(OmbraGUI LANGUAGES CXX C)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Windows-specific definitions
add_definitions(
    -DUNICODE
    -D_UNICODE
    -DWIN32_LEAN_AND_MEAN
    -DNOMINMAX
)

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# GUI source files
set(GUI_SOURCES
    src/main.cpp
    src/app.cpp
    src/auth.cpp
    src/dx11_backend.cpp
    src/spoofer_ui.cpp
)

# Loader source files from parent directory
# NOTE: loader_api.c and payload_loader.c removed - legacy code using broken structures
# TODO: GUI needs migration to use hv_loader.c API instead
set(LOADER_SOURCES
    ../driver_interface.c
    ../loader/hv_loader.c
    ../loader/pe_parser.c
    ../loader/pe_mapper.c
    ../loader/pe_relocs.c
    ../loader/pe_iat.c
    ../loader/pe_imports.c
    ../loader/pe_utils.c
    ../loader/pe_wipe.c
    ../byovd/deployer.c
    ../byovd/supdrv.c
    ../byovd/throttlestop.c
    ../byovd/crypto.c
    ../byovd/nt_helpers.c
)

# Force C files to compile as C (not C++)
set_source_files_properties(${LOADER_SOURCES} PROPERTIES LANGUAGE C)

# Create executable
add_executable(OmbraGUI WIN32
    ${IMGUI_SOURCES}
    ${GUI_SOURCES}
    ${LOADER_SOURCES}
)

# Include directories
target_include_directories(OmbraGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../byovd
    ${CMAKE_CURRENT_SOURCE_DIR}/../loader
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(OmbraGUI PRIVATE
    d3d11
    dxgi
    user32
    gdi32
    shell32
    advapi32
    iphlpapi
)

# Compiler flags
if(MSVC)
    target_compile_options(OmbraGUI PRIVATE
        /W3
        /MP
        /permissive-
        /Zc:__cplusplus
    )

    # Optimization flags for Release
    target_compile_options(OmbraGUI PRIVATE
        $<$<CONFIG:Release>:/O2 /GL>
    )

    # Link-time optimization for Release
    set_target_properties(OmbraGUI PROPERTIES
        LINK_FLAGS_RELEASE "/LTCG /SUBSYSTEM:WINDOWS"
    )
else()
    target_compile_options(OmbraGUI PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Post-build: Copy resources directory to output
add_custom_command(TARGET OmbraGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:OmbraGUI>/resources
    COMMENT "Copying resources to output directory"
)

# Installation
install(TARGETS OmbraGUI
    RUNTIME DESTINATION bin
)

install(DIRECTORY resources
    DESTINATION bin
)
