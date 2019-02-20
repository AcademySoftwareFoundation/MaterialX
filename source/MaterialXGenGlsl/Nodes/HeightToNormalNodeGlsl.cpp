#include <MaterialXGenGlsl/Nodes/HeightToNormalNodeGlsl.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

HeightToNormalNodeGlsl::HeightToNormalNodeGlsl()
    : ConvolutionNode()
{
    _sampleCount = 9;
    _sampleSizeFunctionUV.assign("mx_compute_sample_size_uv");
    _filterFunctionName.assign("mx_normal_from_samples_sobel");
}

ShaderNodeImplPtr HeightToNormalNodeGlsl::create()
{
    return std::shared_ptr<HeightToNormalNodeGlsl>(new HeightToNormalNodeGlsl());
}

void HeightToNormalNodeGlsl::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings) const
{
    // Build a 3x3 grid of samples that are offset by the provided sample size
    offsetStrings.clear();
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

void HeightToNormalNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const
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
    emitInputSamplesUV(stage, node, shadergen, context, sampleStrings);

    const ShaderOutput* output = node.getOutput();

    // Emit code to evaluate samples.
    //
    string sampleName(output->variable + "_samples");
    shadergen.emitLine(stage, "float " + sampleName + "[" + std::to_string(_sampleCount) + "]");
    for (unsigned int i = 0; i < _sampleCount; i++)
    {
        shadergen.emitLine(stage, sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
    }
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, output, true, false);
    shadergen.emitString(stage, " = " + _filterFunctionName);
    shadergen.emitString(stage, "(" + sampleName + ", ");
    shadergen.emitInput(stage, context, scaleInput);
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
