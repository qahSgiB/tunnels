#pragma once


#include "wgpu_plus/wgpu_buffer.hpp"


#include "webgpu/webgpu.h"


#include <string>



using namespace std::string_literals;



/**
 * wrapper around GPUBindGroup and GPUBuffer coresponding to bind group + can create WGPUBindGroupLayout
 */
template<typename T>
class GPUBinding
{
private:
    std::string label_bind_group;

    GPUBuffer<T, true> buffer;
    WGPUBindGroup bind_group = nullptr;


public:
    static const WGPUShaderStageFlags DEFAULT_VISIBILITY;


public:
    explicit GPUBinding(const char* label = nullptr);
    GPUBinding(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUQueue queue = nullptr, const char* label = nullptr);

    ~GPUBinding();

    /* webgpu init */
    void init(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUQueue queue = nullptr, const char* label = nullptr);
    void init_labels(const char* label = nullptr);
    void init_buffer(WGPUDevice device, WGPUQueue queue = nullptr);
    void init_bind_group(WGPUDevice device, WGPUBindGroupLayout bind_group_layout);

    static WGPUBindGroupLayout create_bind_group_layout(WGPUDevice device, const char* label, WGPUShaderStageFlags visibility = DEFAULT_VISIBILITY);

    /* webgpu access */
    GPUBuffer<T, true>& get_buffer();
    const WGPUBindGroup& get_bind_group();

    /* bind */
    void bind(WGPURenderPassEncoder pass, uint32_t group_index);

    /* buffer manipulation */
    T& data();
    const T& data() const;

    void set_needs_update();
    void or_needs_update(bool needs_update);
    void update_buffer(WGPUQueue queue);
    void update_buffer_if_needed(WGPUQueue queue);

    /* webgpu release */
    void release_buffer();
    void release_bind_group();
    void release();
};



template<typename T>
const WGPUShaderStageFlags GPUBinding<T>::DEFAULT_VISIBILITY = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;


template<typename T>
GPUBinding<T>::GPUBinding(const char* label)
{
    init_labels(label);
}

template<typename T>
GPUBinding<T>::GPUBinding(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUQueue queue, const char* label)
{
    init(device, bind_group_layout, queue, label);
}

// template<typename T>
// GPUBinding<T>::GPUBinding(glm::mat4 projection, glm::mat4 view, glm::vec3 pos, WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUQueue queue, const char* label_) :
//     data(projection, view, pos)
// {
//     needs_update = false;

//     init_labels(label_);
//     init_buffer(device, queue);
//     init_bind_group(device, bind_group_layout);
// }

template<typename T>
GPUBinding<T>::~GPUBinding()
{
    release();
}

template<typename T>
void GPUBinding<T>::init(WGPUDevice device, WGPUBindGroupLayout bind_group_layout, WGPUQueue queue, const char* label)
{
    init_labels(label);
    init_buffer(device, queue);
    init_bind_group(device, bind_group_layout);
}

template<typename T>
void GPUBinding<T>::init_labels(const char* label)
{
    label = (label == nullptr) ? "(no label)" : label;
    buffer.init_labels(label);
    label_bind_group = "bind group - GPUBinding<> "s + label;
}

template<typename T>
void GPUBinding<T>::init_buffer(WGPUDevice device, WGPUQueue queue)
{
    buffer.init_buffer(device, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform, queue);
}

template<typename T>
void GPUBinding<T>::init_bind_group(WGPUDevice device, WGPUBindGroupLayout bind_group_layout)
{
    const WGPUBindGroupEntry bind_group_entries[1] = {
        {
            .binding = 0,
            .buffer = buffer.get_buffer(),
            .offset = 0,
            .size = sizeof(T),
            .sampler = nullptr,
            .textureView = nullptr,
        }
    };

    const WGPUBindGroupDescriptor bind_group_desc = {
        .label = label_bind_group.c_str(),
        .layout = bind_group_layout,
        .entryCount = 1,
        .entries = bind_group_entries,
    };

    bind_group = wgpuDeviceCreateBindGroup(device, &bind_group_desc);
}

template<typename T>
WGPUBindGroupLayout GPUBinding<T>::create_bind_group_layout(WGPUDevice device, const char* label, WGPUShaderStageFlags visibility)
{
    WGPUBindGroupLayoutEntry bind_group_layout_entries[1] = {
        {
            .binding = 0,
            .visibility = visibility,
            .buffer = {
                .type = WGPUBufferBindingType_Uniform,
                .hasDynamicOffset = false,
                .minBindingSize = sizeof(T),
            },
        }
    };

    const WGPUBindGroupLayoutDescriptor bind_group_layout_desc = {
        .label = label,
        .entryCount = 1,
        .entries = bind_group_layout_entries,
    };

    return wgpuDeviceCreateBindGroupLayout(device, &bind_group_layout_desc);
}

template<typename T>
GPUBuffer<T, true>& GPUBinding<T>::get_buffer() { return buffer; }

template<typename T>
const WGPUBindGroup& GPUBinding<T>::get_bind_group() { return bind_group; }

template<typename T>
void GPUBinding<T>::bind(WGPURenderPassEncoder pass, uint32_t group_index)
{
    wgpuRenderPassEncoderSetBindGroup(pass, group_index, bind_group, 0, nullptr);
}

template<typename T>
T& GPUBinding<T>::data() { return buffer.data; }

template<typename T>
const T& GPUBinding<T>::data() const { return buffer.data; }

template<typename T>
void GPUBinding<T>::set_needs_update() { buffer.set_needs_update(); }
template<typename T>
void GPUBinding<T>::or_needs_update(bool needs_update) { buffer.or_needs_update(needs_update); }
template<typename T>
void GPUBinding<T>::update_buffer(WGPUQueue queue) { buffer.update_buffer(queue); }
template<typename T>
void GPUBinding<T>::update_buffer_if_needed(WGPUQueue queue) { buffer.update_buffer_if_needed(queue); }

template<typename T>
void GPUBinding<T>::release_bind_group()
{
    wgpuBindGroupRelease(bind_group);
    bind_group = nullptr;
}

template<typename T>
void GPUBinding<T>::release()
{
    buffer.release();
    if (bind_group != nullptr) { release_bind_group(); }
}