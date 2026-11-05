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
// [todo] make possible to change size of data during usage
template<typename T, bool updatable = true, bool is_vertex_buffer = false>
class GPUVectorBuffer
{
public:
    std::vector<T> data;

private:
    std::string label;
    std::string label_buffer;

    size_t buffer_size;
    WGPUBuffer buffer;

    bool needs_update = false;


public:
    explicit GPUVectorBuffer(const char* label_ = nullptr);
    GPUVectorBuffer(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_ = 0, WGPUQueue queue = nullptr, const char* label_ = nullptr);

    ~GPUVectorBuffer();

    /* webgpu init */
    void init(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_ = 0, WGPUQueue queue = nullptr, const char* label_ = nullptr);
    void init_labels(const char* label_ = nullptr);
    void init_buffer(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_ = 0, WGPUQueue queue = nullptr);

    /* validation */
    bool size_valid();
    void size_valid_else_throw();

    /* webgpu access */
    size_t get_size();
    size_t get_size_bytes();
    const WGPUBuffer& get_buffer();

    /* vertex buffer */
    template<bool v = is_vertex_buffer, std::enable_if_t<v && v == is_vertex_buffer, void*> = nullptr> void vertex_buffer(WGPURenderPassEncoder pass, uint32_t slot);

    /* manipulation */
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void set_needs_update();
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void or_needs_update(bool needs_update_);

    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void update_buffer(WGPUQueue queue);
    template<bool b = updatable, std::enable_if_t<b && b == updatable, void*> = nullptr> void update_buffer_if_needed(WGPUQueue queue);

    /* webgpu release */
    void release_buffer();
    void release();
};



template<typename T, bool updatable, bool is_vertex_buffer>
GPUVectorBuffer<T, updatable, is_vertex_buffer>::GPUVectorBuffer(const char* label_)
{
    needs_update = false;
    init_labels(label_);
}

template<typename T, bool updatable, bool is_vertex_buffer>
GPUVectorBuffer<T, updatable, is_vertex_buffer>::GPUVectorBuffer(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_, WGPUQueue queue, const char* label_)
{
    init(device, usage, buffer_size_, queue, label_);
}

template<typename T, bool updatable, bool is_vertex_buffer>
GPUVectorBuffer<T, updatable, is_vertex_buffer>::~GPUVectorBuffer()
{
    release();
}

template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::init(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_, WGPUQueue queue, const char* label_)
{
    needs_update = false;
    init_labels(label_);
    init_buffer(device, usage, buffer_size_, queue);
}

template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::init_labels(const char* label_)
{
    label = (label_ == nullptr) ? "(no label)" : label_;
    label_buffer = "buffer - GPUVectorBuffer<> " + label;
}

template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::init_buffer(WGPUDevice device, WGPUBufferUsageFlags usage, size_t buffer_size_, WGPUQueue queue)
{
    if constexpr (updatable) {
        usage |= WGPUBufferUsage_CopyDst;
    }

    if constexpr (is_vertex_buffer) {
        usage |= WGPUBufferUsage_Vertex;
    }

    buffer_size = (buffer_size_ == 0) ? data.size() : buffer_size_;

    WGPUBufferDescriptor desc = {
        .label = label_buffer.c_str(),
        .usage = usage,
        .size = buffer_size * sizeof(T),
        .mappedAtCreation = !updatable,
    };

    buffer = wgpuDeviceCreateBuffer(device, &desc);

    if (queue != nullptr) {
        if constexpr (updatable) {
            update_buffer(queue);
        } else {
            size_valid_else_throw();

            void* buffer_data = wgpuBufferGetMappedRange(buffer, 0, buffer_size * sizeof(T));
            std::memcpy(buffer_data, data.data(), buffer_size * sizeof(T));
            wgpuBufferUnmap(buffer);
        }
    }
}

template<typename T, bool updatable, bool is_vertex_buffer>
bool GPUVectorBuffer<T, updatable, is_vertex_buffer>::size_valid()
{
    return data.size() == buffer_size;
}

template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::size_valid_else_throw()
{
    if (!size_valid()) {
        throw std::runtime_error("wgpu_plus VALIDATION ERROR - GPUVectorBuffer<> buffer size and vector size does not match (" + label + ")");
    }
}

template<typename T, bool updatable, bool is_vertex_buffer>
size_t GPUVectorBuffer<T, updatable, is_vertex_buffer>::get_size() { return buffer_size; }

template<typename T, bool updatable, bool is_vertex_buffer>
size_t GPUVectorBuffer<T, updatable, is_vertex_buffer>::get_size_bytes() { return buffer_size * sizeof(T); }

template<typename T, bool updatable, bool is_vertex_buffer>
const WGPUBuffer& GPUVectorBuffer<T, updatable, is_vertex_buffer>::get_buffer() { return buffer; }

template<typename T, bool updatable, bool is_vertex_buffer>
template<bool v, std::enable_if_t<v && v == is_vertex_buffer, void*>> void GPUVectorBuffer<T, updatable, is_vertex_buffer>::vertex_buffer(WGPURenderPassEncoder pass, uint32_t slot)
{
    wgpuRenderPassEncoderSetVertexBuffer(pass, slot, buffer, 0, buffer_size * sizeof(T));
}

template<typename T, bool updatable, bool is_vertex_buffer>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUVectorBuffer<T, updatable, is_vertex_buffer>::set_needs_update()
{
    needs_update = true;
}

template<typename T, bool updatable, bool is_vertex_buffer>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUVectorBuffer<T, updatable, is_vertex_buffer>::or_needs_update(bool needs_update_)
{
    needs_update |= needs_update_;
}

template<typename T, bool updatable, bool is_vertex_buffer>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUVectorBuffer<T, updatable, is_vertex_buffer>::update_buffer(WGPUQueue queue)
{
    size_valid_else_throw();
    wgpuQueueWriteBuffer(queue, buffer, 0, data.data(), buffer_size * sizeof(T));
}

template<typename T, bool updatable, bool is_vertex_buffer>
template<bool b, std::enable_if_t<b && b == updatable, void*>> void GPUVectorBuffer<T, updatable, is_vertex_buffer>::update_buffer_if_needed(WGPUQueue queue)
{
    if (needs_update) {
        update_buffer(queue);
        needs_update = false;
    }
}

template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::release_buffer()
{
    wgpuBufferDestroy(buffer);
    wgpuBufferRelease(buffer);
    buffer = nullptr;
}
template<typename T, bool updatable, bool is_vertex_buffer>
void GPUVectorBuffer<T, updatable, is_vertex_buffer>::release()
{
    if (buffer != nullptr) { release_buffer(); }
}
