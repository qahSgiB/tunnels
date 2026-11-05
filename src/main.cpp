#include "app.hpp"


#include <iostream>



/**
 * app needs to be global because when using emscripten app.loop() is async so app cannot be destroyed at the end of __main__
 * emscripten doesn't call destructors of global variables when main exits, therefore even though main exits quickly (because app.loop is async), app isn't destoyed
 */
// [todo] maybe use dynamic allocation using new + delete and call delete in emscripten_render ...
TunnelsApplication app;




extern "C" int __main__([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    // [todo] exception handling
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT\n    what() :  " << e.what() << '\n';
    }

    return 0;
}