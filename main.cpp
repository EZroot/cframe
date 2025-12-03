#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <thread>   
#include <atomic>   

using namespace ftxui;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<float>;

std::atomic_bool quit_thread = false;

void RedrawThread(ScreenInteractive* screen_ptr, float target_fps) {
    const auto delay = std::chrono::duration<float>(1.0f / target_fps); 
    while (!quit_thread) {
        std::this_thread::sleep_for(delay);
        screen_ptr->PostEvent(Event::Custom);
    }
}

int main() {
  const float FPS_TARGET = 30.0f; 
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

  // Cleanup
  quit_thread = true;
  redraw_thread.join();

  return 0;
}