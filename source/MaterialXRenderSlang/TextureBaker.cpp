//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN
TextureBakerSlang::TextureBakerSlang(unsigned int width, unsigned int height, Image::BaseType baseType) :
    TextureBaker<SlangRenderer, SlangShaderGenerator>(width, height, baseType, true)
{
}
MATERIALX_NAMESPACE_END
