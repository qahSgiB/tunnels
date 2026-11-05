struct Camera {
    projection: mat4x4<f32>,
    view: mat4x4<f32>,
    projection_inv: mat4x4<f32>,
    view_inv: mat4x4<f32>,
    pos: vec3<f32>,
};

struct Model {
    m: mat4x4<f32>,
    m_it: mat3x3<f32>,
}



struct VertexOutput {
    @builtin(position) pos_clip_out: vec4<f32>,
    @location(0) pos_clip: vec4<f32>,
    @location(1) center_pos_world: vec3<f32>,
    @location(2) center_pos_view: vec4<f32>,
    @location(3) sphere_color: vec3<f32>,
    @location(4) radius: f32,
};

struct FragmentOutput {
    @builtin(frag_depth) depth: f32,
    @location(0) color: vec4<f32>,
}



@group(0) @binding(0) var<uniform> camera: Camera;
@group(1) @binding(0) var<uniform> model: Model;

@group(2) @binding(0) var<uniform> radius_mult: f32;



@vertex
fn vs_main(
    @builtin(vertex_index) index: u32,
    @location(0) sphere_posr: vec4<f32>,
    @location(1) sphere_color: vec3<f32>
) -> VertexOutput {
    // [todo] move out of function (naga error with const)
    var circle_poss = array<vec2<f32>, 4>(
        vec2<f32>( 1.0,  1.0),
        vec2<f32>(-1.0,  1.0),
        vec2<f32>( 1.0, -1.0),
        vec2<f32>(-1.0, -1.0),
    );

    var center_pos_world = model.m * vec4<f32>(0.0, 0.0, 0.0, 1.0) + vec4<f32>(sphere_posr.xyz, 0.0);

    var to_camera = camera.pos - center_pos_world.xyz;
    var camera_e1 = vec3<f32>(0.0, 0.0, 1.0);
    if (to_camera.z != 0.0) {
        camera_e1 = normalize(vec3<f32>(-to_camera.y, to_camera.x, 0.0));
    }
    var camera_e2 = normalize(cross(to_camera, camera_e1));

    var circle_pos = circle_poss[index];
    var radius = sphere_posr.w * radius_mult;

    var offset_world = (circle_pos.x * camera_e1 + circle_pos.y * camera_e2) * radius;
    var billboard_offset_world = normalize(to_camera) * radius;
    var pos_world = center_pos_world + vec4(offset_world + billboard_offset_world, 0.0);

    var center_pos_view = camera.view * center_pos_world;
    var pos_clip = camera.projection * camera.view * pos_world;

    var out: VertexOutput;
    out.pos_clip_out = pos_clip;
    out.pos_clip = pos_clip;
    out.center_pos_world = center_pos_world.xyz;
    out.center_pos_view = center_pos_view;
    out.sphere_color = sphere_color;
    out.radius = radius;

    return out;
}


@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    // if (in.sphere_color.x == 0.0 && in.sphere_color.y == 0.0 && in.sphere_color.z == 0.0) {
    //     discard;
    // }

    // impostor sphere calculation (view space calculation)
    var p = in.center_pos_view.xyz;
    var o4 = camera.projection_inv * in.pos_clip;
    var o = o4.xyz / o4.w; // [todo] is this correct?
    var a = dot(p, o) / dot(o, o);
    var q = p - a * o;
    if (length(q) > in.radius) {
        discard;
    }
    var k = sqrt(in.radius * in.radius - dot(q, q));
    var s = a * o - k * normalize(o);

    var pos_view = vec4(s, 1.0);
    var pos_clip = camera.projection * pos_view;
    var depth = pos_clip.z / pos_clip.w;

    var pos_world = (camera.view_inv * pos_view).xyz;
    var normal_world = normalize(pos_world - in.center_pos_world);

    // material params
    var material_shininess = 100.0;
    var material_ambient = in.sphere_color * 0.1;
    var material_diffuse = in.sphere_color * 0.9;
    var material_specular = vec3<f32>(0.2);

    var normal = normal_world;
    var to_camera = normalize(camera.pos - pos_world);

    var amb_c = vec3<f32>(0.0);
    var dif_c = vec3<f32>(0.0);
    var spe_c = vec3<f32>(0.0);

    //  --  iter lights
    // light params
    // var light_pos = vec3<f32>(60.0, 40.0, 0.0);
    var light_pos = camera.pos;

    var light_ambient = vec3<f32>(1.0, 1.0, 1.0);
    var light_diffuse = vec3<f32>(1.0, 1.0, 1.0);
    var light_specular = vec3<f32>(1.0, 1.0, 1.0);

    var to_light = normalize(light_pos - pos_world);
    var half = normalize(to_camera + to_light);

    var light_amb_c = 1.0;
    var light_dif_c = max(dot(normal, to_light), 0.0);
    var light_spe_c = 0.0;
    if (light_dif_c > 0.0) {
        light_spe_c = pow(max(dot(normal, half), 0.0), material_shininess);
    }

    amb_c += light_amb_c * light_ambient;
    dif_c += light_dif_c * light_diffuse;
    spe_c += light_spe_c * light_specular;
    //  --  iter lights

    var ambient = material_ambient * amb_c;
    var diffuse = material_diffuse * dif_c;
    var specular = material_specular * spe_c;

    var color = ambient + diffuse + specular;

    var out: FragmentOutput;
    out.color = vec4<f32>(color, 1.0);
    out.depth = depth;

    return out;
}