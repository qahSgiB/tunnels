#include "app.hpp"


#include "algorithm/algorithm.hpp"

#include "bio/element.hpp"

#include "platform/loop.hpp"
#include "platform/wgpu.hpp"

#include "wgpu_plus/attributes.hpp"


#include "glm/trigonometric.hpp"
#include "glm/gtc/round.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_wgpu.h"


#include <array>
#include <unordered_set>
#include <optional>
#include <tuple>
#include <ranges>
#include <iostream>



TunnelsApplication::TunnelsApplication()
{
    // [todo] maybe move out of TunnelsApplication
    #ifdef __EMSCRIPTEN__
    assets_folder = "assets/";
    #else
    assets_folder = "assets/";
    #endif

    pending_recompute = false;

    visu_show_molecule = VisuShowMolecule::None;
    visu_show_tunnel = false;
    visu_show_roi = false;
    visu_show_tunnel_surface = true;

    // [note] default example settings
    algorithm_filter_type = DepthMapFilterType::Min;
    algorithm_filter_min = DepthMapFilterMin::create(glm::radians(60.0f), 2.0f, false);
    algorithm_filter_gaussian = DepthMapFilterGaussian::create(glm::radians(60.0f), false);

    algorithm_settings = AlgorithmSettings {
        .probe = 0.7f,
        .healpix_nside = 16,
        .depth_map_use_probe = false,
        .depth_map_filter = algorithm_filter_min,
        .voxel_size = 0.2f,
        .molecule_spheres_only_intersecting = true,
        .segmentation = true,
        .force_tunnel = true,
        .smoothing_iter_count = 3,
    };

    molecule_pdb_path = "bio/example-molecule/model.1.pdb";
    tunnel_pdb_path = "bio/example-molecule/tunnels/tunel.1.pdb";

    algorithm_debug_info = true; // [todo] change to false

    window_size = glm::u32vec2(800, 600);

    Element::world_init();
}

void TunnelsApplication::init()
{
    init_graphics();
    init_pipeline();
    init_biology();
    compute_algorithm();
    wgpu_instance_process_events();
}

bool TunnelsApplication::loop()
{
    render();
    return true;
}

void TunnelsApplication::destroy()
{
    /* biology */
    tunnel_radius_mult.release();
    tunnel_colors.release();
    tunnel_posrs.release();

    molecule_radius_mult.release();
    molecule_colors.release();
    molecule_posrs.release();

    /* pipeline */
    model_identity.release();
    camera.release();

    wgpuRenderPipelineRelease(mesh_pipeline);
    wgpuPipelineLayoutRelease(mesh_pipeline_layout);
    wgpuShaderModuleRelease(mesh_shader);

    wgpuRenderPipelineRelease(is_pipeline);
    wgpuPipelineLayoutRelease(is_pipeline_layout);
    wgpuShaderModuleRelease(is_shader);

    wgpuBindGroupLayoutRelease(camera_bind_group_layout);
    wgpuBindGroupLayoutRelease(model_bind_group_layout);

    wgpuTextureViewRelease(depth_texture_view);
    wgpuTextureRelease(depth_texture);

    /* graphics - imgui */
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    /* graphics - webgpu */
    wgpuSwapChainRelease(swapchain);
    wgpuQueueRelease(queue);
    wgpuSurfaceRelease(surface);
    wgpuDeviceRelease(device);

    wgpu_release_instance();

    /* graphics - glfw */
    glfwTerminate();
}

void TunnelsApplication::run()
{
    init();
    emscripten_glfw_loop(window, *this);
}

