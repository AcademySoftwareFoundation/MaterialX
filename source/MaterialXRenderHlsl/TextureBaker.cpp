//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerHlsl::TextureBakerHlsl(unsigned int width, unsigned int height,
                                   Image::BaseType baseType) :
    // Same flip-saved-image flag as the GLSL baker - D3D's pixel
    // coordinate origin is top-left, OpenGL's is bottom-left, so
    // both backends flip on save to keep on-disk images upright.
    TextureBaker<HlslRenderer, HlslShaderGenerator>(width, height, baseType, true)
{
}

MATERIALX_NAMESPACE_END
