#include "algorithm/debug_utils.hpp"


#include <iostream>
#include <iomanip>



void adt::duration_print_as_ns(duration dur)
{
    milli_int ns = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
    std::cout << (ns / 1000) << "." << std::setw(3) << std::setfill('0') << (ns % 1000) << " ms";
}

adt::time_point adt::now()
{
    return clock::now();
}