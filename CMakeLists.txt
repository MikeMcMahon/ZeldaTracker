cmake_minimum_required(VERSION 3.5)
project(ZeldaTracker)

set (SOURCE_FILES main.c GameMath.h GameElements.h)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -mwindows")

find_package(SDL)
include_directories($ENV{DEVPATH}\\headers)
file(GLOB SPRITE_SHEETS
        "*.gif"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2 SDL2_image SDL2_ttf)