#pragma once


#include <array>
#include <chrono>



template<size_t history_count>
class FpsWindow
{
private:
    std::chrono::time_point<std::chrono::steady_clock> last_render_start;

    std::array<float, history_count> fps_history;
    size_t fps_history_next_index;
    size_t fps_history_count;
    float fps_sum;

    float fps;

public:
    FpsWindow();

    float getFps();
    
    float frame();
};



template<size_t history_count>
FpsWindow<history_count>::FpsWindow()
{
    last_render_start = std::chrono::time_point<std::chrono::steady_clock>();

    fps_history_next_index = 0;
    fps_history_count = 0;
    fps_sum = 0.0f;

    fps = 0.0f;
}

template<size_t history_count>
float FpsWindow<history_count>::getFps() { return fps; }

template<size_t history_count>
float FpsWindow<history_count>::frame()
{
    std::chrono::time_point<std::chrono::steady_clock> render_start = std::chrono::steady_clock::now();

    std::chrono::steady_clock::duration duration = render_start - last_render_start;
    float new_fps = static_cast<float>(std::chrono::steady_clock::duration::period::den) / (duration.count() * std::chrono::steady_clock::duration::period::num);

    if (fps_history_count == history_count) {
        fps_sum -= fps_history[fps_history_next_index];
    } else {
        fps_history_count++;
    }

    fps_history[fps_history_next_index] = new_fps;
    fps_history_next_index = (fps_history_next_index + 1) % history_count;
    fps_sum += new_fps;

    last_render_start = render_start;

    fps = fps_sum / fps_history_count;

    return fps;
}