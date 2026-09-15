"""Tests for mxgenwgsl.py: token expansion tables, preflight validation, comment
re-attachment, and post-restore fixups. No naga required."""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mxgenwgsl as gen


def _repo_root():
    tools = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(tools, "..", "..", ".."))


# ---------------------------------------------------------------------------
# Token expansion tables
# ---------------------------------------------------------------------------

EXPECTED_TOKEN_EXPANSIONS = {
    "$texSamplerSignature": "int mtlx_sampler_stub",
    "$texSamplerSampler2D": "mtlx_tex_sampler",
    "$albedoTable": "mtlx_albedo_table",
    "$albedoTableSize": "vec2(256.0)",
    "$envRadianceSamples": "mtlx_env_radiance_samples()",
    "$envRadianceMips": "mtlx_env_radiance_mips()",
    "$envMatrix": "mtlx_env_matrix()",
    "$envRadiance": "mtlx_env_radiance_tex()",
    "$envIrradiance": "mtlx_env_irradiance_tex()",
    "$envLightIntensity": "mtlx_env_light_intensity()",
    "$envPrefilterMip": "mtlx_env_prefilter_mip()",
    "$envRadianceSampler2D": "mtlx_tex_sampler",
    "$refractionTwoSided": "false",
    "$closureDataConstructor": "ClosureData(closureType, L, V, N, P, occlusion)",
}


def test_token_expansions_match_golden():
    assert gen.TOKEN_EXPANSIONS == EXPECTED_TOKEN_EXPANSIONS


def test_int_uniform_tokens():
    assert gen.INT_UNIFORM_TOKENS == ("$envRadianceMips", "$envRadianceSamples")


def test_restore_rules_cover_function_expansions():
    """Every restorable expansion in TOKEN_EXPANSIONS must have a TOKEN_RESTORE_RULES entry."""
    assert gen.validateExpansionSymmetry() == []


def test_patch_int_uniform_arithmetic():
    text = "$envRadianceMips - 1.0"
    assert gen.patchIntUniformTokenArithmetic(text) == "$envRadianceMips - 1i"


# ---------------------------------------------------------------------------
# Preflight validation (requires repo checkout)
# ---------------------------------------------------------------------------

def test_validate_wgsl_generator_parity():
    errors = gen.validateWgslGeneratorParity(_repo_root())
    assert errors == [], errors


def test_validate_token_coverage_missing():
    """A fake $-token should not appear in the known set."""
    known = set(gen.TOKEN_EXPANSIONS) | set(gen.WGSL_ONLY_TOKENS)
    assert "$fakeToken" not in known


def test_preflight_clean():
    errors = gen.runPreflight(_repo_root())
    assert errors == [], errors


# ---------------------------------------------------------------------------
# GLSL inline comment re-attachment
# ---------------------------------------------------------------------------

FIS_GLSL_BODY = (
    "vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, FresnelData fd)\n"
    "{\n"
    "    // Generate tangent frame.\n"
    "    X = normalize(X - dot(X, N) * N);\n"
    "    // Transform the view vector to tangent space.\n"
    "    V = vec3(dot(V, X), dot(V, Y), dot(V, N));\n"
    "    // Compute derived properties.\n"
    "    float NdotV = clamp(V.z, M_FLOAT_EPS, 1.0);\n"
    "    // Integrate outgoing radiance using filtered importance sampling.\n"
    "    vec3 radiance = vec3(0.0);\n"
    "    for (int i = 0; i < 4; i++) {\n"
    "        // Compute the half vector and incoming light direction.\n"
    "        vec3 H = mx_ggx_importance_sample_VNDF(Xi, V, alpha);\n"
    "        // Compute dot products for this sample.\n"
    "        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);\n"
    "        // Sample the environment light from the given direction.\n"
    "        vec3 Lw = mx_matrix_mul(tangentToWorld, L);\n"
    "        // Compute the Fresnel term.\n"
    "        vec3 F = mx_compute_fresnel(VdotH, fd);\n"
    "        // Add the radiance contribution of this sample.\n"
    "        radiance += sampleColor * FG;\n"
    "    }\n"
    "    // Apply the global component of the geometric term and normalize.\n"
    "    radiance /= G1V * float(4);\n"
    "    // Return the final radiance.\n"
    "    return radiance * $envLightIntensity;\n"
    "}\n"
)

