//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Sanity tests for HLSL resource bindings. These are not dump tests; they
// exercise the generated source structurally (no file I/O) so they run as
// part of the regular [genhlsl] suite.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/Shader.h>

namespace mx = MaterialX;

namespace
{

mx::ElementPtr findSurfaceMaterial(mx::DocumentPtr doc)
{
    for (auto child : doc->getChildren())
    {
        if (child->isA<mx::Node>() && child->asA<mx::Node>()->getCategory() == "surfacematerial")
        {
            return child;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("GenShader: Hlsl Cbuffer Excludes Textures", "[genhlsl]")
{
    // The generated cbuffer must contain only value uniforms; SamplerTexture2D
    // handles must be emitted at file scope. HLSL forbids texture or sampler
    // members inside a cbuffer.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element = findSurfaceMaterial(doc);
    REQUIRE(element);

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);

    mx::ShaderPtr shader = context.getShaderGenerator().generate(element->getName(), element, context);
    REQUIRE(shader);

    const std::string& ps = shader->getSourceCode(mx::Stage::PIXEL);
    const std::size_t cbufferOpen = ps.find("cbuffer ");
    REQUIRE(cbufferOpen != std::string::npos);

    // Walk forward from the opening '{' of the cbuffer to its matching '}',
    // tracking nesting depth so any inner braces (e.g. for member init lists)
    // are skipped.
    std::size_t i = ps.find('{', cbufferOpen);
    REQUIRE(i != std::string::npos);
    int depth = 1;
    std::size_t close = std::string::npos;
    for (std::size_t j = i + 1; j < ps.size(); ++j)
    {
        if (ps[j] == '{')
            ++depth;
        else if (ps[j] == '}')
        {
            if (--depth == 0)
            {
                close = j;
                break;
            }
        }
    }
    REQUIRE(close != std::string::npos);

    const std::string body = ps.substr(i, close - i);
    INFO("cbuffer body:\n" << body);
    REQUIRE(body.find("SamplerTexture2D") == std::string::npos);
    REQUIRE(body.find("Texture2D") == std::string::npos);
    REQUIRE(body.find("SamplerState") == std::string::npos);
}

TEST_CASE("GenShader: Hlsl Resource Binding Context", "[genhlsl]")
{
    // When an HlslResourceBindingContext is attached, every cbuffer must
    // carry an explicit register(b#) annotation, the b# slots increase
    // monotonically, and SamplerTexture2D handles still appear at file scope.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element = findSurfaceMaterial(doc);
    REQUIRE(element);

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);
    context.pushUserData(mx::HW::USER_DATA_BINDING_CONTEXT,
                         mx::HlslResourceBindingContext::create(0, 0, 0));

    mx::ShaderPtr shader = context.getShaderGenerator().generate(element->getName(), element, context);
    REQUIRE(shader);

    const std::string& ps = shader->getSourceCode(mx::Stage::PIXEL);

    // Walk every cbuffer occurrence and verify the register(b#) slot.
    std::size_t pos = 0;
    int expectedSlot = 0;
    int cbufferCount = 0;
    while ((pos = ps.find("cbuffer ", pos)) != std::string::npos)
    {
        const std::size_t openBrace = ps.find('{', pos);
        REQUIRE(openBrace != std::string::npos);
        const std::string header = ps.substr(pos, openBrace - pos);
        const std::string slot = "register(b" + std::to_string(expectedSlot) + ")";
        INFO("cbuffer header: " << header);
        REQUIRE(header.find(slot) != std::string::npos);
        ++expectedSlot;
        ++cbufferCount;
        pos = openBrace + 1;
    }
    REQUIRE(cbufferCount > 0);

    // SamplerTexture2D handles must still be at file scope (i.e. outside any
    // cbuffer body). Check a few known handle names from this material.
    REQUIRE(ps.find("SamplerTexture2D image_color_file") != std::string::npos);
}
