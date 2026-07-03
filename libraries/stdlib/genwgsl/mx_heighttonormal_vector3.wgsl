// Generated from libraries/stdlib/genglsl/mx_heighttonormal_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_heighttonormal_vector3(height: f32, scale_3: f32, texcoord_29: vec2f, result_82: ptr<function, vec3f>) {
    var scale_4: f32;
    var texcoord_30: vec2f;
    var SOBEL_SCALE_FACTOR: f32 = 0.0625;
    var dHdS: vec2f;
    var dUdS: vec2f;
    var dVdS: vec2f;
    var tangent_3: vec3f;
    var bitangent_3: vec3f;
    var n_2: vec3f;

    scale_4 = scale_3;
    texcoord_30 = texcoord_29;
    dHdS = ((vec2f(dpdx(height), dpdy(height)) * scale_4) * SOBEL_SCALE_FACTOR);
    dUdS = vec2f(dpdx(texcoord_30.x), dpdy(texcoord_30.x));
    dVdS = vec2f(dpdx(texcoord_30.y), dpdy(texcoord_30.y));
    tangent_3 = vec3f(dUdS.x, dVdS.x, dHdS.x);
    bitangent_3 = vec3f(dUdS.y, dVdS.y, dHdS.y);
    n_2 = cross(tangent_3, bitangent_3);
    if (dot(n_2, n_2) < 0.0000000000000001) {
        {
            n_2 = vec3f(0.0, 0.0, 1.0);
        }
    } else {
        if (n_2.z < 0.0) {
            {
                n_2 = (n_2 * -1.0);
            }
        }
    }
    (*result_82) = ((normalize(n_2) * 0.5) + vec3(0.5));
    return;
}
