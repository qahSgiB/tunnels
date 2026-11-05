#pragma once


#include "GLFW/glfw3.h"

#ifdef __EMSCRIPTEN__
#include "emscripten/html5.h"
#endif


#include <concepts>



template<typename T>
concept CLoopableApp = requires(T a)
{
    { a.loop() } -> std::same_as<bool>;
    { a.destroy() } -> std::same_as<void>;
};



#ifdef __EMSCRIPTEN__
template<CLoopableApp T>
EM_BOOL emscripten_loop([[maybe_unused]] double time, void* data) {
    T& app = *(static_cast<T*>(data));

    bool continue_redner = app.loop();

    if (!continue_redner) {
        // [todo] destruction?
        app.destroy();
        // app.~TunnelsApplication();
    }

    return continue_redner;
}
#endif



// [todo] add concept for T
// [todo] better name
template <CLoopableApp T>
void emscripten_glfw_loop([[maybe_unused]] GLFWwindow* window, T& app)
{
    #ifdef __EMSCRIPTEN__
    emscripten_request_animation_frame_loop(emscripten_loop<T>, &app);
    #else

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (!app.loop()) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    app.destroy();
    #endif
}