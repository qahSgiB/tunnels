#include "platform/wgpu.hpp"


#include "platform/defs.hpp"

#ifdef PLATFORM_WEB
    #include "emscripten/html5_webgpu.h"
#else
    #include "dawn/native/DawnNative.h"
    #include "dawn/dawn_proc.h"
    #include "dawn/webgpu_cpp.h"
#endif

#include <vector>
#include <array>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cstring>


/** enabled backends sorted by preference (platform native backends are preferred) */
#ifdef PLATFORM_NATIVE
    #if defined(DAWN_ENABLE_BACKEND_D3D12)
        #define DEFAULT_BACKEND_TYPE WGPUBackendType_D3D12
    #elif defined(DAWN_ENABLE_BACKEND_METAL)
        #define DEFAULT_BACKEND_TYPE WGPUBackendType_Metal
    #elif defined(DAWN_ENABLE_BACKEND_VULKAN)
        #define DEFAULT_BACKEND_TYPE WGPUBackendType_Vulkan
    #elif defined(DAWN_ENABLE_BACKEND_OPENGLES)
        #define DEFAULT_BACKEND_TYPE WGPUBackendType_OpenGLES
    #elif defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
        #define DEFAULT_BACKEND_TYPE WGPUBackendType_OpenGL
    #endif
#endif

/** GLFW platform specific types */
#ifdef PLATFORM_WIN
    typedef void* GLFWNativeWindowHandle;
    typedef void* GLFWNativeDisplayHandle;
#endif
#ifdef PLATFORM_MAC
    typedef void* GLFWNativeWindowHandle;
    typedef void* GLFWNativeDisplayHandle;
#endif
#ifdef PLATFORM_UNIX
    typedef unsigned long GLFWNativeWindowHandle;
    typedef void* GLFWNativeDisplayHandle;
#endif
#ifdef PLATFORM_WEB
    typedef void* GLFWNativeWindowHandle;
    typedef void* GLFWNativeDisplayHandle;
#endif

#ifdef PLATFORM_WIN
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include "GLFW/glfw3native.h"
#endif
#ifdef PLATFORM_MAC
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include "GLFW/glfw3native.h"
#endif
#ifdef PLATFORM_UNIX
    #define GLFW_EXPOSE_NATIVE_X11
    #include "GLFW/glfw3native.h"
#endif



/**
 * staticly allocated webgpu instance
 * it's up to user to ensure that instance is initialized when calling functions which requiere webgpu instance
 * (no checking whether wgpu_instance has value is done in these functions)
 */
// [todo] class/struct
#ifdef PLATFORM_WEB
static std::optional<WGPUInstance> wgpu_instance = std::nullopt;
#else
/** dan::native::Instance enables enumerating adapters (WGPUInstance can only provide one adapter) */
static std::optional<dawn::native::Instance> wgpu_instance = std::nullopt;
#endif



void wgpu_init_instance()
{
#ifdef PLATFORM_WEB
    /* it loks like currently WGPUInstance in emscripten is just empty object without any function */
    WGPUInstanceDescriptor instance_descriptor = {};
    wgpu_instance = wgpuCreateInstance(&instance_descriptor);
#else
    wgpu_instance.emplace();
    
    wgpu_instance->SetBackendValidationLevel(dawn::native::BackendValidationLevel::Full);
    wgpu_instance->EnableBackendValidation(true);

    const DawnProcTable dawn_proc_table(dawn::native::GetProcs());
    dawnProcSetProcs(&dawn_proc_table);
#endif
}

void wgpu_release_instance()
{
#ifdef PLATFORM_WEB
    wgpuInstanceRelease(*wgpu_instance);
    wgpu_instance.reset();
#else
    wgpu_instance.reset();
#endif
}

// static bool wgpu_instance_initialized() { return wgpu_instance.has_value(); }

