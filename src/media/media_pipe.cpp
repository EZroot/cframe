#include "media_pipe.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <cmath>
#include <cstdio>
#include <vector>

const int TARGET_CHANNELS = 3; 
const std::string PIXEL_FORMAT = "rgb24";
const int TARGET_PIXEL_WIDTH = 244; 

MediaPipe::~MediaPipe() {
    close();
}

bool MediaPipe::get_dimensions(const std::string& video_path, int& w, int& h, bool is_url) {
    std::stringstream ss;
    ss << "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0:s=x "
       << "-i \"" << video_path << "\"";

    std::string probe_command = ss.str();

    FILE* probe_pipe = popen(probe_command.c_str(), "r");
    if (!probe_pipe) {
        std::cerr << "ERROR: Failed to open pipe to FFprobe." << std::endl;
        return false;
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), probe_pipe) != nullptr) {
        result += buffer;
    }
    pclose(probe_pipe);
    
    // The output from ffprobe will be original_width * original_height
    std::regex pattern("(\\d+)x(\\d+)");
    std::smatch match;
    if (std::regex_search(result, match, pattern) && match.size() == 3) {
        try {
            int orig_w = std::stoi(match[1].str());
            int orig_h = std::stoi(match[2].str());

            if (orig_w <= 0 || orig_h <= 0) {
                std::cerr << "ERROR: Invalid dimensions returned: " << orig_w << "x" << orig_h << std::endl;
                return false;
            }

            // Set final width
            w = TARGET_PIXEL_WIDTH;

            // Calculate height
            h = static_cast<int>(std::round((double)orig_h * (double)w / (double)orig_w));
            return true;
        } catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to parse dimensions: " << e.what() << std::endl;
            return false;
        }
    }

    std::cerr << "ERROR: Could not parse video dimensions from ffprobe output: " << result << std::endl;
    return false;
}

/// @brief Open a video (or url)
/// @param video_path 
/// @param out_width 
/// @param out_height 
/// @param out_channels 
/// @param is_url 
/// @return 
bool MediaPipe::open(const std::string& video_path, int& out_width, int& out_height, int& out_channels, bool is_url) {
    if (pipe_) {
        close();
    }
    if (!get_dimensions(video_path, width_, height_, is_url)) {
        return false;
    }
    
    std::stringstream ss;
    ss << "ffmpeg -i \"" << video_path << "\" " 
       << "-f rawvideo "
       << "-pix_fmt " << PIXEL_FORMAT << " "
       << "-vcodec rawvideo "
       << "-an " // disable audio for now
       << "-vf \"scale=" << width_ << ":" << height_ << ",setsar=2/1\" " 
       << "-loglevel error " // ignore logs unless errors
       << "-";              // output raw video to stdout 
       
    command_ = ss.str(); 

    std::cout << "Executing FFmpeg command: " << command_ << std::endl;

    pipe_ = popen(command_.c_str(), "r"); 
    if (!pipe_) {
        std::cerr << "ERROR: Failed to open pipe to FFmpeg." << std::endl;
        return false;
    }
    
    channels_ = TARGET_CHANNELS;
    
    frame_size_ = (long)width_ * height_ * channels_;

    out_width = width_;
    out_height = height_;
    out_channels = channels_;

    return true;
}

/// @brief Read the current frame and return the pixel data
/// @param frame_data 
/// @return 
bool MediaPipe::read_frame(std::vector<unsigned char>& frame_data) {
    if (!pipe_) {
        std::cerr << "Pipe is not open." << std::endl;
        return false;
    }

    if (frame_data.size() != frame_size_) {
        frame_data.resize(frame_size_);
    }
    
    size_t bytes_read = fread(frame_data.data(), 1, frame_size_, pipe_);

    if (bytes_read == 0) {
        std::cout << "End of video stream reached." << std::endl;
        return false;
    }

    if (bytes_read < frame_size_) {
        std::cerr << "Warning: Read incomplete frame (" << bytes_read << "/" << frame_size_ << " bytes)." << std::endl;
        return false;
    }

    return true;
}

void MediaPipe::close() {
    if (pipe_) {
        // pclose closes the stream and waits for the FFmpeg process to terminate
        pclose(pipe_); 
        pipe_ = nullptr;
        std::cout << "FFmpeg pipe closed." << std::endl;
    }
}