void TunnelsApplication::init_graphics()
{
    /* glfw */
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(window_size.x, window_size.y, "tunelator 2023 pro", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, on_glfw_cursor_pos);
    glfwSetMouseButtonCallback(window, on_glfw_mouse_button);
    glfwSetFramebufferSizeCallback(window, on_glfw_framebuffer_size);

    glfwShowWindow(window);

    window_size = miscGlfwGetWindowSize(window);
    last_mouse_pos = miscGlfwGetCursorPos(window);

    /* wgpu */
    wgpu_init_instance();

    device = wgpu_create_device();
    // device = wgpu_create_device(WGPUBackendType_Vulkan);
    // device = wgpu_create_device(WGPUBackendType_D3D11); // [note] fastest on windows
    if (device == nullptr) { throw std::runtime_error("wgpu error - wgpu_create_device"); }

    wgpu_log_adapters(device);

    wgpuDeviceSetUncapturedErrorCallback(device, on_wgpu_error, nullptr);
    #ifndef __EMSCRIPTEN__ // [todo] try updating emscripten
    wgpuDeviceSetDeviceLostCallback(device, on_wgpu_device_lost, nullptr);
    #endif
    
    queue = wgpuDeviceGetQueue(device);
    if (queue == nullptr) { throw std::runtime_error("wgpu error - wgpuDeviceGetQueue"); }

    surface = wgpu_create_surface(window);
    if (surface == nullptr) { throw std::runtime_error("wgpu error - wgpu_create_surface"); }

    create_swapchain();

    /* imgui */
    IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_Init(device, 3, WGPUTextureFormat_BGRA8Unorm); // [todo] num frames in flight (second param)
}

void TunnelsApplication::init_pipeline()
{
    /* general */
    create_depth_texture();

    camera_bind_group_layout = CameraGPU::create_bind_group_layout(device, "camera bind group layout");
    model_bind_group_layout = ModelGPU::create_bind_group_layout(device, "model bind group layout");

    /* impostor sphere */
    radius_mult_bind_group_layout = FloatGPU::create_bind_group_layout(device, "atom radius multiplier bind group layut");

    std::array<WGPUBindGroupLayout, 3> is_bind_group_layouts{ camera_bind_group_layout, model_bind_group_layout, radius_mult_bind_group_layout };

    auto [is_vertex_buffer_layouts, is_vertex_buffer_attributes] = ImpostorSphereAttributes::ATTRIBUTES.create_buffers_layouts(
        {},
        {
            BufferAttributes::create({
                BufferAttribute::create(ImpostorSphereAttributes::POSITION_RADIUS)
            }),
            BufferAttributes::create({
                BufferAttribute::create(ImpostorSphereAttributes::COLOR)
            })
        }
    );

    std::tie(is_shader, is_pipeline_layout, is_pipeline) = wgpu_create_simple_pipeline(
        device,
        assets_folder / "shaders/impostor_sphere.wgsl",
        std::span(is_bind_group_layouts),
        std::span(is_vertex_buffer_layouts),
        WGPUPrimitiveTopology_TriangleStrip,
        "shader - impostor sphere",
        "pipeline layout - impostor sphere",
        "pipeline - impostor sphere"
    );

    /* mesh */
    std::array<WGPUBindGroupLayout, 2> mesh_bind_group_layouts{ camera_bind_group_layout, model_bind_group_layout };

    auto [mesh_vertex_buffer_layouts, mesh_vertex_buffer_attributes] = MeshAttributes::ATTRIBUTES.create_buffers_layouts(
        {
            BufferAttributes::create({
                BufferAttribute::create(MeshAttributes::POSITION),
                BufferAttribute::create(MeshAttributes::NORMAL),
                BufferAttribute::create(MeshAttributes::COLOR)
            })
        },
        {}
    );

    std::tie(mesh_shader, mesh_pipeline_layout, mesh_pipeline) = wgpu_create_simple_pipeline(
        device,
        assets_folder / "shaders/mesh.wgsl",
        std::span(mesh_bind_group_layouts),
        std::span(mesh_vertex_buffer_layouts),
        WGPUPrimitiveTopology_TriangleList,
        "shader - mesh",
        "pipeline layout - mesh",
        "pipeline - mesh"
    );

    /* camera + model */
    gui_camera = GUICamera(glm::radians(85.0f), 0.5f, 250.0f, glm::radians(1.0f), glm::radians(1.0f), 1.0f);
    gui_camera.set_position(glm::vec3(0.0f), glm::radians(180.0f), glm::radians(20.0f), 60.0f);
    gui_camera.set_camera(camera.data());

    camera.data().projection = glm::perspective(glm::radians(45.0f), static_cast<float>(window_size.x) / window_size.y, 0.1f, 500.0f);
    camera.data().update_projection_inv();

    camera.set_needs_update();

    model_identity.data() = Model(glm::identity<glm::mat4>());

    camera.init(device, camera_bind_group_layout, nullptr, "main camera");
    model_identity.init(device, model_bind_group_layout, queue, "idenity model");
}

