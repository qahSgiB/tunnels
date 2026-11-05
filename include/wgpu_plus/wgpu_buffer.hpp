#pragma once



#include "webgpu/webgpu.h"


#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <type_traits>
#include <stdexcept>



/**
 * wrapper around GPUBuffer 
 */
template<typename T, bool updatable = true>
class GPUBuffer
{
public:
    T data;

private:
    std::string label;
    std::string label_buffer;

    WGPUBuffer buffer;

    bool needs_update = false;


public:
    explicit GPUBuffer(const char* label = nullptr);
    GPUBuffer(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue = nullptr, const char* label = nullptr);

    ~GPUBuffer();

    // webgpu init
    void init(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue = nullptr, const char* label = nullptr);
    void init_labels(const char* label_ = nullptr);
    void init_buffer(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue = nullptr);

    // webgpu access
    size_t get_size();
    size_t get_size_bytes();
    const WGPUBuffer& get_buffer();

    // update flag manipulation
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void set_needs_update();
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void or_needs_update(bool needs_update_);

    // webgpu update
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void update_buffer(WGPUQueue queue);
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void update_buffer_if_needed(WGPUQueue queue);

    // webgpu release
    void release_buffer();
    void release();
};



template<typename T, bool updatable>
GPUBuffer<T, updatable>::GPUBuffer(const char* label)
{
    needs_update = false;
    init_labels(label);
}

template<typename T, bool updatable>
GPUBuffer<T, updatable>::GPUBuffer(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue, const char* label)
{
    init(device, usage, queue, label);
}

template<typename T, bool updatable>
GPUBuffer<T, updatable>::~GPUBuffer()
{
    release();
}

template<typename T, bool updatable>
void GPUBuffer<T, updatable>::init(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue, const char* label)
{
    needs_update = false;
    init_labels(label);
    init_buffer(device, usage, queue);
}

template<typename T, bool updatable>
void GPUBuffer<T, updatable>::init_labels(const char* label_)
{
    label = (label_ == nullptr) ? "(no label)" : label_;
    label_buffer = "buffer - GPUBuffer<> " + label;
}

template<typename T, bool updatable>
void GPUBuffer<T, updatable>::init_buffer(WGPUDevice device, WGPUBufferUsageFlags usage, WGPUQueue queue)
{
    if constexpr (updatable) {
        usage |= WGPUBufferUsage_CopyDst;
    }

    WGPUBufferDescriptor desc = {
        .label = label_buffer.c_str(),
        .usage = usage,
        .size = sizeof(T),
        .mappedAtCreation = !updatable,
    };

    buffer = wgpuDeviceCreateBuffer(device, &desc);

    if (queue != nullptr) {
        if constexpr (updatable) {
            update_buffer(queue);
        } else {
            void* buffer_data = wgpuBufferGetMappedRange(buffer, 0, sizeof(T));
            std::memcpy(buffer_data, data.data(), sizeof(T));
            wgpuBufferUnmap(buffer);
        }
    }
}

template<typename T, bool updatable>
size_t GPUBuffer<T, updatable>::get_size_bytes() { return sizeof(T); }

template<typename T, bool updatable>
const WGPUBuffer& GPUBuffer<T, updatable>::get_buffer() { return buffer; }

template<typename T, bool updatable>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUBuffer<T, updatable>::set_needs_update()
{
    needs_update = true;
}

template<typename T, bool updatable>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUBuffer<T, updatable>::or_needs_update(bool needs_update_)
{
    needs_update |= needs_update_;
}

template<typename T, bool updatable>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUBuffer<T, updatable>::update_buffer(WGPUQueue queue)
{
    wgpuQueueWriteBuffer(queue, buffer, 0, &data, sizeof(T));
}

template<typename T, bool updatable>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUBuffer<T, updatable>::update_buffer_if_needed(WGPUQueue queue)
{
    if (needs_update) {
        update_buffer(queue);
        needs_update = false;
    }
}

template<typename T, bool updatable>
void GPUBuffer<T, updatable>::release_buffer()
{
    wgpuBufferDestroy(buffer);
    wgpuBufferRelease(buffer);
    buffer = nullptr;
}
template<typename T, bool updatable>
void GPUBuffer<T, updatable>::release()
{
    if (buffer != nullptr) { release_buffer(); }
}
