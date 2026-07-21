#include "mx_microfacet_sheen.wgsl"
#include "mx_microfacet_specular.wgsl"
// This shader cannot be pre-transpiled without generator specific binding information.

fn mx_generate_dir_albedo_table(fragCoord: vec2f, albedoTableSize: f32) -> vec3f 
{
    let uv = fragCoord / albedoTableSize;
    let ggxDirAlbedo = mx_ggx_dir_albedo(uv.x, uv.y, vec3f(1.0, 0.0, 0.0), vec3f(0.0, 1.0, 0.0)).xy;
    let sheenDirAlbedo = mx_imageworks_sheen_dir_albedo(uv.x, uv.y);
    return vec3f(ggxDirAlbedo.x, ggxDirAlbedo.y, sheenDirAlbedo);
}
