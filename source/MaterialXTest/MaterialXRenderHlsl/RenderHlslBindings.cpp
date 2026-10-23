//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Round-trip test for HlslResourceBindingContext: generate a shader with an
// explicit binding context attached, compile via D3DCompile, then ask the
// D3D shader reflection layer what slots it actually assigned. The slots
// reported by reflection must match the slots the binding context allocated.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <algorithm>

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

TEST_CASE("Render: Hlsl Binding Roundtrip", "[renderhlsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    // standard_surface_brass_tiled has both value uniforms (cbuffers) and
    // file textures (SamplerTexture2D), so it exercises the b#/t#/s# paths
    // simultaneously.
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

    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    bool built = prog->build(shader);
    INFO("HlslProgram compile log:\n" << prog->getCompileLog());
    REQUIRE(built);

    auto bindings = prog->getPixelBindings();
    REQUIRE_FALSE(bindings.empty());

    // Sanity: every binding the reflector reports must have a name and a
    // class we recognise.
    for (const auto& b : bindings)
    {
        INFO("binding: " << b.name << " slot=" << b.slot);
        REQUIRE_FALSE(b.name.empty());
        REQUIRE(b.type != mx::HlslResourceType::Other);
    }

    // The first cbuffer the binding context allocates must land on b0.
    auto firstCbuffer = std::min_element(bindings.begin(), bindings.end(),
        [](const mx::HlslResourceBinding& a, const mx::HlslResourceBinding& b) {
            if (a.type != mx::HlslResourceType::CBuffer) return false;
            if (b.type != mx::HlslResourceType::CBuffer) return true;
            return a.slot < b.slot;
        });
    REQUIRE(firstCbuffer != bindings.end());
    REQUIRE(firstCbuffer->type == mx::HlslResourceType::CBuffer);
    REQUIRE(firstCbuffer->slot == 0);

    // Cbuffer slots must be packed contiguously from b0.
    std::vector<unsigned int> cbufferSlots;
    for (const auto& b : bindings)
    {
        if (b.type == mx::HlslResourceType::CBuffer)
            cbufferSlots.push_back(b.slot);
    }
    std::sort(cbufferSlots.begin(), cbufferSlots.end());
    for (std::size_t i = 0; i < cbufferSlots.size(); ++i)
    {
        REQUIRE(cbufferSlots[i] == static_cast<unsigned int>(i));
    }

    // Texture and sampler counts must match: every SamplerTexture2D handle
    // contributes exactly one of each.
    std::size_t texCount = 0, sampCount = 0;
    for (const auto& b : bindings)
    {
        if (b.type == mx::HlslResourceType::Texture) ++texCount;
        else if (b.type == mx::HlslResourceType::Sampler) ++sampCount;
    }
    REQUIRE(texCount == sampCount);
    REQUIRE(texCount > 0);
}
