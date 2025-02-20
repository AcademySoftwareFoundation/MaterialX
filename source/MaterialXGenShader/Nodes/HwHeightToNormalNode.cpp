//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/HwHeightToNormalNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

/// Name of filter function to call to compute normals from input samples
const string filterFunctionName = "mx_normal_from_samples_sobel";

/// Name of function to compute sample size in uv space. Takes uv, filter size, and filter offset
/// as input, and return a 2 channel vector as output
const string sampleSizeFunctionUV = "mx_compute_sample_size_uv";

const unsigned int sampleCount = 9;
const unsigned int filterWidth = 3;
const float filterSize = 1.0;
const float filterOffset = 0.0;

} // namespace

ShaderNodeImplPtr HwHeightToNormalNode::create(const string& samplingIncludeFilename)
{
    return std::make_shared<HwHeightToNormalNode>(samplingIncludeFilename);
}

void HwHeightToNormalNode::createVariables(const ShaderNode&, GenContext&, Shader&) const
{
    // Default filter kernels from ConvolutionNode are not used by this derived class.
}

void HwHeightToNormalNode::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                                       unsigned int, StringVec& offsetStrings) const
{
    // Build a 3x3 grid of samples that are offset by the provided sample size
    for (int row = -1; row <= 1; row++)
    {
        for (int col = -1; col <= 1; col++)
        {
            offsetStrings.push_back(" + " + sampleSizeName + " * " + offsetTypeString + "(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
        }
    }
}

bool HwHeightToNormalNode::acceptsInputType(TypeDesc type) const
{
    // Only support inputs which are float scalar
    return (type == Type::FLOAT && type.isScalar());
}

void HwHeightToNormalNode::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit sampling functions
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLibraryInclude(_samplingIncludeFilename, context, stage);
        shadergen.emitLineBreak(stage);
    }
}

void HwHeightToNormalNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* inInput = node.getInput("in");
        const ShaderInput* scaleInput = node.getInput("scale");

        if (!inInput || !scaleInput)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
        }

        // Create the input "samples". This means to emit the calls to
        // compute the sames and return a set of strings containaing
        // the variables to assign to the sample grid.
        //
        StringVec sampleStrings;
        emitInputSamplesUV(node, sampleCount, filterWidth,
                           filterSize, filterOffset, sampleSizeFunctionUV,
                           context, stage, sampleStrings);

        const ShaderOutput* output = node.getOutput();

        // Emit code to evaluate samples.
        //
        string sampleName(output->getVariable() + "_samples");
        shadergen.emitLine("float " + sampleName + "[" + std::to_string(sampleCount) + "]", stage);
        for (unsigned int i = 0; i < sampleCount; i++)
        {
            shadergen.emitLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i], stage);
        }
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);
        shadergen.emitString(" = " + filterFunctionName, stage);
        shadergen.emitString("(" + sampleName + ", ", stage);
        shadergen.emitInput(scaleInput, context, stage);
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    }
}


MATERIALX_NAMESPACE_END
