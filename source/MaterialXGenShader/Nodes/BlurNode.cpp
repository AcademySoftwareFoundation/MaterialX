#include <MaterialXGenShader/Nodes/BlurNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <cmath>

namespace MaterialX
{

namespace
{
    /// Name of function to compute sample size in uv space. Takes uv, filter size, and filter offset
    /// as input, and return a 2 channel vector as output
    const string sampleSizeFunctionUV = "mx_compute_sample_size_uv";

    const float filterSize = 2.0;
    const float filterOffset = 0.0;
}

const string BlurNode::BOX_FILTER = "box";
const string BlurNode::GAUSSIAN_FILTER = "gaussian";
const string BlurNode::BOX_WEIGHTS_VARIABLE = "c_box_filter_weights";
const string BlurNode::GAUSSIAN_WEIGHTS_VARIABLE = "c_gaussian_filter_weights";
const string BlurNode::IN_STRING = "in";
const string BlurNode::FILTER_TYPE_STRING = "filtertype";
const string BlurNode::FILTER_SIZE_STRING = "size";

BlurNode::BlurNode()
    : ConvolutionNode()
{
}

ShaderNodeImplPtr BlurNode::create()
{
    return std::shared_ptr<BlurNode>(new BlurNode());
}

void BlurNode::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                          unsigned int filterWidth, StringVec& offsetStrings) const
{
    int w = static_cast<int>(filterWidth) / 2;
    // Build a NxN grid of samples that are offset by the provided sample size
    for (int row = -w; row <= w; row++)
    {
        for (int col = -w; col <= w; col++)
        {
            offsetStrings.push_back(" + " + sampleSizeName + " * " + offsetTypeString + "(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
        }
    }
}

bool BlurNode::acceptsInputType(const TypeDesc* type) const
{
    // Float 1-4 is acceptable as input
    return ((type == Type::FLOAT && type->isScalar()) ||
        type->isFloat2() || type->isFloat3() || type->isFloat4());
}

void BlurNode::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const ShaderInput* inInput = node.getInput(IN_STRING);

    // Get input type name string
    const string& inputTypeString = acceptsInputType(inInput->getType()) ?
        shadergen.getSyntax()->getTypeName(inInput->getType()) : EMPTY_STRING;

    const ShaderInput* filterTypeInput = node.getInput(FILTER_TYPE_STRING);
    if (!inInput || !filterTypeInput || inputTypeString.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
    }

    // Compute width of filter. Default is 1 which just means one 1x1 upstream samples
    const ShaderInput* sizeInput = node.getInput(FILTER_SIZE_STRING);
    unsigned int filterWidth = 1;
    unsigned int arrayOffset = 0;
    if (sizeInput)
    {
        float sizeInputValue = sizeInput->getValue()->asA<float>();
        if (sizeInputValue > 0.0f)
        {
            if (sizeInputValue <= 0.333f)
            {
                filterWidth = 3;
                arrayOffset = 1;
            }
            else if (sizeInputValue <= 0.666f)
            {
                filterWidth = 5;
                arrayOffset = 10;
            }
            else
            {
                filterWidth = 7;
                arrayOffset = 35;
            }
        }
    }

    // Sample count is square of filter size
    const unsigned int sampleCount = filterWidth*filterWidth;

    // Check for type of filter to apply
    // Default to box filter
    //
    string weightArrayVariable = BOX_WEIGHTS_VARIABLE;
    if (sampleCount > 1)
    {
        if (filterTypeInput->getValue())
        {
            // Use Gaussian filter.
            if (filterTypeInput->getValue()->getValueString() == GAUSSIAN_FILTER)
            {
                weightArrayVariable = GAUSSIAN_WEIGHTS_VARIABLE;
            }
        }
    }

    // Emit samples
    // Note: The maximum sample count MX_MAX_SAMPLE_COUNT is defined in the shader code and 
    // is assumed to be 49 (7x7 kernel). If this changes the filter size logic here 
    // needs to be adjusted.
    //
    StringVec sampleStrings;
    emitInputSamplesUV(stage, node, shadergen, context, sampleCount, filterWidth, 
        filterSize, filterOffset, sampleSizeFunctionUV, sampleStrings);

    // There should always be at least 1 sample
    if (sampleStrings.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' cannot compute upstream samples");
    }

    const ShaderOutput* output = node.getOutput();

    if (sampleCount > 1)
    {
        const string MX_MAX_SAMPLE_COUNT_STRING("MX_MAX_SAMPLE_COUNT");
        const string MX_WEIGHT_ARRAY_SIZE_STRING("MX_WEIGHT_ARRAY_SIZE");
        const string MX_CONVOLUTION_PREFIX_STRING("mx_convolution_");
        const string SAMPLES_POSTFIX_STRING("_samples");
        const string WEIGHT_POSTFIX_STRING("_weights");

        // Set up sample array
        string sampleName(output->getVariable() + SAMPLES_POSTFIX_STRING);
        shadergen.emitLine(stage, inputTypeString + " " + sampleName + "[" + MX_MAX_SAMPLE_COUNT_STRING + "]");
        for (unsigned int i = 0; i < sampleCount; i++)
        {
            shadergen.emitLine(stage, sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
        }

        // Emit code to evaluate using input sample and weight arrays. 
        // The function to call depends on input type.
        //
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, output, true, false);
        shadergen.emitLineEnd(stage);

        shadergen.emitLineBegin(stage);
        shadergen.emitString(stage, "if (");
        // If strings are support compare against string input,
        // other use int compare
        if (shadergen.getSyntax()->typeSupported(Type::STRING))
        {
            shadergen.emitInput(stage, context, filterTypeInput);
            shadergen.emitString(stage, " == \"" + GAUSSIAN_FILTER + "\")");
        }
        else
        {
            shadergen.emitInput(stage, context, filterTypeInput);
            shadergen.emitString(stage, " == 1)");
        }
        shadergen.emitLineEnd(stage, false);

        shadergen.emitScopeBegin(stage);
        {
            string filterFunctionName = MX_CONVOLUTION_PREFIX_STRING + inputTypeString;
            shadergen.emitLineBegin(stage);
            shadergen.emitString(stage, output->getVariable());
            shadergen.emitString(stage, " = " + filterFunctionName);
            shadergen.emitString(stage, "(" + sampleName + ", " +
                GAUSSIAN_WEIGHTS_VARIABLE + ", " +
                std::to_string(arrayOffset) + ", " +
                std::to_string(sampleCount) +
                ")");
            shadergen.emitLineEnd(stage);
        }
        shadergen.emitScopeEnd(stage);
        shadergen.emitLine(stage, "else", false);
        shadergen.emitScopeBegin(stage);
        {
            string filterFunctionName = MX_CONVOLUTION_PREFIX_STRING + inputTypeString;
            shadergen.emitLineBegin(stage);
            shadergen.emitString(stage, output->getVariable());
            shadergen.emitString(stage, " = " + filterFunctionName);
            shadergen.emitString(stage, "(" + sampleName + ", " +
                BOX_WEIGHTS_VARIABLE + ", " +
                std::to_string(arrayOffset) + ", " +
                std::to_string(sampleCount) +
                ")");
            shadergen.emitLineEnd(stage);
        }
        shadergen.emitScopeEnd(stage);
    }
    else
    {
        // This is just a pass-through of the upstream sample if any,
        // or the constant value on the node.
        //
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, output, true, false);
        shadergen.emitString(stage, " = " + sampleStrings[0]);
        shadergen.emitLineEnd(stage);
    }
END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
