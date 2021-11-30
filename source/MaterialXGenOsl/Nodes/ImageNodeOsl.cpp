//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/Nodes/ImageNodeOsl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

namespace MaterialX
{

    ShaderNodeImplPtr ImageNodeOsl::create()
{
    return std::make_shared<ImageNodeOsl>();
}


void ImageNodeOsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    //const Syntax& syntax = shadergen.getSyntax();
    
    // Set color space override value if any
    string filename;
    string colorSpace;
    string variable;
    for (ShaderInput* input : node.getInputs())
    {
        if (input && input->getValue() && input->getType() == Type::FILENAME)
        {
            ShaderOutput* upstreamOutput = input->getConnection();
            if (upstreamOutput)
            {
                colorSpace = upstreamOutput->getColorspace();
                if (upstreamOutput->getValue())
                {
                    filename = upstreamOutput->getValue()->getValueString();
                }
                variable = upstreamOutput->getVariable();
                std::cout << "Emit output port: " << filename << ". colorspace: " << colorSpace << std::endl;
            }
            else // if (colorSpace.empty())
            { 
                colorSpace = input->getColorspace();
                filename = input->getValue()->getValueString();
                variable = input->getVariable();
                std::cout << "Emit input port: " << filename << ". colorspace: " << colorSpace << std::endl;
            }
            break;
        }
    }
    if (!filename.empty())
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitString(variable + "." + "filename" + " = \"" + filename + "\"", stage);
        shadergen.emitLineEnd(stage);
        shadergen.emitLineBegin(stage);
        shadergen.emitString(variable + "." + "colorspace" + " = \"" + colorSpace + "\"", stage);
        shadergen.emitLineEnd(stage);
    }

    // Declare the output variables.
    emitOutputVariables(node, context, stage);

    shadergen.emitLineBegin(stage);
    string delim = "";

    // Emit function name.
    shadergen.emitString(_functionName + "(", stage);

    // Emit all inputs on the node.
    for (ShaderInput* input : node.getInputs())
    {
        shadergen.emitString(delim, stage);
        shadergen.emitInput(input, context, stage);
        delim = ", ";
    }

    // Emit node outputs.
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitString(delim, stage);
        shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
        delim = ", ";
    }

    // End function call
    shadergen.emitString(")", stage);
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
