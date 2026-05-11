cmake_minimum_required(VERSION 3.20)
project(MindForge LANGUAGES C CXX)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/presentation.cpp
    src/dashboard.cpp
    src/tasks.cpp
    src/login.cpp
    src/profile.cpp
    src/statistics.cpp
    src/accounts.cpp
    src/db.cpp
    src/data.cpp
    src/logic.cpp
    library/src/sqlite3.c
)

add_executable(MindForge ${SOURCES})

target_include_directories(MindForge PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/library/include
    ${CMAKE_SOURCE_DIR}/library/src
)

if(WIN32)
    if(MINGW)
        target_link_libraries(MindForge PRIVATE
            ${CMAKE_SOURCE_DIR}/library/lib/libraylibdll.a
            winmm
            gdi32
            opengl32
        )
    else()
        target_link_libraries(MindForge PRIVATE
            ${CMAKE_SOURCE_DIR}/library/lib/raylib.lib
            winmm
            gdi32
            opengl32
        )
    endif()

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
    target_compile_options(MindForge PRIVATE /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS)
    set_target_properties(MindForge PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()