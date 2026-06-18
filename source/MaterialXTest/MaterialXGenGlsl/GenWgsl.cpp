//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenGlsl/wgsl/WgslShaderGenerator.h>

#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <fstream>

namespace mx = MaterialX;

namespace
{
// Count non-overlapping occurrences of a substring.
int countOccurrences(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t p = haystack.find(needle); p != std::string::npos; p = haystack.find(needle, p + needle.size()))
    {
        n++;
    }
    return n;
}

// Verify that '{' and '}' are balanced and never close below zero.
bool bracesBalanced(const std::string& source)
{
    int depth = 0;
    for (char c : source)
    {
        if (c == '{')
        {
            depth++;
        }
        else if (c == '}')
        {
            if (--depth < 0)
            {
                return false;
            }
        }
    }
    return depth == 0;
}

// Assert the structural invariants every generated pixel shader must satisfy: it is valid-looking
// WGSL (no GLSL leftovers), exposes exactly one entry function, and has balanced braces. These are
// cheap, deterministic checks; full WGSL-syntax validation (naga/tint) runs as a separate CI step.
void checkWgslInvariants(const std::string& pixel, const std::string& label)
{
    INFO("material: " << label);
    REQUIRE(pixel.length() > 0);

    // No GLSL leftovers from the GLSL->WGSL rewrite.
    CHECK(pixel.find("#version") == std::string::npos);
    CHECK(pixel.find("#define") == std::string::npos);
    CHECK(pixel.find("layout(") == std::string::npos);
    CHECK(pixel.find("texture2D ") == std::string::npos); // GLSL combined-sampler type
    CHECK(pixel.find("sampler2D ") == std::string::npos);
    CHECK(pixel.find("gl_") == std::string::npos);

    // Boolean HW uniforms are stored as i32 (WGSL uniform buffers cannot hold bool), so any
    // GLSL boolean condition on one must be lowered to an explicit `!= 0` comparison; a bare
    // `if (u_refractionTwoSided)` would be an invalid `if (i32)` in WGSL.
    CHECK(pixel.find("if (u_refractionTwoSided)") == std::string::npos);

    // MaterialX booleans map to i32 everywhere (uniforms, node-function parameters, args) so
    // the type agrees on both sides of a call. A node-graph parameter spelled `thin_walled: bool`
    // would mismatch the i32 uniform argument passed to it.
    CHECK(pixel.find("thin_walled: bool") == std::string::npos);

    // Library BSDF helpers keep native `bool` parameters; MaterialX boolean inputs passed to
    // them must be wrapped in `bool(...)`. A bare integer literal is rejected by WGSL.
    CHECK(pixel.find("roughness_vector_out, 0, 0.000000") == std::string::npos);

    // Light-bridge helpers must be WGSL `fn`s, not GLSL `int`/`void` definitions.
    CHECK(pixel.find("int numActiveLightSources()") == std::string::npos);
    CHECK(pixel.find("void sampleLightSource") == std::string::npos);

    // Volume shaders (Open PBR, glTF PBR, …) emit VDF helpers; the type must be declared in
    // emitWgslSurfaceTypes() — GLSL struct typedefs are not copied into WGSL output.
    if (pixel.find("mx_anisotropic_vdf") != std::string::npos)
        CHECK(pixel.find("struct VDF") != std::string::npos);

    // mx_hsv.glsl uses chained `float` decls, braceless `else`, `0.0f` literals, and GLSL `int()` casts.
    if (pixel.find("mx_rgbtohsv") != std::string::npos || pixel.find("mx_hsvtorgb") != std::string::npos)
    {
        CHECK(pixel.find("float s =") == std::string::npos);
        CHECK(pixel.find("float g =") == std::string::npos);
        CHECK(pixel.find("else s =") == std::string::npos);
        CHECK(pixel.find("0.0f") == std::string::npos);
        CHECK(pixel.find("int(trunc") == std::string::npos);
    }

    // `else // comment` must not be lowered to an empty `else { }` block (breaks brace balance).
    CHECK(pixel.find("else { // FRESNEL_MODEL_SCHLICK }") == std::string::npos);

    // Native WGSL struct/uniform members must keep `vec3f`/`mat4x4f` spellings (not bare GLSL `vec3`).
    CHECK(pixel.find(": vec3,") == std::string::npos);
    CHECK(pixel.find(": vec3;") == std::string::npos);
    CHECK(pixel.find(": vec4,") == std::string::npos);
    CHECK(pixel.find(": mat4x4,") == std::string::npos);

    // Exactly one WGSL entry function, and balanced scopes.
    CHECK(countOccurrences(pixel, "fn FragmentMain(") == 1);
    CHECK(bracesBalanced(pixel));
}

// Optionally write the generated shader for manual inspection, controlled by an environment
// variable so the test never depends on a hardcoded path. Set MATERIALX_WGSL_DUMP_DIR to enable.
void maybeDump(const mx::ShaderPtr& shader, const std::string& name)
{
    const std::string dumpDir = mx::getEnviron("MATERIALX_WGSL_DUMP_DIR");
    if (dumpDir.empty())
    {
        return;
    }
    std::ofstream(dumpDir + "/" + name + "_pixel.wgsl") << shader->getSourceCode(mx::Stage::PIXEL);
}

