//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CROSS_H
#define MATERIALX_CROSS_H

#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace Cross
{
    /// Initialize global state necessary for cross-compilation.
    void initialize();

    /// Finalize global state necessary for cross-compilation.
    void finalize();

    /// Cross-compile GLSL fragment code to HLSL.
    /// @param glslUniforms GLSL definitions for MaterialX uniforms.
    ///     This GLSL code is not included in the GLSL VP2 fragment because
    ///     GLSL uniforms are generated in the final shader by VP2 based on
    ///     respective XML wrapper properties. However we need to make GLSL
    ///     uniforms known to the HLSL cross-compilation toolchain or else GLSL
    ///     parsing would fail on unknown symbols.
    ///     While public MaterialX uniforms are passed as arguments to the
    ///     fragment's root function, private uniforms are not. Also, both
    ///     public and private uniform samplers are often referenced directly
    ///     from arbitrary MaterialX-generated functions.
    /// @param glslFragment The GLSL code included in the fragment.
    /// @return HLSL fragment code to be included in the fragment.
    ///
    std::string glslToHlsl(
        const std::string& glslUniforms,
        const std::string& glslFragment,
        const std::string& fragmentName
    );
MATERIALX_NAMESPACE_END
}

#endif
