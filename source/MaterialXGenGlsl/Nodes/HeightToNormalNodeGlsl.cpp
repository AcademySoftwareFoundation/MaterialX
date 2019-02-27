#include <MaterialXGenGlsl/Nodes/HeightToNormalNodeGlsl.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

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
}

HeightToNormalNodeGlsl::HeightToNormalNodeGlsl()
    : ConvolutionNode()
{
}

ShaderNodeImplPtr HeightToNormalNodeGlsl::create()
{
    return std::shared_ptr<HeightToNormalNodeGlsl>(new HeightToNormalNodeGlsl());
}

void HeightToNormalNodeGlsl::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, 
                                                        unsigned int, StringVec& offsetStrings) const
{
    // Build a 3x3 grid of samples that are offset by the provided sample size
    for (int row = -1; row <= 1; row++)
    {
        for (int col = -1; col <= 1; col++)
        {
            offsetStrings.push_back(" + " + sampleSizeName + " * " + offsetTypeString +  "(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
        }
    }
}

bool HeightToNormalNodeGlsl::acceptsInputType(const TypeDesc* type) const
{
    // Only support inputs which are float scalar
    return (type == Type::FLOAT && type->isScalar());
}

void HeightToNormalNodeGlsl::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
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
    emitInputSamplesUV(stage, node, shadergen, context, sampleCount, filterWidth, 
        filterSize, filterOffset, sampleSizeFunctionUV, sampleStrings);

    const ShaderOutput* output = node.getOutput();

    // Emit code to evaluate samples.
    //
    string sampleName(output->getVariable() + "_samples");
    shadergen.emitLine(stage, "float " + sampleName + "[" + std::to_string(sampleCount) + "]");
    for (unsigned int i = 0; i < sampleCount; i++)
    {
        shadergen.emitLine(stage, sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
    }
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, output, true, false);
    shadergen.emitString(stage, " = " + filterFunctionName);
    shadergen.emitString(stage, "(" + sampleName + ", ");
    shadergen.emitInput(stage, context, scaleInput);
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
