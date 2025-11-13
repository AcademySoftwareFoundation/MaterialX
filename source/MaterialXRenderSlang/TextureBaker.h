//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TEXTUREBAKER_SLANG
#define MATERIALX_TEXTUREBAKER_SLANG

/// @file
/// Texture baking functionality

#include <iostream>

#include <MaterialXCore/Unit.h>
#include <MaterialXRender/TextureBaker.h>

#include <MaterialXRenderSlang/Export.h>

#include <MaterialXRenderSlang/SlangRenderer.h>
#include <MaterialXGenSlang/SlangShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// A shared pointer to a TextureBakerMsl
using TextureBakerSlangPtr = shared_ptr<class TextureBakerSlang>;

/// A vector of baked documents with their associated names.
using BakedDocumentVec = std::vector<std::pair<std::string, DocumentPtr>>;

/// @class TextureBakerSlang
/// An implementation of TextureBaker based on Slang shader generation.
class MX_RENDERSLANG_API TextureBakerSlang : public TextureBaker<SlangRenderer, SlangShaderGenerator>
{
  public:
    static shared_ptr<TextureBakerSlang> create(unsigned int width = 1024, unsigned int height = 1024, Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return shared_ptr<TextureBakerSlang>(new TextureBakerSlang(width, height, baseType));
    }

    TextureBakerSlang(unsigned int width, unsigned int height, Image::BaseType baseType);
};

MATERIALX_NAMESPACE_END

#endif
