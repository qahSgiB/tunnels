#pragma once


#include <chrono>



/* algorithm debug timing */
namespace adt
{
    using clock = std::chrono::steady_clock;
    using duration = std::chrono::steady_clock::duration;
    using time_point = std::chrono::steady_clock::time_point;
    using milli_int = std::chrono::milliseconds::rep;



    void duration_print_as_ns(duration dur);

    time_point now();
}