#include "timelog.hpp"


#include <iostream>
#include <iomanip>



Timelog::Timelog() : last(std::nullopt) {};

void Timelog::start()
{
    last = std::chrono::steady_clock::now();
}

void Timelog::step(std::string_view s)
{
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

    std::cout << s << "\n";

    if (last.has_value()) {
        int64_t ns = (now - last.value()).count() / 1000;
        std::cout << "time : " << (ns / 1000) << "." << std::setw(3) << std::setfill('0') << (ns % 1000) << " ms\n";
    } else {
        std::cout << "time : 0.000 ms\n";
    }

    last = now;
}