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
    const Syntax& syntax = shadergen.getSyntax();
    
    // Set color space override value if any
    const ShaderInput* fileInput = nullptr;
    for (ShaderInput* input : node.getInputs())
    {
        if (input->getType() == Type::FILENAME &&
            !input->getConnection())
        {
            fileInput = input;
            break;
        }
    }
    if (fileInput)
    {
        string variable = fileInput->getVariable();
        string fileName = fileInput->getValue() ? syntax.getValue(fileInput->getType(), *fileInput->getValue()) : 
                                                  syntax.getDefaultValue(fileInput->getType());
        string colorspace = EMPTY_STRING;
        shadergen.emitString(variable + "." + "filename" + " = " + fileName, stage);
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