void TunnelsApplication::init_biology()
{
    /* molecule - data */
    std::vector<PDBAtom> molecule_pdb = PDBAtom::atoms_from_filepath(assets_folder / molecule_pdb_path);

    auto rng1 = molecule_pdb
        | std::views::transform(ProteinAtom::from_pdb_atom)
        | std::views::filter([] (const auto& atom) { return atom.has_value(); })
        | std::views::transform([] (const auto& atom) { return std::move(*atom); });
    molecule.insert(molecule.end(), rng1.begin(), rng1.end());

    auto rng2 = molecule | std::views::transform([] (const auto& atom) { return glm::vec4(atom.pos, atom.radius); });
    molecule_posrs.data.insert(molecule_posrs.data.end(), rng2.begin(), rng2.end());

    auto rng3 = molecule | std::views::transform([] (const auto& atom) {
        auto color_iter = element_coloring.find(atom.symbol);
        return (color_iter == element_coloring.end()) ? glm::vec3(0.0f) : color_iter->second;
    });
    molecule_colors.data.insert(molecule_colors.data.end(), rng3.begin(), rng3.end());

    visu_molecule_spheres_cap = molecule.size();

    /* camera */
    glm::vec3 center(0.0f);
    for (glm::vec4 m_sphere : molecule_posrs.data) { center += glm::vec3(m_sphere); }
    center /= static_cast<float>(molecule_posrs.data.size());

    gui_camera.center = center;
    gui_camera.set_camera(camera.data());
    camera.update_buffer(queue);

    /* molecule - webgpu */
    molecule_posrs.init(device, WGPUBufferUsage_None, 0, queue, "molecule spheres positions and radiuses");
    molecule_colors.init(device, WGPUBufferUsage_None, 0, queue, "molecule spheres colors");

    molecule_radius_mult.data() = 1.0f;
    molecule_radius_mult.init(device, radius_mult_bind_group_layout, queue, "molecule radius multiplier");

    /* tunnel - data */
    std::vector<PDBAtom> tunnel_pdb = PDBAtom::atoms_from_filepath(assets_folder / tunnel_pdb_path);

    auto rng4 = tunnel_pdb | std::views::transform([] (const auto& sphere) { return glm::vec4(sphere.pos, sphere.radius); });
    tunnel_posrs.data.insert(tunnel_posrs.data.end(), rng4.begin(), rng4.end());

    tunnel_colors.data.assign(tunnel_posrs.data.size(), glm::vec3(0.5f));

    /* tunnel - webgpu */
    tunnel_posrs.init(device, WGPUBufferUsage_None, 0, queue, "tunnel spheres positions and radiuses");
    tunnel_colors.init(device, WGPUBufferUsage_None, 0, queue, "tunnel spheres colors");

    tunnel_radius_mult.data() = 1.0f;
    tunnel_radius_mult.init(device, radius_mult_bind_group_layout, queue, "tunnel radius multiplier");
}

void TunnelsApplication::create_swapchain()
{
    const WGPUSwapChainDescriptor swapchain_descriptor = {
        .usage = WGPUTextureUsage_RenderAttachment,
        .format = WGPUTextureFormat_BGRA8Unorm,
        .width = window_size.x,
        .height = window_size.y,
        .presentMode = WGPUPresentMode_Fifo,
    };

    swapchain = wgpuDeviceCreateSwapChain(device, surface, &swapchain_descriptor);
}

void TunnelsApplication::create_depth_texture()
{
    std::tie(depth_texture, depth_texture_view) = wgpu_create_simple_depth_texture_with_view(device, window_size);
}