FIS_WGSL_BODY = (
    "fn mx_environment_radiance(N: vec3f, V: vec3f, X: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData) -> vec3f {\n"
    "    var radiance: vec3f = vec3(0.0);\n"
    "    let X_f = normalize((X - (dot(X, N) * N)));\n"
    "    let V_f = vec3f(dot(V, X_f), dot(V, Y), dot(V, N));\n"
    "    let NdotV = clamp(V_f.z, 0.00000001, 1.0);\n"
    "    loop {\n"
    "        if !(false) { break; }\n"
    "        {\n"
    "            let H = mx_ggx_importance_sample_VNDF(Xi, V_f, alpha);\n"
    "            let NdotL = clamp(L.z, 0.00000001, 1.0);\n"
    "            let Lw = mx_matrix_mul_mat3_vec3(tangentToWorld, L);\n"
    "            let F = mx_compute_fresnel(VdotH, fd);\n"
    "            radiance = (radiance + (sampleColor * FG));\n"
    "        }\n"
    "        continuing { }\n"
    "    }\n"
    "    radiance = (radiance / vec3(G1V * 4.0));\n"
    "    return (radiance * $envLightIntensity);\n"
    "}\n"
)

EXPECTED_COMMENTS = [
    "Generate tangent frame",
    "Transform the view vector",
    "Compute derived properties",
    "Integrate outgoing radiance",
    "Compute the half vector",
    "Compute dot products",
    "Sample the environment light",
    "Compute the Fresnel term",
    "Add the radiance contribution",
    "Apply the global component",
    "Return the final radiance",
]


def test_inject_inline_comments_fis_shape():
    out = gen.injectInlineComments(FIS_GLSL_BODY, FIS_WGSL_BODY)
    for needle in EXPECTED_COMMENTS:
        assert needle in out, f"missing comment containing {needle!r}:\n{out}"
    assert out.count("Generate tangent frame") == 1, out
    assert gen.injectInlineComments(FIS_GLSL_BODY, out) == out, "not idempotent"


# ---------------------------------------------------------------------------
# Post-restore fixups (applyWgslLibPostRestore)
# ---------------------------------------------------------------------------

def test_post_restore_does_not_break_latlong_lookup():
    cleaned = """fn mx_latlong_map_lookup(dir: vec3f, transform: mat4x4f, lod: f32, $texSamplerSignature) -> vec3f {
    let envDir = normalize((mx_matrix_mul_mat4_vec4(transform, vec4f(dir.x, dir.y, dir.z, 0.0))).xyz);
    let uv = mx_latlong_projection(envDir);
    return textureSample($texSamplerSampler2D, uv, lod).rgb;
}"""
    out = gen.applyWgslLibPostRestore(cleaned)
    assert "dir_2" not in out, out
    assert "normalize((mx_matrix_mul_mat4_vec4(transform" in out, out


def test_post_restore_int_uniform_mips_arithmetic():
    raw = "let lod = mx_latlong_compute_lod(Lw, pdf, f32(($envRadianceMips - 1.0)), samples);"
    out = gen.applyWgslLibPostRestore(raw)
    assert "$envRadianceMips - 1i" in out, out
    assert "1.0" not in out, out
    gen.assertValidWgslSyntax(out.replace("$envRadianceMips", "u_envRadianceMips"), "mips_arith")


def test_post_restore_expands_env_latlong_calls():
    raw = (
        "let sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, mtlx_env_radiance_tex());\n"
        "let Li = mx_latlong_map_lookup(N, $envMatrix, 0.0, mtlx_env_irradiance_tex());"
    )
    out = gen.applyWgslLibPostRestore(raw)
    assert "mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance, $envRadianceSampler)" in out, out
    assert "mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance, $envIrradianceSampler)" in out, out


def test_post_restore_tex_lookup_swizzles():
    raw = (
        "(*result) = (mtlx_tex_lookup_rgb(uv, 0.0));\n"
        "(*out) = mtlx_tex_lookup_rg(uv);\n"
        "(*alpha) = mtlx_tex_lookup_r(uv);"
    )
    out = gen.applyWgslLibPostRestore(raw)
    assert "textureSample($texSamplerSampler2D, uv).rgb" in out, out
    assert "textureSample($texSamplerSampler2D, uv).rg" in out, out
    assert "textureSample($texSamplerSampler2D, uv).r" in out, out
