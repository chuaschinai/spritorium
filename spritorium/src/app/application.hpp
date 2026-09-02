#pragma once

#include <chrono>

constexpr double FPS_GOAL { 1000.0 / 60.0 };

struct Application {
    Application();
    ~Application();

    void Run();

private:
    void Begin();
    void End();

    std::chrono::duration<double, std::milli> FpsTarget { FPS_GOAL };
    std::chrono::time_point<std::chrono::high_resolution_clock> FpsStart;
};