// static void wgpu_instance_initialized_or_error()
// {
//     if (!wgpu_instance.has_value()) {
//         throw std::runtime_error("WGPUInstance not initialized"); // [todo] better message
//     }
// }

WGPUInstance wgpu_get_instance()
{
#ifdef PLATFORM_WEB
    return *wgpu_instance;
#else
    return wgpu_instance->Get();
#endif
}

/**
 * on native user needs to manually call wgpuInstanceProcessEvents to process internal events in dawn (errors, buffer mapping, ...)
 * in browser is event processing asynchronous (probably?)
 */
void wgpu_instance_process_events()
{
    #ifndef __EMSCRIPTEN__
    wgpuInstanceProcessEvents(wgpu_instance->Get());
    #endif
}

#ifdef PLATFORM_NATIVE
static const char* wgpu_backend_type_str(WGPUBackendType backend_type)
{
    switch (backend_type) {
    case WGPUBackendType_Undefined: return "Undefined";
    case WGPUBackendType_Null:      return "Null";
    case WGPUBackendType_WebGPU:    return "WebGPU";
    case WGPUBackendType_D3D11:     return "D3D11";
    case WGPUBackendType_D3D12:     return "D3D12";
    case WGPUBackendType_Metal:     return "Metal";
    case WGPUBackendType_Vulkan:    return "Vulkan";
    case WGPUBackendType_OpenGL:    return "OpenGL";
    case WGPUBackendType_OpenGLES:  return "OpenGL ES";
    default:                           return "";
    }
}

static const char* wgpu_adapter_type_str(WGPUAdapterType adapter_type)
{
    switch (adapter_type) {
    case WGPUAdapterType_DiscreteGPU:   return "Discrete GPU";
    case WGPUAdapterType_IntegratedGPU: return "Integrated GPU";
    case WGPUAdapterType_CPU:           return "CPU";
    case WGPUAdapterType_Unknown:       return "Unknown";
    default:                               return "";
    }
}

static void wgpu_log_adapter(const dawn::native::Adapter& adapter, const WGPUAdapterProperties* selected_adapter_props) {
    WGPUAdapterProperties props;
    props.nextInChain = nullptr;
    adapter.GetProperties(&props);

    bool decor = selected_adapter_props != nullptr && 
                 (std::strcmp(selected_adapter_props->name, props.name) == 0) &&
                 selected_adapter_props->backendType == props.backendType;

    if (decor) { std::cout << "  >>>  "; }
    std::cout << props.name << " (" << props.driverDescription << ")\n";
    if (decor) { std::cout << "  >>>  "; }
    std::cout << "  vendor : " << props.vendorName << "\n";
    if (decor) { std::cout << "  >>>  "; }
    std::cout << "  backend type : " << wgpu_backend_type_str(props.backendType) << "\n";
    if (decor) { std::cout << "  >>>  "; }
    std::cout << "  adapter type : " << wgpu_adapter_type_str(props.adapterType) << "\n";
    std::cout << "\n";
}
#endif

#ifdef PLATFORM_WEB
void wgpu_log_adapters([[maybe_unused]] WGPUDevice device) {}
#else
void wgpu_log_adapters(WGPUDevice device)
{
    std::optional<WGPUAdapterProperties> selected_adapter_props = std::nullopt;
    if (device != nullptr) {
        WGPUAdapter selected_adapter = wgpuDeviceGetAdapter(device);
        selected_adapter_props.emplace();
        selected_adapter_props->nextInChain = nullptr;
        wgpuAdapterGetProperties(selected_adapter, &(*selected_adapter_props));
    }

    const WGPUAdapterProperties* selected_adapter_props_pointer = selected_adapter_props.has_value() ? &(*selected_adapter_props) : nullptr;

    std::ranges::for_each(
        wgpu_instance->EnumerateAdapters(),
        [selected_adapter_props_pointer](auto adapter) { wgpu_log_adapter(adapter, selected_adapter_props_pointer); }
    );

    if (selected_adapter_props.has_value()) {
        wgpuAdapterPropertiesFreeMembers(*selected_adapter_props);
    }
}
#endif

