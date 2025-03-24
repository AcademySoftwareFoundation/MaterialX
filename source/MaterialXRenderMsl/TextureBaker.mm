//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderMsl/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerMsl::TextureBakerMsl(unsigned int width, unsigned int height, Image::BaseType baseType) :
    TextureBaker<MslRenderer, MslShaderGenerator>(width, height, baseType, false)
{
}

MATERIALX_NAMESPACE_END
