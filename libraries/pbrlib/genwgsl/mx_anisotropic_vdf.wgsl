// Generated from libraries/pbrlib/genglsl/mx_anisotropic_vdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_anisotropic_vdf(closureData_21: ClosureData, absorption_2: vec3f, scattering: vec3f, anisotropy_2: f32, vdf: ptr<function, VDF>) {
    var closureData_22: ClosureData;
    var absorption_3: vec3f;
    var anisotropy_3: f32;

    closureData_22 = closureData_21;
    absorption_3 = absorption_2;
    anisotropy_3 = anisotropy_2;
    if (closureData_22.closureType == 2i) {
        {
            (*vdf).response = vec3(0.0);
            (*vdf).throughput = exp(-(absorption_3));
            return;
        }
    } else {
        return;
    }
}
