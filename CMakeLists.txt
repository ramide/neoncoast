cmake_minimum_required(VERSION 3.20)
project(neoncoast C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

file(GLOB GAME_SOURCES src/*.c)
add_executable(neoncoast ${GAME_SOURCES})
target_link_libraries(neoncoast raylib)
target_include_directories(neoncoast PRIVATE src)

if(WIN32)
    target_link_libraries(neoncoast winmm gdi32)
elseif(APPLE)
    target_link_libraries(neoncoast "-framework Cocoa" "-framework IOKit")
else()
    target_link_libraries(neoncoast pthread m dl X11)
endif()

target_compile_definitions(neoncoast PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")
