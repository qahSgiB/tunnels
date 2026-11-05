#pragma once


#include "GLFW/glfw3.h"

#include "webgpu/webgpu.h"

#include "optional"



void wgpu_init_instance();
void wgpu_release_instance();

WGPUInstance wgpu_get_instance();


void wgpu_instance_process_events();


/**
 * logs available webgpu adapters on native, does nothing on web
 * if device is provided highlights adapter from which was device created
 * */
void wgpu_log_adapters(WGPUDevice device = nullptr);

WGPUDevice wgpu_create_device(std::optional<WGPUBackendType> backend_type = std::nullopt);

WGPUSurface wgpu_create_surface(GLFWwindow* window);