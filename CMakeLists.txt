cmake_minimum_required(VERSION 3.20)
project(neoncoast C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(third_party/raylib)

file(GLOB GAME_SOURCES src/*.c)
add_executable(neoncoast ${GAME_SOURCES})
target_link_libraries(neoncoast raylib)
# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
target_include_directories(neoncoast PRIVATE src)

if(WIN32)
    target_link_libraries(neoncoast winmm gdi32)
elseif(APPLE)
    target_link_libraries(neoncoast "-framework Cocoa" "-framework IOKit")
elseif(ANDROID)
else()
    target_link_libraries(neoncoast pthread m dl X11)
endif()

target_compile_definitions(neoncoast PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")
