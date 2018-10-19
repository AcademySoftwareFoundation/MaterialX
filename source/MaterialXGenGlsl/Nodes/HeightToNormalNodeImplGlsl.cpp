#include <MaterialXGenGlsl/Nodes/HeightToNormalNodeImplGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

HeightToNormalNodeImplGlsl::HeightToNormalNodeImplGlsl()
    : ParentClass()
{
    _sampleCount = 9;
    _sampleSizeFunctionUV.assign("sx_compute_sample_size_uv");
    _filterFunctionName.assign("sx_normal_from_samples_sobel");
}

ShaderNodeImplPtr HeightToNormalNodeImplGlsl::create()
{
    return std::shared_ptr<HeightToNormalNodeImplGlsl>(new HeightToNormalNodeImplGlsl());
}

void HeightToNormalNodeImplGlsl::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings)
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

bool HeightToNormalNodeImplGlsl::acceptsInputType(const TypeDesc* type)
{
    // Only support inputs which are float scalar
    return (type == Type::FLOAT && type->isScalar());
}

void HeightToNormalNodeImplGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    const ShaderInput* inInput = node.getInput("in");
    const ShaderInput* scaleInput = node.getInput("scale");

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
