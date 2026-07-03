// Generated from libraries/pbrlib/genglsl/mx_blackbody.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_blackbody(temperatureKelvin: f32, colorValue: ptr<function, vec3f>) {
    var temperatureKelvin_1: f32;
    var xc: f32;
    var yc: f32;
    var t_6: f32;
    var t2_: f32;
    var t3_: f32;
    var xc2_: f32;
    var xc3_: f32;
    var XYZ: vec3f;

    temperatureKelvin_1 = temperatureKelvin;
    temperatureKelvin_1 = clamp(temperatureKelvin_1, 800.0, 25000.0);
    t_6 = (1000.0 / temperatureKelvin_1);
    t2_ = (t_6 * t_6);
    t3_ = ((t_6 * t_6) * t_6);
    if (temperatureKelvin_1 < 4000.0) {
        {
            xc = ((((-0.2661239 * t3_) - (0.234358 * t2_)) + (0.8776956 * t_6)) + 0.17991);
        }
    } else {
        {
            xc = ((((-3.025847 * t3_) + (2.1070378 * t2_)) + (0.2226347 * t_6)) + 0.24039);
        }
    }
    xc2_ = (xc * xc);
    xc3_ = ((xc * xc) * xc);
    if (temperatureKelvin_1 < 2222.0) {
        {
            yc = ((((-1.1063814 * xc3_) - (1.3481102 * xc2_)) + (2.1855583 * xc)) - 0.20219684);
        }
    } else {
        if (temperatureKelvin_1 < 4000.0) {
            {
                yc = ((((-0.9549476 * xc3_) - (1.3741859 * xc2_)) + (2.09137 * xc)) - 0.16748866);
            }
        } else {
            {
                yc = ((((3.081758 * xc3_) - (5.873387 * xc2_)) + (3.7511299 * xc)) - 0.37001482);
            }
        }
    }
    if (yc <= 0.0) {
        {
            (*colorValue) = vec3(1.0);
            return;
        }
    }
    XYZ = vec3f((xc / yc), 1.0, (((1.0 - xc) - yc) / yc));
    (*colorValue) = (mx_matrix_mul_mat3_vec3(XYZ_to_RGB, XYZ));
    (*colorValue) = max(((*colorValue)), vec3(0.0));
    return;
}
