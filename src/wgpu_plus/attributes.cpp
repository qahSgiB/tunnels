#include "wgpu_plus/attributes.hpp"



// [todo]
size_t AttributeUtils::get_attribute_format_size(WGPUVertexFormat format)
{
    switch (format) {
    case WGPUVertexFormat_Uint8x2:   return 0;
    case WGPUVertexFormat_Uint8x4:   return 0;
    case WGPUVertexFormat_Sint8x2:   return 0;
    case WGPUVertexFormat_Sint8x4:   return 0;
    case WGPUVertexFormat_Unorm8x2:  return 0;
    case WGPUVertexFormat_Unorm8x4:  return 0;
    case WGPUVertexFormat_Snorm8x2:  return 0;
    case WGPUVertexFormat_Snorm8x4:  return 0;
    case WGPUVertexFormat_Uint16x2:  return 0;
    case WGPUVertexFormat_Uint16x4:  return 0;
    case WGPUVertexFormat_Sint16x2:  return 0;
    case WGPUVertexFormat_Sint16x4:  return 0;
    case WGPUVertexFormat_Unorm16x2: return 0;
    case WGPUVertexFormat_Unorm16x4: return 0;
    case WGPUVertexFormat_Snorm16x2: return 0;
    case WGPUVertexFormat_Snorm16x4: return 0;
    case WGPUVertexFormat_Float16x2: return 0;
    case WGPUVertexFormat_Float16x4: return 0;
    case WGPUVertexFormat_Float32:   return sizeof(float) * 1;
    case WGPUVertexFormat_Float32x2: return sizeof(float) * 2;
    case WGPUVertexFormat_Float32x3: return sizeof(float) * 3;
    case WGPUVertexFormat_Float32x4: return sizeof(float) * 4;
    case WGPUVertexFormat_Uint32:    return 0;
    case WGPUVertexFormat_Uint32x2:  return 0;
    case WGPUVertexFormat_Uint32x3:  return 0;
    case WGPUVertexFormat_Uint32x4:  return 0;
    case WGPUVertexFormat_Sint32:    return 0;
    case WGPUVertexFormat_Sint32x2:  return 0;
    case WGPUVertexFormat_Sint32x3:  return 0;
    case WGPUVertexFormat_Sint32x4:  return 0;
    default: break;
    }
    return 0;
}



BufferAttribute BufferAttribute::create(size_t shader_attribute_index, std::optional<size_t> offset)
{
    return BufferAttribute { .shader_attribute_index = shader_attribute_index, .offset = offset };
}

BufferAttributes BufferAttributes::create(std::vector<BufferAttribute> attributes, std::optional<size_t> stride)
{
    return BufferAttributes { .attributes = std::move(attributes), .stride = stride };
}

ShaderAttribute ShaderAttribute::create(WGPUVertexFormat format, uint32_t location)
{
    return ShaderAttribute { .format = format, .location = location };
}



ShaderAttributes::ShaderAttributes(std::vector<ShaderAttribute> vertex_sas_, std::vector<ShaderAttribute> instance_sas_) : vertex_sas(std::move(vertex_sas_)), instance_sas(std::move(instance_sas_)) {}


std::tuple<std::vector<WGPUVertexBufferLayout>, std::vector<WGPUVertexAttribute>> ShaderAttributes::create_buffers_layouts(const std::vector<BufferAttributes>& vertex_buffers, const std::vector<BufferAttributes>& instance_buffers) const
{
    size_t buffers_count = vertex_buffers.size() + instance_buffers.size();
    size_t sa_count = vertex_sas.size() + instance_sas.size();

    std::vector<WGPUVertexBufferLayout> wgpu_buffer_layouts;
    wgpu_buffer_layouts.reserve(buffers_count);

    std::vector<WGPUVertexAttribute> wgpu_attributes;
    wgpu_attributes.reserve(sa_count);

    add_buffers_layouts(instance_sas, instance_buffers, wgpu_attributes, wgpu_buffer_layouts, WGPUVertexStepMode_Instance);
    add_buffers_layouts(vertex_sas, vertex_buffers, wgpu_attributes, wgpu_buffer_layouts, WGPUVertexStepMode_Vertex);

    return { std::move(wgpu_buffer_layouts), std::move(wgpu_attributes) };
}

void ShaderAttributes::add_buffers_layouts(const std::vector<ShaderAttribute>& sas, const std::vector<BufferAttributes>& buffers, std::vector<WGPUVertexAttribute>& wgpu_attributes, std::vector<WGPUVertexBufferLayout>& wgpu_buffer_layouts, WGPUVertexStepMode step_mode)
{
    for (const BufferAttributes& buffer_attributes : buffers) {
        size_t wgpu_attributes_offset = wgpu_attributes.size();
        size_t buffer_attributes_count = buffer_attributes.attributes.size();

        size_t auto_offset = 0;

        bool calculate_stride = !buffer_attributes.stride.has_value();
        size_t auto_stride = calculate_stride ? 0 : buffer_attributes.stride.value();

        for (const BufferAttribute& buffer_attribute : buffer_attributes.attributes) {
            const ShaderAttribute& shader_attribute = sas[buffer_attribute.shader_attribute_index];

            if (buffer_attribute.offset.has_value()) {
                auto_offset = buffer_attribute.offset.value();
            }

            wgpu_attributes.push_back(WGPUVertexAttribute {
                .format = shader_attribute.format,
                .offset = auto_offset,
                .shaderLocation = shader_attribute.location,
            });

            auto_offset += AttributeUtils::get_attribute_format_size(shader_attribute.format);

            if (calculate_stride && auto_offset > auto_stride) {
                auto_stride = auto_offset;
            }
        }

        wgpu_buffer_layouts.push_back(WGPUVertexBufferLayout {
            .arrayStride = auto_stride,
            .stepMode = step_mode,
            .attributeCount = static_cast<uint32_t>(buffer_attributes_count),
            .attributes = wgpu_attributes.data() + wgpu_attributes_offset,
        });
    }
}



const ShaderAttributes ImpostorSphereAttributes::ATTRIBUTES{
    {},
    {
        ShaderAttribute { // position and radius
            .format = WGPUVertexFormat_Float32x4,
            .location = 0,
        },
        ShaderAttribute { // color
            .format = WGPUVertexFormat_Float32x3,
            .location = 1,
        },
    }
};

const size_t ImpostorSphereAttributes::POSITION_RADIUS = 0;
const size_t ImpostorSphereAttributes::COLOR = 1;



const ShaderAttributes MeshAttributes::ATTRIBUTES{
    {
        ShaderAttribute { // position
            .format = WGPUVertexFormat_Float32x3,
            .location = 0,
        },
        ShaderAttribute { // normal
            .format = WGPUVertexFormat_Float32x3,
            .location = 1,
        },
        ShaderAttribute { // color
            .format = WGPUVertexFormat_Float32x3,
            .location = 2,
        },
    },
    {}
};

const size_t MeshAttributes::POSITION = 0;
const size_t MeshAttributes::NORMAL = 1;
const size_t MeshAttributes::COLOR = 2;