#pragma once


#include "wgpu_plus/wgpu_binding.hpp"


#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "GLFW/glfw3.h"

#include "webgpu/webgpu.h"


#include <string>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <span>
#include <filesystem>



using FloatGPU = GPUBinding<float>;



const extern std::unordered_map<std::string, glm::vec3> element_coloring;



glm::u32vec2 miscGlfwGetWindowSize(GLFWwindow* window);

glm::vec2 miscGlfwGetCursorPos(GLFWwindow* window);


std::pair<WGPUTexture, WGPUTextureView> wgpu_create_simple_depth_texture_with_view(WGPUDevice device, glm::u32vec2 size);

WGPUShaderModule wgpu_create_simple_wgsl_shader(WGPUDevice device, std::filesystem::path path, const char* label = nullptr);

template<size_t E1, size_t E2>
std::tuple<WGPUShaderModule, WGPUPipelineLayout, WGPURenderPipeline> wgpu_create_simple_pipeline(
    WGPUDevice device,
    std::filesystem::path shader_path,
    std::span<WGPUBindGroupLayout, E1> bind_group_layouts,
    std::span<WGPUVertexBufferLayout, E2> vertex_buffer_layouts,
    WGPUPrimitiveTopology primitive_topology,
    const char* shader_label = nullptr,
    const char* pipeline_layout_label = nullptr,
    const char* pipeline_label = nullptr
);

WGPURenderPassEncoder wgpu_create_simple_render_pass(
    WGPUCommandEncoder encoder,
    WGPUTextureView swapchain_view,
    WGPUColor clear_color,
    const char* label = nullptr
);

WGPURenderPassEncoder wgpu_create_simple_render_pass_with_depth(
    WGPUCommandEncoder encoder,
    WGPUTextureView swapchain_view,
    WGPUTextureView depth_texture_view,
    WGPUColor clear_color,
    const char* label = nullptr
);



template<size_t E1, size_t E2>
std::tuple<WGPUShaderModule, WGPUPipelineLayout, WGPURenderPipeline> wgpu_create_simple_pipeline(
    WGPUDevice device,
    std::filesystem::path shader_path,
    std::span<WGPUBindGroupLayout, E1> bind_group_layouts,
    std::span<WGPUVertexBufferLayout, E2> vertex_buffer_layouts,
    WGPUPrimitiveTopology primitive_topology,
    const char* shader_label,
    const char* pipeline_layout_label,
    const char* pipeline_label
) {
    pipeline_layout_label = (pipeline_layout_label == nullptr) ? "pipeline layout (no label)" : pipeline_layout_label;
    pipeline_label = (pipeline_label == nullptr) ? "pipeline (no label)" : pipeline_label;

    WGPUShaderModule shader = wgpu_create_simple_wgsl_shader(device, shader_path, shader_label);

    WGPUPipelineLayoutDescriptor layout_desc = {
        .label = pipeline_layout_label,
        .bindGroupLayoutCount = bind_group_layouts.size(),
        .bindGroupLayouts = bind_group_layouts.data(),
    };

    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layout_desc);

    WGPUVertexState vertex_state = {
        .module = shader,
        .entryPoint = "vs_main",
        .constantCount = 0,
        .constants = nullptr,
        .bufferCount = static_cast<uint32_t>(vertex_buffer_layouts.size()),
        .buffers = vertex_buffer_layouts.data(),
    };

    WGPUPrimitiveState primitive_state = {
        .topology = primitive_topology,
        .stripIndexFormat = WGPUIndexFormat_Undefined,
        .frontFace = WGPUFrontFace_CCW,
        .cullMode = WGPUCullMode_None,
    };

    WGPUDepthStencilState depth_stencil_state = {
        .format = WGPUTextureFormat_Depth32Float,
        .depthWriteEnabled = true,
        .depthCompare = WGPUCompareFunction_Less,
        .stencilFront = {
            .compare = WGPUCompareFunction_Always,
            .failOp = WGPUStencilOperation_Keep,
            .depthFailOp = WGPUStencilOperation_Keep,
            .passOp = WGPUStencilOperation_Keep,
        },
        .stencilBack = {
            .compare = WGPUCompareFunction_Always,
            .failOp = WGPUStencilOperation_Keep,
            .depthFailOp = WGPUStencilOperation_Keep,
            .passOp = WGPUStencilOperation_Keep,
        },
        .stencilReadMask = 0,
        .stencilWriteMask = 0,
        .depthBias = 0,
        .depthBiasSlopeScale = 0.0,
        .depthBiasClamp = 0.0,
    };

    WGPUMultisampleState multisample_state = {
        .count = 1,
        .mask = ~((uint32_t) 0),
        .alphaToCoverageEnabled = false,
    };

    WGPUBlendState blend_state = {
        .color = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_One,
            .dstFactor = WGPUBlendFactor_Zero,
        },
        .alpha = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_One,
            .dstFactor = WGPUBlendFactor_Zero,
        },
    };

    WGPUColorTargetState color_target_state = {
        .format = WGPUTextureFormat_BGRA8Unorm,
        .blend = &blend_state,
        .writeMask = WGPUColorWriteMask_All,
    };

    WGPUFragmentState fragment_state = {
        .module = shader,
        .entryPoint = "fs_main",
        .constantCount = 0,
        .constants = nullptr,
        .targetCount = 1,
        .targets = &color_target_state,
    };

    WGPURenderPipelineDescriptor pipeline_desc = {
        .label = pipeline_label,
        .layout = layout,
        .vertex = vertex_state,
        .primitive = primitive_state,
        .depthStencil = &depth_stencil_state,
        .multisample = multisample_state,
        .fragment = &fragment_state,
    };

    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeline_desc);

    return { shader, layout, pipeline };
}