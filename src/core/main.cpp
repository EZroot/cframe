#include <ftxui/dom/elements.hpp> 
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <iostream>

void RunAppLoop(const float FPS_TARGET, const std::string& video_path); 

const std::string DEFAULT_VIDEO_PATH = "/home/ezroot/Downloads/Download(2).mp4";

int main(int argc, char* argv[]) {
    std::string video_path;

    if (argc > 1) {
        video_path = argv[1]; 
        std::cout << "Using provided video file: " << video_path << std::endl;
    } else {
        video_path = DEFAULT_VIDEO_PATH;
        std::cout << "No video path provided. Using default: " << video_path << std::endl;
        std::cout << "Usage: ./cframe <path_to_vid>" << std::endl;
    }

    const float FPS_TARGET = 30.0f; 
    RunAppLoop(FPS_TARGET, video_path);
    return 0;
}