void TunnelsApplication::compute_algorithm()
{
    /* algorithm */
    std::unordered_set<size_t> around;
    std::vector<glm::vec3> tunnel_surface_uncolored;

    std::tie(roi_posrs.data, around, tunnel_surface_uncolored) = compute(std::span(tunnel_posrs.data), std::span(molecule_posrs.data), algorithm_settings, algorithm_debug_info);

    last_algorithm_settings = algorithm_settings;

    /* around */
    auto rng1 = around | std::views::transform([this] (size_t molecule_index) { return molecule[molecule_index].aa_id; });
    std::unordered_set<size_t> around_aas(rng1.begin(), rng1.end());

    for (size_t molecule_sphere_index = 0; molecule_sphere_index < molecule_posrs.data.size(); molecule_sphere_index++) {
        if (around.contains(molecule_sphere_index)) {
            molecule_around_posrs.data.push_back(molecule_posrs.data[molecule_sphere_index]);
            molecule_around_colors.data.push_back(molecule_colors.data[molecule_sphere_index]);
            // molecule_around_colors.data.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // [note] blue atom coloring
        }
    }

    for (size_t molecule_sphere_index = 0; molecule_sphere_index < molecule_posrs.data.size(); molecule_sphere_index++) {
        size_t aa_id = molecule[molecule_sphere_index].aa_id;

        if (around_aas.contains(aa_id)) {
            molecule_around_aas_posrs.data.push_back(molecule_posrs.data[molecule_sphere_index]);
            molecule_around_aas_colors.data.push_back(molecule_colors.data[molecule_sphere_index]);
        }
    }

    /* around webgpu */
    molecule_around_posrs.init(device, WGPUBufferUsage_None, 0, queue, "molecule around spheres positions and radiuses");
    molecule_around_colors.init(device, WGPUBufferUsage_None, 0, queue, "molecule around spheres colors");

    molecule_around_aas_posrs.init(device, WGPUBufferUsage_None, 0, queue, "molecule around aminoacids spheres positions and radiuses");
    molecule_around_aas_colors.init(device, WGPUBufferUsage_None, 0, queue, "molecule around aminoacids spheres colors");

    /* roi colors + webgpu */
    roi_colors.data.assign(roi_posrs.data.size(), glm::vec3(0.0f, 0.0f, 1.0f));

    roi_posrs.init(device, WGPUBufferUsage_None, 0, queue, "ROI spheres positions and radiuses");
    roi_colors.init(device, WGPUBufferUsage_None, 0, queue, "ROI spheres colors");

    roi_radius_mult.data() = 1.0f;
    roi_radius_mult.init(device, radius_mult_bind_group_layout, queue, "ROI radius multiplier");

    /* tunnel surface webgpu */
    for (size_t vertex_index = 0; vertex_index < tunnel_surface_uncolored.size(); vertex_index += 2) {
        tunnel_surface.data.push_back(tunnel_surface_uncolored[vertex_index]);
        tunnel_surface.data.push_back(tunnel_surface_uncolored[vertex_index + 1]);
        // tunnel_surface.data.push_back(glm::vec3(0.9f, 0.15f, 0.7f));
        tunnel_surface.data.push_back(glm::vec3(1.0f, 0.4f, 0.0f));
    }

    tunnel_surface.init(device, WGPUBufferUsage_None, 0, queue, "tunnel surface mesh");

    // [note] shows that tunnel spheres and molecule spheres are overlaping
    // for (glm::vec4 t : tunnel_posrs.data) {
    //     for (glm::vec4 m : molecule_posrs.data) {
    //         float dist = glm::distance(glm::vec3(t), glm::vec3(m));
    //         float r = t.w + m.w;

    //         if (dist < r) {
    //             std::cout << dist << " | " << r << "\n";
    //         }
    //     }
    // }
}

void TunnelsApplication::recompute()
{
    if (!std::filesystem::is_regular_file(assets_folder / molecule_pdb_path)) {
        std::cout << "molecule filepath doesn't exists or is not regular file\n";
        return;
    }

    if (!std::filesystem::is_regular_file(assets_folder / tunnel_pdb_path)) {
        std::cout << "tunnel filepath doesn't exists or is not regular file\n";
        return;
    }

    molecule_posrs.release();
    molecule_posrs.data.clear();
    molecule_colors.release();
    molecule_colors.data.clear();
    tunnel_posrs.release();
    tunnel_posrs.data.clear();
    tunnel_colors.release();
    tunnel_colors.data.clear();
    roi_posrs.release();
    roi_posrs.data.clear();
    roi_colors.release();
    roi_colors.data.clear();

    molecule_around_posrs.release();
    molecule_around_posrs.data.clear();
    molecule_around_colors.release();
    molecule_around_colors.data.clear();
    molecule_around_aas_posrs.release();
    molecule_around_aas_posrs.data.clear();
    molecule_around_aas_colors.release();
    molecule_around_aas_colors.data.clear();

    tunnel_surface.release();
    tunnel_surface.data.clear();

    molecule.clear();

    init_biology();
    compute_algorithm();
}

