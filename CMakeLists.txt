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
    GIT_TAG release-3.2.8
    )
FetchContent_MakeAvailable(SDL3)

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
    "src/ScreenManager.cpp"
    "src/ScreenManager.h")

# Add source to this project's executable.
add_executable(${PROJECT_NAME} ${PROJECT_SRC})

# Link SDL to project
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Include src in project
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)