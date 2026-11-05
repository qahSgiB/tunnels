#include "wgpu_plus/camera.hpp"

#include "glm/trigonometric.hpp"
#include "glm/gtc/matrix_transform.hpp"



Camera::Camera() {}
Camera::Camera(glm::mat4 projection_, glm::mat4 view_, glm::vec3 pos_) : projection(projection_), view(view_), pos(pos_)
{
    update_projection_inv();
    update_view_inv();
}

void Camera::update_projection_inv()
{
    projection_inv = glm::inverse(projection);
}

void Camera::update_view_inv()
{
    view_inv = glm::inverse(view);
}



GUICamera::GUICamera() {}
GUICamera::GUICamera(float max_elevation_, float min_distance_, float max_distance_, float angle_vel_, float elevation_vel_, float distance_vel_) :
    max_elevation(max_elevation_),
    min_distance(min_distance_),
    max_distance(max_distance_),
    angle_vel(angle_vel_),
    elevation_vel(elevation_vel_),
    distance_vel(distance_vel_)
{}

void GUICamera::set_position(glm::vec3 center_, float angle_, float elevation_, float distance_)
{
    center = center_;
    angle = angle_;
    elevation = elevation_;
    distance = distance_;
}

void GUICamera::orientation_move(glm::vec2 delta)
{
    angle -= delta.x * angle_vel;
    angle = glm::mod(angle, glm::two_pi<float>());

    elevation += delta.y * elevation_vel;
    elevation = glm::clamp(elevation, -max_elevation, max_elevation);
}

void GUICamera::zoom_move(glm::vec2 delta)
{
    distance += delta.y * distance_vel;
    distance = glm::clamp(distance, min_distance, max_distance);
}

void GUICamera::set_camera(Camera& camera) const
{
    float camera_x = glm::sin(angle);
    float camera_z = glm::cos(angle);

    float scale = glm::cos(elevation);
    float camera_y = glm::sin(elevation);

    camera.pos = glm::vec3(camera_x * scale, camera_y, camera_z * scale) * distance + center;
    camera.view = glm::lookAt(camera.pos, center, glm::vec3(0.0f, 1.0f, 0.0f));
    camera.update_view_inv();
}