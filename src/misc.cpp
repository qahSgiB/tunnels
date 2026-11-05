#include "misc.hpp"


#include <fstream>
#include <sstream>



const std::unordered_map<std::string, glm::vec3> element_coloring{
    { "C", glm::vec3(0.5f, 0.5f, 0.5f) },
    { "H", glm::vec3(1.0f, 0.0f, 0.0f) },
    { "O", glm::vec3(0.9f, 0.9f, 0.9f) },
    { "N", glm::vec3(0.0f, 0.0f, 1.0f) },
};



// [todo] rename
glm::u32vec2 miscGlfwGetWindowSize(GLFWwindow* window)
{
    int width_i;
    int height_i;
    glfwGetFramebufferSize(window, &width_i, &height_i);
    return glm::u32vec2(static_cast<uint32_t>(width_i), static_cast<uint32_t>(height_i));
}

glm::vec2 miscGlfwGetCursorPos(GLFWwindow* window)
{
    double cx_d = -1.0;
    double cy_d = -1.0;
    glfwGetCursorPos(window, &cx_d, &cy_d);
    return glm::vec2(static_cast<float>(cx_d), static_cast<float>(cy_d));
}


std::pair<WGPUTexture, WGPUTextureView> wgpu_create_simple_depth_texture_with_view(WGPUDevice device, glm::u32vec2 size)
{
    WGPUExtent3D depth_texture_size = {
        .width = size.x,
        .height = size.y,
        .depthOrArrayLayers = 1,
    };

    WGPUTextureDescriptor depth_texture_desc = {
        .label = "depth texture",
        .usage = WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .size = depth_texture_size,
        .format = WGPUTextureFormat_Depth32Float,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = nullptr,
    };

    WGPUTexture depth_texture = wgpuDeviceCreateTexture(device, &depth_texture_desc);

    // depth texture - view
    WGPUTextureViewDescriptor depth_texture_view_desc = {
        .label = "depth texture view",
        .format = WGPUTextureFormat_Depth32Float,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_All,
    };

    WGPUTextureView depth_texture_view = wgpuTextureCreateView(depth_texture, &depth_texture_view_desc);

    return { depth_texture, depth_texture_view };
}

WGPUShaderModule wgpu_create_simple_wgsl_shader(WGPUDevice device, std::filesystem::path path, const char* label)
{
    label = (label == nullptr) ? "shader (no label)" : label;

    std::ifstream shader_file(path);
    shader_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    std::stringstream shader_source_stream;
    shader_source_stream << shader_file.rdbuf();
    std::string shader_source = shader_source_stream.str();

    WGPUShaderModuleWGSLDescriptor wgsl_shader_desc = {
        .chain = {
            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
        },
        .code = shader_source.c_str(),
    };

    WGPUShaderModuleDescriptor shader_desc = {
        .nextInChain = &wgsl_shader_desc.chain,
        .label = label,
    };

    return wgpuDeviceCreateShaderModule(device, &shader_desc);
}

WGPURenderPassEncoder wgpu_create_simple_render_pass(
    WGPUCommandEncoder encoder,
    WGPUTextureView swapchain_view,
    WGPUColor clear_color,
    const char* label
) {
    label = (label == nullptr) ? "render pass (no label)" : label;

    WGPURenderPassColorAttachment color_attachment = {
        .view = swapchain_view,
        .loadOp = WGPULoadOp_Load,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = clear_color,
    };

    const WGPURenderPassDescriptor render_pass_desc = {
        .label = label,
        .colorAttachmentCount = 1,
        .colorAttachments = &color_attachment,
        .depthStencilAttachment = nullptr,
    };

    return wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
}

WGPURenderPassEncoder wgpu_create_simple_render_pass_with_depth(
    WGPUCommandEncoder encoder,
    WGPUTextureView swapchain_view,
    WGPUTextureView depth_texture_view,
    WGPUColor clear_color,
    const char* label
) {
    label = (label == nullptr) ? "render pass (no label)" : label;

    WGPURenderPassColorAttachment color_attachment = {
        .view = swapchain_view,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = clear_color,
    };

    WGPURenderPassDepthStencilAttachment depth_stencil_attachment = {
        .view = depth_texture_view,
        .depthLoadOp = WGPULoadOp_Clear,
        .depthStoreOp = WGPUStoreOp_Store,
        .depthClearValue = 1.0,
        .depthReadOnly = false,
        .stencilLoadOp = WGPULoadOp_Undefined,
        .stencilStoreOp = WGPUStoreOp_Undefined,
        .stencilClearValue = 0,
        .stencilReadOnly = false,
    };

    const WGPURenderPassDescriptor render_pass_desc = {
        .label = label,
        .colorAttachmentCount = 1,
        .colorAttachments = &color_attachment,
        .depthStencilAttachment = &depth_stencil_attachment,
    };

    return wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
}