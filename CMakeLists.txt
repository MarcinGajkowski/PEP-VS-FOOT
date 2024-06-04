cmake_minimum_required(VERSION 3.5)
project(mygame)

# Create an option to switch between a system sdl library and a vendored sdl library
option(MYGAME_VENDORED "Use vendored libraries" ON)

if(MYGAME_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
    # add_subdirectory(vendored/sdl2_image EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    list(APPEND CMAKE_PREFIX_PATH VisualC/pkg-support/cmake/sdl2-config.cmake)
    # list(APPEND CMAKE_PREFIX_PATH VisualC/pkg-support/cmake/sdl2_image-config.cmake)
    # set(SDL2_DIR vendored/sdl/VisualC/pkg-support/cmake/sdl2-config.cmake)
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    # find_package(SDL2_image REQUIRED CONFIG REQUIRED COMPONENTS SDL2_image)

    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
    # find_package(SDL2_image REQUIRED CONFIG COMPONENTS SDL2_image)
endif()

# Create your game executable target as usual
add_executable(mygame WIN32 mygame.cpp
        vendored/sdl/include/SDL.h
)
# target_sources(mygame PRIVATE mygame.cpp
#         vendored/SDL2_image/include/SDL_image.h)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(mygame PRIVATE SDL2::SDL2main)
    # target_link_libraries(mygame PRIVATE SDL2_image::SDL2_image)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(mygame PRIVATE SDL2::SDL2)
# target_link_libraries(mygame PRIVATE SDL2_image::SDL2_image)