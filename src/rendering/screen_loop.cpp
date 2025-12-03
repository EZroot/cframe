#include "timer.h" 

#include <ftxui/dom/elements.hpp> 
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <thread>
#include <iostream>
#include <cmath>

std::atomic_bool quit_thread = false;

void RedrawThread(ftxui::ScreenInteractive* screen_ptr, float target_fps) {
    const Duration delay = Duration(1.0f / target_fps); 
    
    while (!quit_thread) {
        std::this_thread::sleep_for(delay);
        screen_ptr->PostEvent(ftxui::Event::Custom);
    }
}

void RunAppLoop(const float FPS_TARGET) {
    using namespace ftxui;

    float total_elapsed_time = 0.0f;
    TimePoint last_time = Clock::now();

    auto screen = ScreenInteractive::TerminalOutput();
    
    std::thread redraw_thread(RedrawThread, &screen, FPS_TARGET); 

    auto renderer = Renderer([&]{
        TimePoint current_time = Clock::now();
        Duration delta_duration = current_time - last_time;
        float delta_time = delta_duration.count();
        last_time = current_time;
        total_elapsed_time += delta_time;

        int canvas_width = 80;
        int canvas_height = 40;
        
        float amplitude_x = canvas_width / 4.0f;
        float amplitude_y = canvas_height / 4.0f;
        
        int animated_x = static_cast<int>(canvas_width / 2.0f + amplitude_x * std::sin(total_elapsed_time * 1.5f));
        int animated_y = static_cast<int>(canvas_height / 2.0f + amplitude_y * std::cos(total_elapsed_time * 1.0f));

        auto c = Canvas(canvas_width, canvas_height);
        c.DrawPoint(animated_x, animated_y, Color::Green);
        c.DrawPoint(animated_x + 1, animated_y, Color::Green);
        c.DrawPoint(animated_x, animated_y + 1, Color::Green);
        c.DrawPoint(animated_x + 1, animated_y + 1, Color::Green);

        std::string dt_text = "Delta(T): " + std::to_string(delta_time) + "s";
        std::string total_dt_text = "Elapsed(T): " + std::to_string(total_elapsed_time) + "s";

        return vbox({
            text("(FPS: " + std::to_string(FPS_TARGET) + ")") | bold,
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