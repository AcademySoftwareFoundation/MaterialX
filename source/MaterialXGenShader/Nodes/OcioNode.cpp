//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OCIO

#include <MaterialXGenShader/Nodes/OcioNode.h>
#include <MaterialXGenShader/OcioColorManagementSystem.h>

#include <MaterialXCore/Interface.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <cstring>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// OCIO implementation name suffixes (IMPL_MXOCIO_<function>_color3|color4) and OSL temp naming.
constexpr const char IMPL_SUFFIX_COLOR3[] = "_color3";
constexpr const char IMPL_SUFFIX_COLOR4[] = "_color4";
constexpr size_t IMPL_SUFFIX_COLOR3_LEN = sizeof(IMPL_SUFFIX_COLOR3) - 1;
constexpr size_t IMPL_SUFFIX_COLOR4_LEN = sizeof(IMPL_SUFFIX_COLOR4) - 1;
constexpr const char COLOR4_SUFFIX[] = "_color4_temp";

} // anonymous namespace

ShaderNodeImplPtr OcioNode::create()
{
    return std::make_shared<OcioNode>();
}

void OcioNode::initialize(const InterfaceElement& element, GenContext& context)
{
    ShaderNodeImpl::initialize(element, context);

    // Single function shared between color3 and color4 nodes, use a custom hash with only the function name.
    _hash = std::hash<string>{}(getFunctionName());
}

void OcioNode::emitFunctionDefinition(
    const ShaderNode& /*node*/,
    GenContext& context,
    ShaderStage& stage) const
{
    if (stage.getName() == Stage::PIXEL)
    {
        auto ocioManager = std::dynamic_pointer_cast<OcioColorManagementSystem>(context.getShaderGenerator().getColorManagementSystem());

        if (context.getShaderGenerator().getTarget() == "genosl")
        {
            const ShaderGenerator& shadergen = context.getShaderGenerator();
            shadergen.emitLibraryInclude("stdlib/genosl/lib/vector4_extra_ops.osl", context, stage);
            shadergen.emitLineBreak(stage);
        }

        stage.addString(ocioManager->getGpuProcessorCode(getName(), getFunctionName()));
        stage.endLine(false);
    }
}

void OcioNode::emitFunctionCall(
    const ShaderNode& node,
    GenContext& context,
    ShaderStage& stage) const
{
    if (stage.getName() == Stage::PIXEL)
    {
        auto functionName = getFunctionName();

        // TODO: Adjust syntax for other languages.
        // TODO: Handle LUT samplers.
        const bool isColor3 = getName().back() == '3';

        const auto& shadergen = context.getShaderGenerator();
        shadergen.emitLineBegin(stage);

        const auto* output = node.getOutput();
        const auto* colorInput = node.getInput(0);

        if (context.getShaderGenerator().getTarget() == "genosl")
        {
            // For OSL, since swizzling the output of a function is not allowed, we need:
            // Function call for color4: color4 res = func(in);
            // Function call for color3:
            //    color4 res_color4 = func(color4(in, 1.0));
            //    color res = res_color4.rgb;
            if (isColor3)
            {
                shadergen.emitString("color4 " + output->getVariable() + COLOR4_SUFFIX + " = ", stage);
                shadergen.emitString(functionName + "(color4(", stage);
                shadergen.emitInput(colorInput, context, stage);
                shadergen.emitString(", 1.0))", stage);
                shadergen.emitLineEnd(stage);
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(output, true, false, context, stage);
                shadergen.emitString(" = " + output->getVariable() + COLOR4_SUFFIX + ".rgb", stage);
                shadergen.emitLineEnd(stage);
            }
            else
            {
                shadergen.emitOutput(output, true, false, context, stage);
                shadergen.emitString(" = ", stage);
                shadergen.emitString(functionName + "(", stage);
                shadergen.emitInput(colorInput, context, stage);
                shadergen.emitString(")", stage);
                shadergen.emitLineEnd(stage);
            }
        }
        else
        {
            // The OCIO function uses a vec4 parameter, so:
            // Function call for color4: vec4 res = func(in);
            // Function call for color3: vec3 res = func(vec4(in, 1.0)).rgb;
            shadergen.emitOutput(output, true, false, context, stage);
            shadergen.emitString(" = ", stage);

            shadergen.emitString(functionName + "(", stage);
            if (isColor3)
            {
                if (context.getShaderGenerator().getTarget() == "genglsl")
                {
                    shadergen.emitString("vec4(", stage);
                }
                else if (context.getShaderGenerator().getTarget() == "genmsl")
                {
                    shadergen.emitString("float4(", stage);
                }
            }
            shadergen.emitInput(colorInput, context, stage);
            if (isColor3)
            {
                shadergen.emitString(", 1.0)", stage);
            }

            shadergen.emitString(")", stage);

            if (isColor3)
            {
                shadergen.emitString(".rgb", stage);
            }
            shadergen.emitLineEnd(stage);
        }
    }
}

string OcioNode::getFunctionNameFromImplName(const string& implName)
{
    const size_t startPos = OcioColorManagementSystem::IMPL_PREFIX.size();
    if (implName.size() <= startPos)
    {
        return implName;
    }
    // Same length trim as legacy getFunctionName() (_color3 / _color4 suffix).
    const size_t suffixLen = implName.size() > startPos + IMPL_SUFFIX_COLOR4_LEN &&
                                     implName.compare(implName.size() - IMPL_SUFFIX_COLOR4_LEN, IMPL_SUFFIX_COLOR4_LEN,
                                                      IMPL_SUFFIX_COLOR4) == 0
                                 ? IMPL_SUFFIX_COLOR4_LEN
                                 : IMPL_SUFFIX_COLOR3_LEN;
    const size_t length = implName.size() - suffixLen - startPos;
    return implName.substr(startPos, length);
}

string OcioNode::getFunctionName() const
{
    return getFunctionNameFromImplName(getName());
}

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO
