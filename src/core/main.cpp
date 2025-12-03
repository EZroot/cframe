#include <ftxui/dom/elements.hpp> 
#include <ftxui/dom/canvas.hpp>
#include <ftxui/component/component.hpp>
#include <iostream>

void RunAppLoop(const float FPS_TARGET); 

int main() {
    const float FPS_TARGET = 30.0f; 
    RunAppLoop(FPS_TARGET);
    return 0;
}