void TunnelsApplication::update()
{
    fps_window.frame();

    if (pending_recompute) {
        recompute();
        pending_recompute = false;
    }
}

void TunnelsApplication::render()
{
    update();

    render_imgui();
    render_draw();
}

void TunnelsApplication::render_imgui()
{
    /* new frame */
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    /* sizing */
    float menu_unit = ImGui::GetFontSize();
    ImVec2 menu_spacing_size(10.0f, menu_unit * 0.2f);
    
    float visu_menu_width = menu_unit * 20.0f;
    float algorithm_menu_width = menu_unit * 25.0f;

    float menu_offset = 10.0f;

    /* visu window */
    ImGui::Begin("visualization settings");
    ImGui::SetWindowPos(ImVec2(menu_offset, menu_offset));
    ImGui::SetWindowSize(ImVec2(visu_menu_width, 0.0));
    ImGui::PushItemWidth(visu_menu_width * 0.5f);

    render_imgui_visu(menu_spacing_size);

    ImGui::End();

    /* algorithm window */
    ImGui::Begin("algorithm setting");
    ImGui::SetWindowPos(ImVec2(window_size.x - algorithm_menu_width - menu_offset, menu_offset));
    ImGui::SetWindowSize(ImVec2(algorithm_menu_width, 0.0));
    ImGui::PushItemWidth(algorithm_menu_width * 0.5f);

    render_imgui_algorithm(menu_spacing_size);

    ImGui::End();

    /* finish */    
    ImGui::Render();

    /* update buffers */
    camera.update_buffer_if_needed(queue);
    molecule_radius_mult.update_buffer_if_needed(queue);
    tunnel_radius_mult.update_buffer_if_needed(queue);
    roi_radius_mult.update_buffer_if_needed(queue);
}

