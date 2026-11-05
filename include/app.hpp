#pragma once


#include "algorithm/filter.hpp"
#include "algorithm/settings.hpp"

#include "bio/atom.hpp"

#include "misc.hpp"

#include "fps.hpp"

#include "wgpu_plus/glm_options.hpp"
#include "wgpu_plus/wgpu_vector_buffer.hpp"
#include "wgpu_plus/camera.hpp"
#include "wgpu_plus/model.hpp"


#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "GLFW/glfw3.h"

#include "imgui.h"


#include <filesystem>



enum class VisuShowMolecule { All, Around, AroundAA, None };



class TunnelsApplication
{
private:
    /* misc */
    std::filesystem::path assets_folder;

    FpsWindow<100> fps_window;

    bool pending_recompute;

    /* settings - visu */
    size_t visu_molecule_spheres_cap;

    VisuShowMolecule visu_show_molecule;
    bool visu_show_tunnel;
    bool visu_show_roi;
    bool visu_show_tunnel_surface;

    /* settings - algorithm */
    AlgorithmSettings algorithm_settings;

    DepthMapFilterType algorithm_filter_type;
    DepthMapFilterMin algorithm_filter_min;
    DepthMapFilterGaussian algorithm_filter_gaussian;

    AlgorithmSettings last_algorithm_settings;

    std::string molecule_pdb_path;
    std::string tunnel_pdb_path;

    bool algorithm_debug_info;

    /* graphics - glfw */
    glm::u32vec2 window_size;
    
    GLFWwindow* window;
    glm::vec2 last_mouse_pos;

    /* graphics - webgpu */
    WGPUDevice device;
    WGPUSurface surface;
    WGPUQueue queue;
    WGPUSwapChain swapchain;

    /* pipeline */
    WGPUTexture depth_texture;
    WGPUTextureView depth_texture_view;

    WGPUBindGroupLayout model_bind_group_layout;
    WGPUBindGroupLayout camera_bind_group_layout;

    /* pipeline - impostor sphere */
    WGPUBindGroupLayout radius_mult_bind_group_layout;

    WGPUShaderModule is_shader;
    WGPUPipelineLayout is_pipeline_layout;
    WGPURenderPipeline is_pipeline;

    /* pipeline - mesh */
    WGPUShaderModule mesh_shader;
    WGPUPipelineLayout mesh_pipeline_layout;
    WGPURenderPipeline mesh_pipeline;

    /* pipeline - camera + model */
    GUICamera gui_camera;

    CameraGPU camera;
    ModelGPU model_identity;

    /* biology - molecule */
    std::vector<ProteinAtom> molecule;
    GPUVectorBuffer<glm::vec4, false, true> molecule_posrs;
    GPUVectorBuffer<glm::vec3, true, true> molecule_colors;
    FloatGPU molecule_radius_mult;

    /* biology - tunnel */
    GPUVectorBuffer<glm::vec4, false, true> tunnel_posrs;
    GPUVectorBuffer<glm::vec3, false, true> tunnel_colors;
    FloatGPU tunnel_radius_mult;

    /* algorithm - roi */
    GPUVectorBuffer<glm::vec4, false, true> roi_posrs;
    GPUVectorBuffer<glm::vec3, false, true> roi_colors;
    FloatGPU roi_radius_mult;

    /* algorithm - molecule around */
    GPUVectorBuffer<glm::vec4, false, true> molecule_around_posrs;
    GPUVectorBuffer<glm::vec3, true, true> molecule_around_colors;

    /* algorithm - molecule around aas */
    GPUVectorBuffer<glm::vec4, false, true> molecule_around_aas_posrs;
    GPUVectorBuffer<glm::vec3, true, true> molecule_around_aas_colors;

    /* algorithm - tunnel surface */
    GPUVectorBuffer<glm::vec3, false, true> tunnel_surface;

public:
    TunnelsApplication();

    void init();
    bool loop();
    void destroy();

    void run();

private:
    void init_graphics();
    void init_pipeline();
    void init_biology();

    void create_swapchain();
    void create_depth_texture();

    void compute_algorithm();

    void recompute();

    void update();

    void render();
    void render_imgui();
    void render_imgui_visu(ImVec2 menu_spacing_size);
    void render_imgui_algorithm(ImVec2 menu_spacing_size);
    void render_draw();
    void render_draw_is(WGPURenderPassEncoder pass);
    void render_draw_mesh(WGPURenderPassEncoder pass);

    void on_mouse_move(glm::vec2 mouse_pos);
    void on_mouse_button(int button, int action, int mods);
    void on_resize(glm::u32vec2 new_window_size);

    static void on_glfw_error(int error, const char* description);
    static void on_glfw_cursor_pos(GLFWwindow* window, double cx_d, double cy_d);
    static void on_glfw_mouse_button(GLFWwindow* window, int button, int action, int mods);
    static void on_glfw_framebuffer_size(GLFWwindow* window, int width, int height);

    static void on_wgpu_error(WGPUErrorType error_type, const char* message, void* user_data);
    static void on_wgpu_device_lost(WGPUDeviceLostReason reason, const char* message, void* userdata);
};