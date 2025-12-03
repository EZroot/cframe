#include "media_pipe.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <regex>

const int TARGET_CHANNELS = 3; 
const std::string PIXEL_FORMAT = "rgb24";
const int TARGET_PIXEL_WIDTH = 160;

MediaPipe::~MediaPipe() {
    close();
}

bool MediaPipe::get_dimensions(const std::string& video_path, int& w, int& h) {
    std::stringstream ss;
    ss << "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0:s=x "
       << "-f lavfi -i \"movie='" << video_path << "',scale=" << TARGET_PIXEL_WIDTH << ":-1,setsar=2/1[out];[out]null\"";

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
    
    std::regex pattern("(\\d+)x(\\d+)");
    std::smatch match;
    if (std::regex_search(result, match, pattern) && match.size() == 3) {
        try {
            w = std::stoi(match[1].str());
            h = std::stoi(match[2].str());
            return true;
        } catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to parse dimensions: " << e.what() << std::endl;
            return false;
        }
    }

    std::cerr << "ERROR: Could not parse video dimensions from ffprobe output: " << result << std::endl;
    return false;
}

/// @brief Open a ffmpeg pipe
/// @param video_path 
/// @param out_width The actual pixel width of the scaled video frame
/// @param out_height The actual pixel height of the scaled video frame
/// @param out_channels 
/// @return 
bool MediaPipe::open(const std::string& video_path, int& out_width, int& out_height, int& out_channels) {
    if (pipe_) {
        close();
    }

    if (!get_dimensions(video_path, width_, height_)) {
        return false;
    }

    std::stringstream ss;
    ss << "ffmpeg -i \"" << video_path << "\" " 
       << "-f rawvideo "
       << "-pix_fmt " << PIXEL_FORMAT << " "
       << "-vcodec rawvideo "
       << "-an " // disable audio for now
       << "-vf scale=" << TARGET_PIXEL_WIDTH << ":-1,setsar=2/1 " 
       << "-loglevel error " // ignore logs unless errors
       << "-";              // output to stdout 
       
    command_ = ss.str();
    
    std::cout << "Executing FFmpeg command: " << command_ << std::endl;

    // popen executes the command and returns a FILE pointer to the output stream
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

/// @brief Read a single frame and return the pixels
/// @param frame_data 
/// @return 
bool MediaPipe::read_frame(std::vector<unsigned char>& frame_data) {
    if (!pipe_) {
        std::cerr << "Pipe is not open." << std::endl;
        return false;
    }

    frame_data.resize(frame_size_);
    
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

/// @brief Close and cleanup ffmpeg pipe
void MediaPipe::close() {
    if (pipe_) {
        // pclose closes the stream and waits for the FFmpeg process to terminate
        pclose(pipe_); 
        pipe_ = nullptr;
        std::cout << "FFmpeg pipe closed." << std::endl;
    }
}