// A minimal standard_surface, kept inline so the smoke test is independent of example assets.
const std::string STANDARD_SURFACE_MTLX = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="ss" type="surfaceshader">
    <input name="base" type="float" value="1.0"/>
    <input name="base_color" type="color3" value="0.8, 0.5, 0.3"/>
    <input name="specular" type="float" value="0.5"/>
    <input name="specular_roughness" type="float" value="0.3"/>
    <input name="metalness" type="float" value="0.0"/>
  </standard_surface>
  <surfacematerial name="mat" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="ss"/>
  </surfacematerial>
</materialx>
)";

// Load the standard MaterialX data libraries used by all cases below.
mx::DocumentPtr loadStandardLibraries(const mx::FileSearchPath& searchPath)
{
    mx::DocumentPtr stdlib = mx::createDocument();
    mx::loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" },
                      searchPath, stdlib);
    return stdlib;
}
} // namespace

TEST_CASE("WgslGen: syntax uses inherited GLSL type spellings", "[genwgsl]")
{
    const mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    const mx::Syntax& syntax = generator->getSyntax();

    // WgslSyntax inherits GLSL type spellings from GlslSyntax (matching upstream); the native
    // WGSL type names are produced later, at emission time, by the generator.
    CHECK(syntax.getTypeName(mx::Type::FLOAT) == "float");
    CHECK(syntax.getTypeName(mx::Type::BOOLEAN) == "bool");
    CHECK(syntax.getTypeName(mx::Type::VECTOR3) == "vec3");
    CHECK(syntax.getTypeName(mx::Type::COLOR3) == "vec3");
}

TEST_CASE("WgslGen: hwSrgbEncodeOutput encodes surface output", "[genwgsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, STANDARD_SURFACE_MTLX);
    doc->setDataLibrary(stdlib);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().hwSrgbEncodeOutput = true;

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
    REQUIRE(shader != nullptr);

    const std::string pixel = shader->getSourceCode(mx::Stage::PIXEL);
    checkWgslInvariants(pixel, "standard_surface_srgb");
    CHECK(pixel.find("mx_srgb_encode(") != std::string::npos);
    CHECK(pixel.find("return vec4f(mx_srgb_encode(") != std::string::npos);
}

TEST_CASE("WgslGen: directional lights emit LightData binding", "[genwgsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);
    mx::loadLibraries({ "libraries/lights" }, searchPath, stdlib);
    mx::NodeDefPtr directionalLight = stdlib->getNodeDef("ND_directional_light");
    REQUIRE(directionalLight != nullptr);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, STANDARD_SURFACE_MTLX);
    doc->setDataLibrary(stdlib);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().hwMaxActiveLightSources = 4;
    mx::HwShaderGenerator::bindLightShader(*directionalLight, 1, context);

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
    REQUIRE(shader != nullptr);

    const std::string pixel = shader->getSourceCode(mx::Stage::PIXEL);
    checkWgslInvariants(pixel, "standard_surface_lights");
    CHECK(pixel.find("struct LightData") != std::string::npos);
    CHECK(pixel.find("fn numActiveLightSources") != std::string::npos);
    CHECK(pixel.find("fn sampleLightSource") != std::string::npos);
    CHECK(pixel.find("mx_directional_light(light, position, (*result))") == std::string::npos);
    CHECK(pixel.find("u_lightData") != std::string::npos);
    CHECK(countOccurrences(pixel, "var<uniform> u_lightData") == 1);

    maybeDump(shader, "standard_surface_lights");
}

TEST_CASE("WgslGen: example materials emit valid WGSL", "[genwgsl]")
{
    // Broad generation coverage across the full TestSuite + Examples corpus lives in
    // GenGlsl.cpp's "GenShader: Wgsl GLSL Shader Generation" (a generation smoke test). This
    // case adds the WGSL-specific invariant assertions on the few materials that hit
    // branches the standard_surface feature tests above do not: brass_tiled drives the image-node
    // / sampler-splitting path, and open_pbr exercises the volume (VDF) closure.
    const mx::StringVec materials = {
        "resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx",
        "resources/Materials/Examples/OpenPbr/open_pbr_default.mtlx",
    };

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);

    for (const std::string& relPath : materials)
    {
        const mx::FilePath resolved = searchPath.find(relPath);
        INFO("material file: " << relPath);
        REQUIRE(resolved.exists());

        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, resolved);
        doc->setDataLibrary(stdlib);

        // Each material gets a fresh generator so per-generation state cannot leak between cases.
        mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
        mx::GenContext context(generator);
        context.registerSourceCodeSearchPath(searchPath);
        context.registerSourceCodeSearchPath(resolved.getParentPath());

        std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
        REQUIRE(!elements.empty());

        mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
        REQUIRE(shader != nullptr);

        const std::string name = resolved.getBaseName();
        checkWgslInvariants(shader->getSourceCode(mx::Stage::PIXEL), name);

        maybeDump(shader, name);
    }
}
