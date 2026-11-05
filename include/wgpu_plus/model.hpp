#pragma once


#include "wgpu_plus/glm_options.hpp"


#include "wgpu_plus/wgpu_binding.hpp"


#include "glm/mat4x4.hpp"
#include "glm/mat3x4.hpp"



struct Model
{
    // wgsl compatible

    glm::mat4 model;
    glm::mat3x4 model_it; // mat3x4 needed for padding

    Model();
    Model(glm::mat4 model_);

    void update_model_it();
};

static_assert(offsetof(Model, model) == 0, "incorrect Model layout (model offest)");
static_assert(offsetof(Model, model_it) == sizeof(glm::mat4), "incorrect Model layout (model_it offest)"); // [todo] use sizeof(float) * n
static_assert(sizeof(Model) == sizeof(glm::mat4) + sizeof(glm::mat4x3), "incorrect Model layout (total size)");



using ModelGPU = GPUBinding<Model>;