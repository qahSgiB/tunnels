#pragma once



#include <optional>
#include <string_view>
#include <chrono>



class Timelog
{
private:
    std::optional<std::chrono::time_point<std::chrono::steady_clock>> last;

public:
    Timelog();

    void start();
    void step(std::string_view s);
};