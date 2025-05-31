//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSYNTAX_H
#define MATERIALX_WGSLSYNTAX_H

/// @file
/// Vulkan GLSL syntax class for WGSL

#include <MaterialXGenGlsl/VkSyntax.h>

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for Wgsl GLSL
class MX_GENGLSL_API WgslSyntax : public VkSyntax
{
  public:
    WgslSyntax(TypeSystemPtr typeSystem);

    static SyntaxPtr create(TypeSystemPtr typeSystem) { return std::make_shared<WgslSyntax>(typeSystem); }
};

MATERIALX_NAMESPACE_END

#endif
