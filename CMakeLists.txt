# Require a recent version of CMake (3.14+ for FetchContent)
cmake_minimum_required(VERSION 3.14) 

# Define the project name and C++ standard (FTXUI requires C++17)
project(KubuntuTUIProject CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Fetch FTXUI Source Code ---
include(FetchContent)

# Declare where to find FTXUI
FetchContent_Declare(
    ftxui_content
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v4.0.0 # Use a stable tag to ensure reproducibility
)

# Download and make the content available to the build
FetchContent_MakeAvailable(ftxui_content)
# -------------------------------

# Define your application executable
add_executable(cframe main.cpp)

# Link your executable against the necessary FTXUI targets/modules
# You only need ftxui::dom and ftxui::screen for this example.
target_link_libraries(cframe PRIVATE 
	ftxui::dom
	ftxui::screen
	ftxui::component
)
