# CMakeList.txt : CMake project for Chip-8_Interpreter, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.19)
project(Chip-8_Interpreter)

set(CMAKE_CXX_STANDARD 20)

#Fetch libraries
include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.11
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.1
)
FetchContent_MakeAvailable(SDL2_mixer)

FetchContent_Declare(
    Imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8-docking
    )
FetchContent_MakeAvailable(Imgui)

FetchContent_Declare(
	nlohmannJson 
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmannJson)

find_package(OpenSSL REQUIRED)

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
    "src/TimeManager.h" 
    "src/TimeManager.cpp" 
    "src/QuirkManager.h" 
    "src/QuirkManager.cpp"
)

set(IMGUI_SRC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp")

# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Add source to this project's executable.
add_executable(${PROJECT_NAME} ${PROJECT_SRC} ${IMGUI_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})

# Link SDL to project
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2::SDL2 SDL2_mixer::SDL2_mixer OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json)

# Include src in project
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/roms)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
