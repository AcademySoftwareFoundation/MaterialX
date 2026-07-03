struct ClosureData {
    closureType: i32,
    L: vec3f,
    V: vec3f,
    N: vec3f,
    P: vec3f,
    occlusion: f32,
}

struct BSDF {
    response: vec3f,
    throughput: vec3f,
}

struct VDF {
    response: vec3f,
    throughput: vec3f,
}

// EDF (Emission Distribution Function) is represented as a simple vec3 color value.
// In GLSL this was `#define EDF vec3`; in WGSL we use a type alias.
alias EDF = vec3f;

// In GLSL this was `#define material surfaceshader`; in WGSL we use a type alias.
// The surfaceshader struct must be defined before this file is included.
alias material = surfaceshader;

struct FresnelData {
    // Fresnel model (FRESNEL_MODEL_DIELECTRIC / _CONDUCTOR / _SCHLICK) and thin-film flag.
    model: i32,
    airy: bool,
    // Physical Fresnel.
    ior: vec3f,
    extinction: vec3f,
    // Generalized Schlick Fresnel.
    F0: vec3f,
    F82: vec3f,
    F90: vec3f,
    exponent: f32,
    // Thin film.
    thinfilm_thickness: f32,
    thinfilm_ior: f32,
    // Selects the refracted environment lookup over the reflected one (surface transmission).
    refraction: bool,
}

fn makeClosureData(closureType: i32, L: vec3f, V: vec3f, N: vec3f, P: vec3f, occlusion: f32) -> ClosureData {
    return ClosureData(closureType, L, V, N, P, occlusion);
}
