//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OCIO

#include <MaterialXGenShader/Nodes/OpenColorIONode.h>
#include <MaterialXGenShader/OpenColorIOManagementSystem.h>

#include <MaterialXCore/Interface.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorTypes.h>

#include <cstring>
#include <functional>
#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace
{
// Internal OCIO strings:
constexpr const char OCIO_COLOR3[] = "color3";
constexpr const char COLOR4_SUFFIX[] = "_color4_temp";

// Lengths where needed:
constexpr auto OCIO_COLOR3_LEN = sizeof(OCIO_COLOR3) / sizeof(OCIO_COLOR3[0]);

} // namespace

ShaderNodeImplPtr OpenColorIONode::create()
{
    return std::make_shared<OpenColorIONode>();
}

void OpenColorIONode::initialize(const InterfaceElement& element, GenContext& context)
{
    ShaderNodeImpl::initialize(element, context);

    // Single function shared between color3 and color4 nodes, use a custom hash with only the function name.
    _hash = std::hash<string>{}(getFunctionName());
}

void OpenColorIONode::emitFunctionDefinition(
    const ShaderNode& /*node*/,
    GenContext& context,
    ShaderStage& stage) const
{
    if (stage.getName() == Stage::PIXEL)
    {
        auto ocioManager = std::dynamic_pointer_cast<OpenColorIOManagementSystem>(context.getShaderGenerator().getColorManagementSystem());

        auto gpuProcessor = ocioManager->getGpuProcessor(getName());
        OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();

        // TODO: Extend to essl and MDL and possibly SLang.
        bool isOSL = false;
        if (context.getShaderGenerator().getTarget() == "genglsl")
        {
            shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
        }
        else if (context.getShaderGenerator().getTarget() == "genmsl")
        {
            shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_MSL_2_0);
        }
        else if (context.getShaderGenerator().getTarget() == "genosl")
        {
            shaderDesc->setLanguage(OCIO::LANGUAGE_OSL_1);
            isOSL = true;
        }

        auto functionName = getFunctionName();

        shaderDesc->setFunctionName(functionName.c_str());

        gpuProcessor->extractGpuShaderInfo(shaderDesc);

        string shaderText = shaderDesc->getShaderText();
        
        // For OSL, we need to extract the function from the shader OCIO creates.
        if (isOSL)
        {
            const ShaderGenerator& shadergen = context.getShaderGenerator();
            shadergen.emitLibraryInclude("stdlib/genosl/lib/vector4_extra_ops.osl", context, stage);
            shadergen.emitLineBreak(stage);
            auto startpos = shaderText.find(string{"color4 "} + shaderDesc->getFunctionName());
            if (startpos != string::npos)
            {
                auto endpos = shaderText.find(string{"outColor = "} + shaderDesc->getFunctionName(), startpos);
                if (endpos != string::npos)
                {
                    shaderText = shaderText.substr(startpos, endpos - startpos);
                }
            }
        }
        
        stage.addString(shaderText);
        stage.endLine(false);
    }
}

void OpenColorIONode::emitFunctionCall(
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

string OpenColorIONode::getFunctionName() const
{
    auto name = getName();

    // Strip _color3 and _color4 suffixes and impl prefix:
    size_t startPos = OpenColorIOManagementSystem::IMPL_PREFIX.size();
    size_t length = name.size() - OCIO_COLOR3_LEN - 1 - startPos;

    return name.substr(startPos, length);
}

MATERIALX_NAMESPACE_END

#endif
