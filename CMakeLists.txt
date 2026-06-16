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

    # Build genicon tool (standalone - no raylib deps)
    add_executable(genicon EXCLUDE_FROM_ALL tools/genicon.c)

    # Generate .ico at build time
    add_custom_target(generate_icon ALL
        COMMAND genicon
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/neoncoast.ico
            ${CMAKE_SOURCE_DIR}/res/neoncoast.ico
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS genicon
        COMMENT "Generating neoncoast.ico...")

    add_dependencies(neoncoast generate_icon)
    target_sources(neoncoast PRIVATE res/resources.rc)
elseif(APPLE)
    target_link_libraries(neoncoast "-framework Cocoa" "-framework IOKit")
elseif(ANDROID)
else()
    target_link_libraries(neoncoast pthread m dl X11)
endif()

target_compile_definitions(neoncoast PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")
target_compile_definitions(neoncoast PRIVATE SUPPORT_FILEFORMAT_OGG=1)