void TunnelsApplication::render_imgui_visu(ImVec2 menu_spacing_size)
{
    bool gui_camera_changed = false;
    float gui_camera_max_elevation_deg = glm::degrees(gui_camera.max_elevation);
    int visu_show_molecule_i = static_cast<int>(visu_show_molecule);
    int visu_molecule_spheres_cap_i = static_cast<int>(visu_molecule_spheres_cap);

    /* fps */
    ImGui::Text("fps : %d", static_cast<int>(glm::round(fps_window.getFps())));

    /* ui camera */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  camera  ========");
    ImGui::Dummy(menu_spacing_size);

    gui_camera_changed |= ImGui::SliderAngle("angle", &gui_camera.angle, 0.0f, 360.0f, "%.1f");
    gui_camera_changed |= ImGui::SliderAngle("elevation", &gui_camera.elevation, -gui_camera_max_elevation_deg, gui_camera_max_elevation_deg, "%.1f");
    gui_camera_changed |= ImGui::SliderFloat("distance", &gui_camera.distance, gui_camera.min_distance, gui_camera.max_distance, "%.1f");

    /* ui molecule */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  molecule  ========");
    ImGui::Dummy(menu_spacing_size);
    ImGui::PushID("molecule");

    ImGui::RadioButton("full", &visu_show_molecule_i, static_cast<int>(VisuShowMolecule::All));
    ImGui::RadioButton("around atoms", &visu_show_molecule_i, static_cast<int>(VisuShowMolecule::Around));
    ImGui::RadioButton("around aminoacids", &visu_show_molecule_i, static_cast<int>(VisuShowMolecule::AroundAA));
    ImGui::RadioButton("hide", &visu_show_molecule_i, static_cast<int>(VisuShowMolecule::None));
    molecule_radius_mult.or_needs_update(ImGui::SliderFloat("radius", &molecule_radius_mult.data(), 0.1f, 1.0f, "%.3f"));
    ImGui::SliderInt("cap", &visu_molecule_spheres_cap_i, 0, static_cast<int>(molecule.size()));

    ImGui::PopID();

    /* ui tunnel */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  tunnel  ========");
    ImGui::Dummy(menu_spacing_size);
    ImGui::PushID("tunnel");

    ImGui::Checkbox("show", &visu_show_tunnel);
    tunnel_radius_mult.or_needs_update(ImGui::SliderFloat("radius", &tunnel_radius_mult.data(), 0.1f, 1.0f, "%.3f"));

    ImGui::PopID();

    /* ui roi */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  ROI  ========");
    ImGui::Dummy(menu_spacing_size);
    ImGui::PushID("roi");

    ImGui::Checkbox("show", &visu_show_roi);
    roi_radius_mult.or_needs_update(ImGui::SliderFloat("radius", &roi_radius_mult.data(), 0.1f, 1.0f, "%.3f"));

    ImGui::PopID();

    /* ui tunnel surface */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  surface  ========");
    ImGui::Dummy(menu_spacing_size);
    ImGui::PushID("surface");

    ImGui::Checkbox("show", &visu_show_tunnel_surface);

    ImGui::PopID();

    /* update */
    visu_show_molecule = static_cast<VisuShowMolecule>(visu_show_molecule_i);
    visu_molecule_spheres_cap = static_cast<size_t>(visu_molecule_spheres_cap_i);

    if (gui_camera_changed) {
        gui_camera.set_camera(camera.data());
        camera.set_needs_update();
    }
}
void TunnelsApplication::render_imgui_algorithm([[maybe_unused]] ImVec2 menu_spacing_size)
{
    int smoothing_iter_count_i = static_cast<int>(algorithm_settings.smoothing_iter_count);
    int filter_type_i = static_cast<int>(algorithm_filter_type);

    bool filter_type_changed = false;

    /* main */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  main  ========");
    ImGui::Dummy(menu_spacing_size);

    ImGui::SliderFloat("probe", &algorithm_settings.probe, 0.25f, 2.5f, "%.2f");
    ImGui::SliderFloat("voxel size", &algorithm_settings.voxel_size, 0.05f, 1.0f, "%.2f");
    ImGui::Checkbox("segmentation", &algorithm_settings.segmentation);
    ImGui::SliderInt("smooting iterations", &smoothing_iter_count_i, 0, 10);

    /* depth map filter */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  filtering  ========");
    ImGui::Dummy(menu_spacing_size);

    filter_type_changed |= ImGui::RadioButton("none", &filter_type_i, static_cast<int>(DepthMapFilterType::None));
    filter_type_changed |= ImGui::RadioButton("minimun", &filter_type_i, static_cast<int>(DepthMapFilterType::Min));
    filter_type_changed |= ImGui::RadioButton("gaussian", &filter_type_i, static_cast<int>(DepthMapFilterType::Gaussian));

    algorithm_filter_type = static_cast<DepthMapFilterType>(filter_type_i);

    if (algorithm_filter_type == DepthMapFilterType::Min) {
        float angle_deg = glm::degrees(algorithm_filter_min.angle);

        ImGui::Dummy(menu_spacing_size);

        filter_type_changed |= ImGui::SliderFloat("angle", &angle_deg, 5.0f, 90.0f, "%.1f");
        filter_type_changed |= ImGui::SliderFloat("distance", &algorithm_filter_min.distance, 0.1f, 10.0f, "%.1f");
        filter_type_changed |= ImGui::Checkbox("per pixel", &algorithm_filter_min.per_pixel);

        algorithm_filter_min.angle = glm::radians(angle_deg);
    } else if (algorithm_filter_type == DepthMapFilterType::Gaussian) {
        float angle_deg = glm::degrees(algorithm_filter_gaussian.angle);

        ImGui::Dummy(menu_spacing_size);

        filter_type_changed |= ImGui::SliderFloat("angle", &angle_deg, 5.0f, 90.0f, "%.1f");
        filter_type_changed |= ImGui::Checkbox("per pixel", &algorithm_filter_gaussian.per_pixel);

        algorithm_filter_gaussian.angle = glm::radians(angle_deg);
    }

    /* other */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  details  ========");
    ImGui::Dummy(menu_spacing_size);

    ImGui::Checkbox("depth map use probe", &algorithm_settings.depth_map_use_probe);
    ImGui::Checkbox("msoi", &algorithm_settings.molecule_spheres_only_intersecting);
    ImGui::Checkbox("force tunnel", &algorithm_settings.force_tunnel);

    /* compute */
    ImGui::Dummy(menu_spacing_size);
    ImGui::TextUnformatted("  ========  RUN  ========");
    ImGui::Dummy(menu_spacing_size);

    ImGui::InputText("molecule file", &molecule_pdb_path);
    ImGui::InputText("tunnel file", &tunnel_pdb_path);

    ImGui::Dummy(menu_spacing_size);

    ImGui::Checkbox("debug info", &algorithm_debug_info);

    ImGui::Dummy(menu_spacing_size);

    pending_recompute |= ImGui::Button("RUN");

    /* update */
    algorithm_settings.smoothing_iter_count = static_cast<size_t>(smoothing_iter_count_i);

    if (filter_type_changed) {
        if (algorithm_filter_type == DepthMapFilterType::None) {
            algorithm_settings.depth_map_filter = DepthMapFilter(DepthMapFilterNone {});
        } else if (algorithm_filter_type == DepthMapFilterType::Min) {
            algorithm_settings.depth_map_filter = DepthMapFilter(algorithm_filter_min);
        } else {
            algorithm_settings.depth_map_filter = DepthMapFilter(algorithm_filter_gaussian);
        }
    }

    /* settings changed info */
    ImGui::Dummy(menu_spacing_size);

    if (algorithm_settings == last_algorithm_settings) {
        ImGui::TextUnformatted("");
    } else {
        ImGui::TextUnformatted("setting changed");
    }

    ImGui::Dummy(menu_spacing_size);
}

