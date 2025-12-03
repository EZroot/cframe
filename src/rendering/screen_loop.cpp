#include "timer.h" 
#include "media_pipe.h"

#include <ftxui/dom/elements.hpp> 
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/color.hpp>

#include <thread>
#include <iostream>
#include <cmath>
#include <vector>

using namespace ftxui;
std::atomic_bool quit_thread = false;

void RedrawThread(ftxui::ScreenInteractive* screen_ptr, float target_fps) {
    const Duration delay = Duration(1.0f / target_fps); 
    
    while (!quit_thread) {
        std::this_thread::sleep_for(delay);
        screen_ptr->PostEvent(ftxui::Event::Custom);
    }
}

auto MapFrameToCanvas = [](ftxui::Canvas& c, const std::vector<unsigned char>& data, int w, int h) {
    const int channels = 3;
    
    for (int y = 0; y < h; ++y) { 
        for (int x = 0; x < w; ++x) {
            long index = ((long)y * w + x) * channels; 

            if (index + 2 < data.size()) {
                unsigned char R = data[index];
                unsigned char G = data[index + 1];
                unsigned char B = data[index + 2];
                const ftxui::Color color_value = ftxui::Color::RGB((int)R, (int)G, (int)B);
                c.DrawBlock(x, y, true, color_value);
            }
        }
    }
};

void RunAppLoop(const float FPS_TARGET, const std::string& video_path, const bool is_url) {

    float total_elapsed_time = 0.0f;
    TimePoint last_time = Clock::now();
    std::string status_message = "Status: Initializing...";

    MediaPipe pipe;
    int w = 0, h = 0, c = 0;
    std::vector<unsigned char> current_frame_pixels;

    if (!pipe.open(video_path, w, h, c, is_url)) {
        std::cout << "Pipe failed to open video file" << std::endl;
        return;
    }
    
    const int canvas_width = w; 
    const int canvas_height = h;

    auto screen = ScreenInteractive::TerminalOutput();
    std::thread redraw_thread(RedrawThread, &screen, FPS_TARGET); 

    auto renderer = Renderer([&]{
        TimePoint current_time = Clock::now();
        Duration delta_duration = current_time - last_time;
        float delta_time = delta_duration.count();
        last_time = current_time;
        total_elapsed_time += delta_time;

        bool read_success = pipe.read_frame(current_frame_pixels);
        
        auto c = Canvas(canvas_width, canvas_height);
        
        if (read_success) {
            MapFrameToCanvas(c, current_frame_pixels, w, h);
            status_message = "Status: Playing";
        } else {
            MapFrameToCanvas(c, current_frame_pixels, w, h); 
            status_message = "Status: Video Ended";
            screen.Exit(); 
        }

        std::string dt_text = "Delta(T): " + std::to_string(delta_time) + "s";
        std::string total_dt_text = "Elapsed(T): " + std::to_string(total_elapsed_time) + "s";
        
        return vbox({
            text("(FPS: " + std::to_string(FPS_TARGET) + ") | " + status_message) | bold,
            separator(),
            canvas(c) | border,
            text(dt_text) | color(Color::Yellow),
            text(total_dt_text) | color(Color::Yellow),
        });
    });

    screen.Loop(renderer);

    quit_thread = true;
    redraw_thread.join();
}