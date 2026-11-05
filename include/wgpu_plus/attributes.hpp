#pragma once



#include "webgpu/webgpu.h"


#include <vector>
#include <tuple>
#include <optional>



class AttributeUtils
{
public:
    static size_t get_attribute_format_size(WGPUVertexFormat format);
};



/**
 * describes shader attribute in buffer (maps position in buffer with shader attribute)
 * used for buffer description
 * if offset isn't specified it will be automaticly calculated when creating buffer layout
 */
struct BufferAttribute
{
    size_t shader_attribute_index;
    std::optional<size_t> offset = std::nullopt;

    static BufferAttribute create(size_t shader_attribute_index, std::optional<size_t> offset = std::nullopt);
};

/**
 * describes multiple shader attribute in buffer + describes stride (maps positions in buffer with shader attributes)
 * used for buffer description
 * if stride isn't specified it will be automaticly calculated when creating buffer layout
 */
struct BufferAttributes
{
    std::vector<BufferAttribute> attributes;
    std::optional<size_t> stride = std::nullopt;

    static BufferAttributes create(std::vector<BufferAttribute> attributes, std::optional<size_t> stride = std::nullopt);
};


/**
 * describes shader attribute in wgsl shader file
 * used for shader description
 */
struct ShaderAttribute
{
    WGPUVertexFormat format;
    uint32_t location;

    static ShaderAttribute create(WGPUVertexFormat format, uint32_t location);
};

/**
 * describe all attributes in wgsl shader file
 * seperate per instance and per vertex attributes (<= mixing per instance and per vertex attributes in one buffer probably doesn't make sense)
 * used for shader description
 */
struct ShaderAttributes
{
    // shader attributes
    std::vector<ShaderAttribute> vertex_sas;
    std::vector<ShaderAttribute> instance_sas;


    ShaderAttributes(std::vector<ShaderAttribute> vertex_sas_, std::vector<ShaderAttribute> instance_sas_);


    /**
     * creates wgpu buffer layout and attributes from buffer to shader attributes mapping
     *
     * @param vertex_buffers per vertex attributes mapping
     * @param instance_buffers per instance attributes mapping
     */
    std::tuple<std::vector<WGPUVertexBufferLayout>, std::vector<WGPUVertexAttribute>> create_buffers_layouts(
        const std::vector<BufferAttributes>& vertex_buffers,
        const std::vector<BufferAttributes>& instance_buffers
    ) const;

private:
    static void add_buffers_layouts(
        const std::vector<ShaderAttribute>& sas,
        const std::vector<BufferAttributes>& buffers,
        std::vector<WGPUVertexAttribute>& wgpu_attributes,
        std::vector<WGPUVertexBufferLayout>& wgpu_buffer_layouts,
        WGPUVertexStepMode step_mode
    );
};



class ImpostorSphereAttributes
{
public:
    static const ShaderAttributes ATTRIBUTES;

    static const size_t POSITION_RADIUS;
    static const size_t COLOR;
};

class MeshAttributes
{
public:
    static const ShaderAttributes ATTRIBUTES;

    static const size_t POSITION;
    static const size_t NORMAL;
    static const size_t COLOR;
};