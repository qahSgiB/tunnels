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
    @builtin(position) pos_clip: vec4<f32>,
    @location(0) pos_world: vec3<f32>,
    @location(1) normal_world: vec3<f32>,
    @location(2) color: vec3<f32>,
};



@group(0) @binding(0) var<uniform> camera: Camera;
@group(1) @binding(0) var<uniform> model: Model;



@vertex
fn vs_main(
    @location(0) pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) color: vec3<f32>
) -> VertexOutput {
    var pos_world = model.m * vec4<f32>(pos, 1.0);
    var pos_clip = camera.projection * camera.view * pos_world;

    var normal_world = normalize(model.m_it * normal);

    var out: VertexOutput;
    out.pos_clip = pos_clip;
    out.pos_world = pos_world.xyz;
    out.normal_world = normal_world;
    out.color = color;

    return out;
}


@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    // return vec4<f32>(in.color, 1.0);

    // material params
    var material_shininess = 100.0;
    var material_ambient = in.color * 0.1;
    var material_diffuse = in.color * 0.9;
    var material_specular = vec3<f32>(0.2);

    var normal = normalize(in.normal_world);
    var to_camera = normalize(camera.pos - in.pos_world);

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

    var to_light = normalize(light_pos - in.pos_world);
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

    return vec4<f32>(color, 1.0);
}