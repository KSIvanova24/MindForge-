cmake_minimum_required(VERSION 3.20)
project(MindForge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/presentation.cpp
)

add_executable(MindForge ${SOURCES})

target_include_directories(MindForge PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/library/include
)

if(WIN32)
    target_link_libraries(MindForge PRIVATE
        ${CMAKE_SOURCE_DIR}/library/lib/raylib.lib
        winmm
        gdi32
        opengl32
    )

    add_custom_command(TARGET MindForge POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/library/lib/raylib.dll"
            "$<TARGET_FILE_DIR:MindForge>/raylib.dll"
    )
else()
    find_package(raylib REQUIRED)
    target_link_libraries(MindForge PRIVATE raylib)
endif()

if(MSVC)
    target_compile_options(MindForge PRIVATE /W3 /utf-8)
    set_target_properties(MindForge PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()