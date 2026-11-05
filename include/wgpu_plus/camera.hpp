#pragma once


#include "wgpu_plus/glm_options.hpp"


#include "wgpu_plus/wgpu_binding.hpp"


#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"



struct Camera
{
    // wgsl compatible

    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 projection_inv;
    glm::mat4 view_inv;
    glm::vec3 pos;

    float _padding;

    Camera();
    Camera(glm::mat4 projection_, glm::mat4 view_, glm::vec3 pos_);

    void update_projection_inv();
    void update_view_inv();
};

// [todo] proper size calculation (compatible with wgsl)
static_assert(offsetof(Camera, projection)      == 0,                  "incorrect Camera layout (projection offest)");
static_assert(offsetof(Camera, view)            == sizeof(float) * 16, "incorrect Camera layout (view offest)");
static_assert(offsetof(Camera, projection_inv)  == sizeof(float) * 32, "incorrect Camera layout (projection_inv offest)");
static_assert(offsetof(Camera, view_inv)        == sizeof(float) * 48, "incorrect Camera layout (view_inv offest)");
static_assert(offsetof(Camera, pos)             == sizeof(float) * 64, "incorrect Camera layout (pos offest)");
static_assert(sizeof(Camera)                    == sizeof(float) * 68, "incorrect Camera layout (total size)");


struct GUICamera
{
    glm::vec3 center;
    float angle;
    float elevation;
    float distance;

    // settings
    float max_elevation;
    float min_distance;
    float max_distance;

    float angle_vel; // per pixel
    float elevation_vel;
    float distance_vel;

    GUICamera();

    /**
     * @param angle_vel_, @param elevation_vel_ are in radians per pixel
     */
    GUICamera(float max_elevation_, float min_distance_, float max_distance_, float angle_vel_, float elevation_vel_, float distance_vel_);

    void set_position(glm::vec3 center_, float angle_, float elevation_, float distance_);

    void orientation_move(glm::vec2 delta);
    void zoom_move(glm::vec2 delta);

    /**
     * calculates @param Camera object from this GUICamera (Camera object can be sent to wgsl)
     */
    void set_camera(Camera& camera) const;
};



using CameraGPU = GPUBinding<Camera>;