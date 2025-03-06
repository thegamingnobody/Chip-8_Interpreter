# CMakeList.txt : CMake project for Chip-8_Interpreter, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Chip-8_Interpreter "Chip-8_Interpreter.cpp" "Chip-8_Interpreter.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chip-8_Interpreter PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
