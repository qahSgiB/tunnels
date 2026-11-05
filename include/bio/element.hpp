#pragma once


#include <string>
#include <vector>
#include <unordered_map>
#include <functional>



struct Element
{
    size_t number;
    std::string symbol;
    std::string name;

    float radius; /* van der walls radius */


    static Element create(size_t number, std::string symbol, std::string name, float radius);


    static void world_init();

    // [todo] seperate world class maybe
    static std::vector<Element> world;
    static std::unordered_map<std::string, std::reference_wrapper<const Element>> world_symbol_to_element;
};