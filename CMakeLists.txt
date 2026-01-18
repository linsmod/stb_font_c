cmake_minimum_required(VERSION 3.10)
project(stb_font_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(ENABLE_EXAMPLES "Build example programs" ON)
option(ENABLE_SDL3 "Use SDL3 instead of SDL2 (default: SDL2)" OFF)

# Find SDL
if(ENABLE_SDL3)
    find_package(SDL3 QUIET)
    if(SDL3_FOUND)
        set(SDL_LIBRARIES SDL3::SDL3)
        set(SDL_INCLUDE_DIRS "")
        message(STATUS "Using SDL3")
    else()
        message(WARNING "SDL3 not found, falling back to SDL2")
    endif()
endif()

if(NOT SDL_LIBRARIES)
    find_package(SDL2 REQUIRED)
    set(SDL_LIBRARIES SDL2::SDL2)
    set(SDL_INCLUDE_DIRS "")
    message(STATUS "Using SDL2")
endif()

# Source files
set(LIB_SOURCES
    stb_font_cache.c
)

set(LIB_HEADERS
    stb_font_cache.h
)

# Create static library
add_library(stb_font_c STATIC ${LIB_SOURCES})
target_include_directories(stb_font_c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(stb_font_c PUBLIC STB_FONT_SDL_ENABLED)
target_link_libraries(stb_font_c PUBLIC ${SDL_LIBRARIES} m)
target_include_directories(stb_font_c PRIVATE ${SDL_INCLUDE_DIRS})

# Example program
if(ENABLE_EXAMPLES)
    add_executable(example example.c)
    target_link_libraries(example stb_font_c ${SDL_LIBRARIES})
    target_compile_definitions(example PRIVATE STB_FONT_SDL_ENABLED)
endif()

# Installation
install(TARGETS stb_font_c
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES ${LIB_HEADERS}
    DESTINATION include
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build examples: ${ENABLE_EXAMPLES}")
message(STATUS "  Use SDL3: ${ENABLE_SDL3}")
message(STATUS "")
