# Require a recent version of CMake (3.14+ for FetchContent)
cmake_minimum_required(VERSION 3.14) 

# Define the project name and C++ standard (FTXUI requires C++17)
project(KubuntuTUIProject CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Useful for IDEs like VS Code/Kate

# --- Fetch FTXUI Source Code ---
include(FetchContent)

# Declare where to find FTXUI
FetchContent_Declare(
    ftxui_content
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v4.0.0 
)

# Download and make the content available to the build
FetchContent_MakeAvailable(ftxui_content)
# -------------------------------

# Define your application executable
# --- FIX: List all source files relative to the project root ---
add_executable(cframe
    src/core/main.cpp           
    src/rendering/screen_loop.cpp 
    src/media/media_pipe.cpp 
)

# Link your executable against the necessary FTXUI targets/modules
# These are necessary because you use ScreenInteractive, Renderer, Canvas, etc.
target_link_libraries(cframe PRIVATE 
    ftxui::dom
    ftxui::screen
    ftxui::component
)

# Optional but Recommended: Add the 'src' directory to the include path.
# This allows files in 'src/core/' to include headers as #include <whatever>
target_include_directories(cframe PRIVATE
    src/media
    src/rendering
)