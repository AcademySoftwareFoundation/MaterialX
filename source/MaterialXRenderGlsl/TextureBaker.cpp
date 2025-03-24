//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderGlsl/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN
TextureBakerGlsl::TextureBakerGlsl(unsigned int width, unsigned int height, Image::BaseType baseType) :
    TextureBaker<GlslRenderer, GlslShaderGenerator>(width, height, baseType, true)
{
}
MATERIALX_NAMESPACE_END
