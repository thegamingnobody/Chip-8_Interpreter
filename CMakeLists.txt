# CMakeList.txt : CMake project for Chip-8_Interpreter, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.16)
project(Chip-8_Interpreter)

set(CMAKE_CXX_STANDARD 20)

#Fetch libraries
include(FetchContent)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.4
    )
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    Imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8-docking
    )
FetchContent_MakeAvailable(Imgui)

add_library(GLAD STATIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include/glad/glad.h"	
	"${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c")
target_include_directories(GLAD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")


set(PROJECT_SRC
    "src/Main.cpp"
    "src/Chip-8_Interpreter.cpp"
    "src/Chip-8_Interpreter.h"    
	"src/Singleton.h"
    "src/Renderer.cpp"
    "src/Renderer.h" 
    "src/InputManager.cpp"
    "src/InputManager.h"
    "src/TypeDefinitions.h"
    "src/Logger.cpp"
    "src/Logger.h" 
)
set(IMGUI_SRC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Add source to this project's executable.
add_executable(${PROJECT_NAME} ${PROJECT_SRC} ${IMGUI_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})

# Link SDL to project
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Include src in project
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/roms)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)