void TunnelsApplication::render_draw()
{
    /* draw */
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);

    WGPUTextureView swapchain_view = wgpuSwapChainGetCurrentTextureView(swapchain);

    WGPURenderPassEncoder pass = wgpu_create_simple_render_pass_with_depth(encoder, swapchain_view, depth_texture_view, { 1.0, 1.0, 1.0, 1.0 }, "render pass - normal");
    wgpuRenderPassEncoderSetViewport(pass, 0.0f, 0.0f, static_cast<float>(window_size.x), static_cast<float>(window_size.y), 0.0f, 1.0f);
    render_draw_is(pass);
    render_draw_mesh(pass);
    wgpuRenderPassEncoderEnd(pass);

    WGPURenderPassEncoder imgui_pass = wgpu_create_simple_render_pass(encoder, swapchain_view, { 0.0, 0.0, 0.0, 0.0 }, "render pass - imgui"); // [todo] do not clear maybe
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), imgui_pass);
    wgpuRenderPassEncoderEnd(imgui_pass);

    /* finish */
    const WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(queue, 1, &cmd_buffer);

    #ifndef __EMSCRIPTEN__
    wgpuSwapChainPresent(swapchain); // [todo] move out
    #endif

    wgpuCommandBufferRelease(cmd_buffer);
    wgpuRenderPassEncoderRelease(imgui_pass);
    wgpuRenderPassEncoderRelease(pass);
    wgpuTextureViewRelease(swapchain_view);
    wgpuCommandEncoderRelease(encoder);

    wgpu_instance_process_events();
}

void TunnelsApplication::render_draw_is(WGPURenderPassEncoder pass)
{
    /* general */
    wgpuRenderPassEncoderSetPipeline(pass, is_pipeline);

    camera.bind(pass, 0);
    model_identity.bind(pass, 1);

    /* molecule */
    molecule_radius_mult.bind(pass, 2);
    
    if (visu_show_molecule == VisuShowMolecule::All) {
        molecule_posrs.vertex_buffer(pass, 0);
        molecule_colors.vertex_buffer(pass, 1);
        wgpuRenderPassEncoderDraw(pass, 4, static_cast<uint32_t>(visu_molecule_spheres_cap), 0, 0);
    } else if (visu_show_molecule == VisuShowMolecule::Around) {
        molecule_around_posrs.vertex_buffer(pass, 0);
        molecule_around_colors.vertex_buffer(pass, 1);
        wgpuRenderPassEncoderDraw(pass, 4, static_cast<uint32_t>(molecule_around_posrs.data.size()), 0, 0);
    } else if (visu_show_molecule == VisuShowMolecule::AroundAA) {
        molecule_around_aas_posrs.vertex_buffer(pass, 0);
        molecule_around_aas_colors.vertex_buffer(pass, 1);
        wgpuRenderPassEncoderDraw(pass, 4, static_cast<uint32_t>(molecule_around_aas_posrs.data.size()), 0, 0);
    }

    /* tunnel */
    if (visu_show_tunnel) {
        tunnel_radius_mult.bind(pass, 2);
        tunnel_posrs.vertex_buffer(pass, 0);
        tunnel_colors.vertex_buffer(pass, 1);
        wgpuRenderPassEncoderDraw(pass, 4, static_cast<uint32_t>(tunnel_posrs.data.size()), 0, 0);
    }

    /* roi */
    if (visu_show_roi) {
        roi_radius_mult.bind(pass, 2);
        roi_posrs.vertex_buffer(pass, 0);
        roi_colors.vertex_buffer(pass, 1);
        wgpuRenderPassEncoderDraw(pass, 4, static_cast<uint32_t>(roi_posrs.data.size()), 0, 0);
    }
}