#ifdef PLATFORM_WEB
WGPUDevice wgpu_create_device([[maybe_unused]] std::optional<WGPUBackendType> backend_type)
{
    return emscripten_webgpu_get_device();
}
#else
WGPUDevice wgpu_create_device(std::optional<WGPUBackendType> backend_type)
{
    WGPUBackendType best_backend_type = backend_type.value_or(DEFAULT_BACKEND_TYPE);

    std::vector<dawn::native::Adapter> adapters = wgpu_instance->EnumerateAdapters();

    /** returns *first* greatest element (by standard) */
    auto adapter_iter = std::ranges::max_element(adapters, {}, [best_backend_type](const dawn::native::Adapter& adapter) {
        WGPUAdapterProperties props;
        props.nextInChain = nullptr;
        adapter.GetProperties(&props);

        uint8_t score = (props.backendType == best_backend_type) ? 10 : 0;

        switch (props.adapterType) {
        case WGPUAdapterType_DiscreteGPU:
            score += 3;
            break;
        case WGPUAdapterType_IntegratedGPU:
            score += 2;
            break;
        case WGPUAdapterType_CPU:
            score += 1;
            break;
        default:
            break;
        }

        wgpuAdapterPropertiesFreeMembers(props);

        return score;
    });
    if (adapter_iter == adapters.cend()) {
        return nullptr;
    }

    dawn::native::Adapter& adapter = *adapter_iter;

    WGPUDevice device = adapter.CreateDevice();

    return device;
}
#endif

#ifdef PLATFORM_NATIVE
static GLFWNativeWindowHandle glfw_native_window_handle(GLFWwindow* window)
{
#ifdef PLATFORM_WIN
    return glfwGetWin32Window(window);
#endif
#ifdef PLATFORM_MAC
    return glfwGetCocoaWindow(window);
#endif
#ifdef PLATFORM_UNIX
    return glfwGetX11Window(window);
#endif
}

static GLFWNativeDisplayHandle glfw_native_display_handle()
{
#ifdef PLATFORM_WIN
    return GetModuleHandle(nullptr);
#endif
#ifdef PLATFORM_MAC
    return nullptr;
#endif
#ifdef PLATFORM_UNIX
    return glfwGetX11Display();
#endif
}
#endif

#ifdef PLATFORM_WEB
WGPUSurface wgpu_create_surface([[maybe_unused]] GLFWwindow* window)
{
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas_descriptor = {};
    canvas_descriptor.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    canvas_descriptor.selector = "canvas";

    WGPUSurfaceDescriptor surface_descriptor = {};
    surface_descriptor.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&canvas_descriptor);

    return wgpuInstanceCreateSurface(*wgpu_instance, &surface_descriptor);
}
#else
WGPUSurface wgpu_create_surface(GLFWwindow* window)
{
#ifdef PLATFORM_WIN
    wgpu::SurfaceDescriptorFromWindowsHWND hwnd_descriptor;
    hwnd_descriptor.hwnd = glfw_native_window_handle(window);
    hwnd_descriptor.hinstance = glfw_native_display_handle();
    
    WGPUSurfaceDescriptor surface_descriptor;
    surface_descriptor.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&hwnd_descriptor);

    return wgpuInstanceCreateSurface(wgpu_instance->Get(), &surface_descriptor);
#endif
#ifdef PLATFORM_UNIX
    wgpu::SurfaceDescriptorFromXlibWindow x11_descriptor;
    x11_descriptor.window = glfw_native_window_handle(window);
    x11_descriptor.display = glfw_native_display_handle();

    WGPUSurfaceDescriptor surface_descriptor = {};
    surface_descriptor.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&x11_descriptor);

    return wgpuInstanceCreateSurface(wgpu_instance->Get(), &surface_descriptor);
#endif
}
#endif