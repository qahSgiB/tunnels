#include "bio/element.hpp"


#include <ranges>
#include <unordered_map>
#include <utility>



Element Element::create(size_t number, std::string symbol, std::string name, float radius)
{
    return Element {
        .number = number,
        .symbol = std::move(symbol),
        .name = std::move(name),
        .radius = radius
    };
}


void Element::world_init()
{
    auto rng = world | std::views::transform([] (const Element& element) { return std::make_pair(element.symbol, std::cref(element));});

    world_symbol_to_element.clear();
    world_symbol_to_element.insert(rng.begin(), rng.end());
}


std::vector<Element> Element::world{
    Element::create(  1, "H" , "Hydrogen"     , 1.200f),
    Element::create(  2, "He", "Helium"       , 1.400f),
    Element::create(  3, "Li", "Lithium"      , 1.820f),
    Element::create(  4, "Be", "Beryllium"    , 1.530f),
    Element::create(  5, "B" , "Boron"        , 1.920f),
    Element::create(  6, "C" , "Carbon"       , 1.700f),
    Element::create(  7, "N" , "Nitrogen"     , 1.550f),
    Element::create(  8, "O" , "Oxygen"       , 1.520f),
    Element::create(  9, "F" , "Fluorine"     , 1.470f),
    Element::create( 10, "Ne", "Neon"         , 1.540f),
    Element::create( 11, "Na", "Sodium"       , 2.270f),
    Element::create( 12, "Mg", "Magnesium"    , 1.730f),
    Element::create( 13, "Al", "Aluminium"    , 1.840f),
    Element::create( 14, "Si", "Silicon"      , 2.100f),
    Element::create( 15, "P" , "Phosphorus"   , 1.800f),
    Element::create( 16, "S" , "Sulfur"       , 1.800f),
    Element::create( 17, "Cl", "Chlorine"     , 1.750f),
    Element::create( 18, "Ar", "Argon"        , 1.880f),
    Element::create( 19, "K" , "Potassium"    , 2.750f),
    Element::create( 20, "Ca", "Calcium"      , 2.310f),
    Element::create( 21, "Sc", "Scandium"     , 2.110f),
    Element::create( 22, "Ti", "Titanium"     , 2.000f),
    Element::create( 23, "V" , "Vanadium"     , 2.000f),
    Element::create( 24, "Cr", "Chromium"     , 2.000f),
    Element::create( 25, "Mn", "Manganese"    , 2.000f),
    Element::create( 26, "Fe", "Iron"         , 2.000f),
    Element::create( 27, "Co", "Cobalt"       , 2.000f),
    Element::create( 28, "Ni", "Nickel"       , 1.630f),
    Element::create( 29, "Cu", "Copper"       , 1.400f),
    Element::create( 30, "Zn", "Zinc"         , 1.390f),
    Element::create( 31, "Ga", "Gallium"      , 1.870f),
    Element::create( 32, "Ge", "Germanium"    , 2.110f),
    Element::create( 33, "As", "Arsenic"      , 1.850f),
    Element::create( 34, "Se", "Selenium"     , 1.900f),
    Element::create( 35, "Br", "Bromine"      , 1.850f),
    Element::create( 36, "Kr", "Krypton"      , 2.020f),
    Element::create( 37, "Rb", "Rubidium"     , 3.030f),
    Element::create( 38, "Sr", "Strontium"    , 2.490f),
    Element::create( 39, "Y" , "Yttrium"      , 2.000f),
    Element::create( 40, "Zr", "Zirconium"    , 2.000f),
    Element::create( 41, "Nb", "Niobium"      , 2.000f),
    Element::create( 42, "Mo", "Molybdenum"   , 2.000f),
    Element::create( 43, "Tc", "Technetium"   , 2.000f),
    Element::create( 44, "Ru", "Ruthenium"    , 2.000f),
    Element::create( 45, "Rh", "Rhodium"      , 2.000f),
    Element::create( 46, "Pd", "Palladium"    , 1.630f),
    Element::create( 47, "Ag", "Silver"       , 1.720f),
    Element::create( 48, "Cd", "Cadmium"      , 1.580f),
    Element::create( 49, "In", "Indium"       , 1.930f),
    Element::create( 50, "Sn", "Tin"          , 2.170f),
    Element::create( 51, "Sb", "Antimony"     , 2.060f),
    Element::create( 52, "Te", "Tellurium"    , 2.060f),
    Element::create( 53, "I" , "Iodine"       , 1.980f),
    Element::create( 54, "Xe", "Xenon"        , 2.160f),
    Element::create( 55, "Cs", "Cesium"       , 3.430f),
    Element::create( 56, "Ba", "Barium"       , 2.680f),
    Element::create( 57, "La", "Lanthanum"    , 2.000f),
    Element::create( 58, "Ce", "Cerium"       , 2.000f),
    Element::create( 59, "Pr", "Praseodymium" , 2.000f),
    Element::create( 60, "Nd", "Neodymium"    , 2.000f),
    Element::create( 61, "Pm", "Promethium"   , 2.000f),
    Element::create( 62, "Sm", "Samarium"     , 2.000f),
    Element::create( 63, "Eu", "Europium"     , 2.000f),
    Element::create( 64, "Gd", "Gadolinium"   , 2.000f),
    Element::create( 65, "Tb", "Terbium"      , 2.000f),
    Element::create( 66, "Dy", "Dysprosium"   , 2.000f),
    Element::create( 67, "Ho", "Holmium"      , 2.000f),
    Element::create( 68, "Er", "Erbium"       , 2.000f),
    Element::create( 69, "Tm", "Thulium"      , 2.000f),
    Element::create( 70, "Yb", "Ytterbium"    , 2.000f),
    Element::create( 71, "Lu", "Lutetium"     , 2.000f),
    Element::create( 72, "Hf", "Hafnium"      , 2.000f),
    Element::create( 73, "Ta", "Tantalum"     , 2.000f),
    Element::create( 74, "W" , "Tungsten"     , 2.000f),
    Element::create( 75, "Re", "Rhenium"      , 2.000f),
    Element::create( 76, "Os", "Osmium"       , 2.000f),
    Element::create( 77, "Ir", "Iridium"      , 2.000f),
    Element::create( 78, "Pt", "Platinum"     , 1.750f),
    Element::create( 79, "Au", "Gold"         , 1.660f),
    Element::create( 80, "Hg", "Mercury"      , 1.550f),
    Element::create( 81, "Tl", "Thallium"     , 1.960f),
    Element::create( 82, "Pb", "Lead"         , 2.020f),
    Element::create( 83, "Bi", "Bismuth"      , 2.070f),
    Element::create( 84, "Po", "Polonium"     , 1.970f),
    Element::create( 85, "At", "Astatine"     , 2.020f),
    Element::create( 86, "Rn", "Radon"        , 2.200f),
    Element::create( 87, "Fr", "Francium"     , 3.480f),
    Element::create( 88, "Ra", "Radium"       , 2.830f),
    Element::create( 89, "Ac", "Actinium"     , 2.000f),
    Element::create( 90, "Th", "Thorium"      , 2.000f),
    Element::create( 91, "Pa", "Protactinium" , 2.000f),
    Element::create( 92, "U" , "Uranium"      , 1.860f),
    Element::create( 93, "Np", "Neptunium"    , 2.000f),
    Element::create( 94, "Pu", "Plutonium"    , 2.000f),
    Element::create( 95, "Am", "Americium"    , 2.000f),
    Element::create( 96, "Cm", "Curium"       , 2.000f),
    Element::create( 97, "Bk", "Berkelium"    , 2.000f),
    Element::create( 98, "Cf", "Californium"  , 2.000f),
    Element::create( 99, "Es", "Einsteinium"  , 2.000f),
    Element::create(100, "Fm", "Fermium"      , 2.000f),
    Element::create(101, "Md", "Mendelevium"  , 2.000f),
    Element::create(102, "No", "Nobelium"     , 2.000f),
    Element::create(103, "Lr", "Lawrencium"   , 2.000f),
    Element::create(104, "Rf", "Rutherfordium", 2.000f),
    Element::create(105, "Db", "Dubnium"      , 2.000f),
    Element::create(106, "Sg", "Seaborgium"   , 2.000f),
    Element::create(107, "Bh", "Bohrium"      , 2.000f),
    Element::create(108, "Hs", "Hassium"      , 2.000f),
    Element::create(109, "Mt", "Meitnerium"   , 2.000f),
    Element::create(110, "Ds", "Darmstadtium" , 2.000f),
    Element::create(111, "Rg", "Roentgenium"  , 2.000f),
    Element::create(112, "Cn", "Copernicium"  , 2.000f),
    // Element::create(113, "H (WAT)", "Hydrogen (Water)", 0.0f),
    // Element::create(114, "O (WAT)", "Oxygen (Water)", 0.0f),
    // Element::create(115, "D", "Hydrogen", 0.0f),
};

std::unordered_map<std::string, std::reference_wrapper<const Element>> Element::world_symbol_to_element{};