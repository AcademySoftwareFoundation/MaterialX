#include <MaterialXGenGlsl/Nodes/HeightToNormalGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

HeightToNormalGlsl::HeightToNormalGlsl()
    : ParentClass()
{
    _sampleCount = 9;
    _sampleSizeFunctionUV.assign("sx_compute_sample_size_uv");
    _filterFunctionName.assign("sx_normal_from_samples_sobel");
}

SgImplementationPtr HeightToNormalGlsl::create()
{
    return std::shared_ptr<HeightToNormalGlsl>(new HeightToNormalGlsl());
}

void HeightToNormalGlsl::computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings)
{
    // Build a 3x3 grid of samples that are offset by the provided sample size
    offsetStrings.clear();
    for (int row = -1; row <= 1; row++)
    {
        for (int col = -1; col <= 1; col++)
        {
            offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
        }
    }
}

bool HeightToNormalGlsl::acceptsInputType(const TypeDesc* type)
{
    // Only support inputs which are float scalar
    return (type == Type::FLOAT && type->isScalar());
}

void HeightToNormalGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    const SgInput* inInput = node.getInput("in");
    const SgInput* scaleInput = node.getInput("scale");

    if (!inInput || !scaleInput)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
    }

    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    {
        // Create the input "samples". This means to emit the calls to 
        // compute the sames and return a set of strings containaing
        // the variables to assign to the sample grid.
        //  
        StringVec sampleStrings;
        emitInputSamplesUV(node, context, shadergen, shader, sampleStrings);

        // Emit code to evaluate samples.
        //
        string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

        string sampleName(node.getOutput()->name + "_samples");
        shader.addLine("float " + sampleName + "[" + std::to_string(_sampleCount) + "]");
        for (unsigned int i = 0; i < _sampleCount; i++)
        {
            shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
        }
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = " + _filterFunctionName);
        shader.addStr("(" + sampleName + ", " + scaleValueString + ")");
        shader.endLine();
    }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
