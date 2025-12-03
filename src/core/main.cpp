#include <ftxui/dom/elements.hpp> 
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <iostream>

void RunAppLoop(const float FPS_TARGET, const std::string& video_path, const bool is_url); 

const std::string DEFAULT_VIDEO_PATH = "/home/ezroot/Downloads/Download(2).mp4";
const float FPS_TARGET = 30.0f; 

int main(int argc, char* argv[]) {
    std::string video_path = DEFAULT_VIDEO_PATH;
    bool is_url;

    std::vector<std::string> args(argv + 1, argv + argc);
    std::string path_candidate;

    for (const auto& arg : args) {
        if (arg == "-u" || arg == "-url" || arg == "--url") {
            is_url = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: ./cframe [OPTIONS] <path_to_video_file_or_url>" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -u, --url    Treat the path argument as a network stream (URL)." << std::endl;
            return 0; 
        } else {
            path_candidate = arg;
        }
    }

    if (!path_candidate.empty()) {
        video_path = path_candidate;
        std::cout << "Using provided source: " << video_path;
        if (is_url) {
            std::cout << " (Mode: URL Stream)";
        } else {
            std::cout << " (Mode: Local File)";
        }
        std::cout << std::endl;
    } else {
        if (video_path.find("http://") == 0 || video_path.find("https://") == 0) {
            is_url = true;
        }
        std::cout << "No source provided. Using default: " << video_path;
        if (is_url) {
            std::cout << " (Mode: URL Stream)";
        }
        std::cout << std::endl;
    }
    
    if (path_candidate.empty() && is_url) {
        std::cerr << "Warning: '--url' flag specified but no path was provided. Using default path." << std::endl;
    }

    RunAppLoop(FPS_TARGET, video_path, is_url);
    return 0;
}