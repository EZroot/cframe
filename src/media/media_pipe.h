#pragma once

#include <string>
#include <cstdio>
#include <vector>

class MediaPipe {
public:
    MediaPipe() = default;
    ~MediaPipe();
    
    bool open(const std::string& video_path, int& out_width, int& out_height, int& out_channels);
    bool read_frame(std::vector<unsigned char>& frame_data);
    void close();
    bool get_dimensions(const std::string& video_path, int& w, int& h);

private:
    FILE* pipe_ = nullptr; 
    long frame_size_ = 0;  
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    std::string command_;
};