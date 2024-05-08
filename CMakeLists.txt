cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -Wextra")

project(Game_of_life VERSION 0.1 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

include(FindSDL2_gfx)

find_package(SDL2_gfx REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_GFX_INCLUDE_DIRS})

add_executable(gof src/main.c src/cells.c)
target_link_libraries(gof ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES})