void TunnelsApplication::render_draw_mesh([[maybe_unused]] WGPURenderPassEncoder pass)
{
    if (visu_show_tunnel_surface) {
        wgpuRenderPassEncoderSetPipeline(pass, mesh_pipeline);
        tunnel_surface.vertex_buffer(pass, 0);

        wgpuRenderPassEncoderDraw(pass, static_cast<uint32_t>(tunnel_surface.data.size() / 3), 1, 0, 0);
    }
}

void TunnelsApplication::on_mouse_move([[maybe_unused]] glm::vec2 mouse_pos)
{
    glm::vec2 delta = mouse_pos - last_mouse_pos;

    bool right_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);

    if (right_pressed) {
        if (ctrl_pressed) {
            gui_camera.zoom_move(delta);
        } else {
            gui_camera.orientation_move(delta);
        }

        gui_camera.set_camera(camera.data());
        camera.set_needs_update();
    }

    last_mouse_pos = mouse_pos;
}

void TunnelsApplication::on_mouse_button([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods) {}

void TunnelsApplication::on_resize(glm::u32vec2 new_window_size)
{
    window_size = new_window_size;

    /* webgpu */
    wgpuSwapChainRelease(swapchain);
    create_swapchain();

    wgpuTextureViewRelease(depth_texture_view);
    wgpuTextureDestroy(depth_texture);
    wgpuTextureRelease(depth_texture);
    create_depth_texture();

    /* camera */
    camera.data().projection = glm::perspective(glm::radians(45.0f), static_cast<float>(window_size.x) / window_size.y, 0.1f, 500.0f);
    camera.data().update_projection_inv();
    camera.set_needs_update();

    /* render */
    render();
}


void TunnelsApplication::on_glfw_error(int error, const char* description)
{
    std::cout << "GLFW error : " << error << "\n";
    std::cout << description << '\n';
}

void TunnelsApplication::on_glfw_cursor_pos(GLFWwindow* window, double cx_d, double cy_d)
{
    TunnelsApplication& app = *static_cast<TunnelsApplication*>(glfwGetWindowUserPointer(window));
    app.on_mouse_move(glm::vec2(static_cast<float>(cx_d), static_cast<float>(cy_d)));
}

void TunnelsApplication::on_glfw_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    TunnelsApplication& app = *static_cast<TunnelsApplication*>(glfwGetWindowUserPointer(window));
    app.on_mouse_button(button, action, mods);
}

void TunnelsApplication::on_glfw_framebuffer_size(GLFWwindow* window, int width, int height)
{
    TunnelsApplication& app = *static_cast<TunnelsApplication*>(glfwGetWindowUserPointer(window));
    app.on_resize(glm::u32vec2(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
}

void TunnelsApplication::on_wgpu_error(WGPUErrorType error_type, const char* message, [[maybe_unused]] void* user_data)
{
    std::string error_type_s = "";
    switch (error_type) {
        case WGPUErrorType_Validation:  error_type_s = "validation"; break;
        case WGPUErrorType_OutOfMemory: error_type_s = "out of memory"; break;
        case WGPUErrorType_Unknown:     error_type_s = "unknown"; break;
        case WGPUErrorType_DeviceLost:  error_type_s = "device lost"; break;
        default:                        error_type_s = "unknown";
    }

    std::cerr << "WGPU error (type: " << error_type_s << ")\n";
    std::cerr << message << "\n";
}

void TunnelsApplication::on_wgpu_device_lost(WGPUDeviceLostReason reason, const char* message, [[maybe_unused]] void* userdata)
{
    std::string reason_s = "";
    switch (reason) {
        case WGPUDeviceLostReason_Destroyed: reason_s = "destroyed"; break;
        case WGPUDeviceLostReason_Undefined: reason_s = "undefined"; break;
        default:                             reason_s = "unknown";
    }

    std::cerr << "WGPU device lost (reason: " << reason_s << ")\n";
    std::cerr << message << "\n";
}