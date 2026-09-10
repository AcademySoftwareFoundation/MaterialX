//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TEXTUREBAKER_HLSL
#define MATERIALX_TEXTUREBAKER_HLSL

/// @file
/// HLSL-backed TextureBaker.

#include <MaterialXRender/TextureBaker.h>

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Renderer.h>
#include <MaterialXRenderHlsl/HlslRenderer.h>

#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a TextureBakerHlsl.
using TextureBakerHlslPtr = shared_ptr<class TextureBakerHlsl>;

/// @class TextureBakerHlsl
/// TextureBaker specialisation that drives the HLSL renderer + generator.
/// All baking machinery - graph traversal, shader compilation, render-to-
/// texture for each baked output, image read-back, MaterialX document
/// authoring - is provided by the templated base class; this subclass
/// only wires the HLSL backend types in.
class MX_RENDERHLSL_API TextureBakerHlsl : public TextureBaker<HlslRenderer, HlslShaderGenerator>
{
  public:
    static TextureBakerHlslPtr create(unsigned int width = 1024,
                                      unsigned int height = 1024,
                                      Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return TextureBakerHlslPtr(new TextureBakerHlsl(width, height, baseType));
    }

    TextureBakerHlsl(unsigned int width, unsigned int height, Image::BaseType baseType);
};

/// Shared pointer to a TextureBakerHlslD3D12.
using TextureBakerHlslD3D12Ptr = shared_ptr<class TextureBakerHlslD3D12>;

/// @class TextureBakerHlslD3D12
/// TextureBaker specialisation that drives the D3D12 HLSL renderer and
/// generator. As with TextureBakerHlsl, all baking machinery is provided
/// by the templated base class.
class MX_RENDERHLSL_API TextureBakerHlslD3D12 : public TextureBaker<HlslD3D12Renderer, HlslShaderGenerator>
{
  public:
    static TextureBakerHlslD3D12Ptr create(unsigned int width = 1024,
                                           unsigned int height = 1024,
                                           Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return TextureBakerHlslD3D12Ptr(new TextureBakerHlslD3D12(width, height, baseType));
    }

    TextureBakerHlslD3D12(unsigned int width, unsigned int height, Image::BaseType baseType);
};

MATERIALX_NAMESPACE_END

#endif
