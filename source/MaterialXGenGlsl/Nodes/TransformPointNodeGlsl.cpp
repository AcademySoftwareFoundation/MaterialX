//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/TransformPointNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr TransformPointNodeGlsl::create()
{
    return std::make_shared<TransformPointNodeGlsl>();
}

string TransformPointNodeGlsl::getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    return "vec4(" + shadergen.getUpstreamResult(in, context) + ", 1.0)";
}

} // namespace MaterialX
