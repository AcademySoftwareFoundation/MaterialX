#include <MaterialXGenGlsl/Nodes/BlurGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{
BlurGlsl::BlurGlsl()
    : ParentClass()
    , _filterFunctionName("sx_blur_box_float")
    , _filterType("box")
    , _inputTypeString("float")
{
    _filterSize = 2;
    _sampleCount = 9; // For box
    _sampleSizeFunctionUV.assign("sx_compute_sample_size_uv");
}

SgImplementationPtr BlurGlsl::create()
{
    return std::shared_ptr<BlurGlsl>(new BlurGlsl());
}

void BlurGlsl::computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings)
{
    // Build a 3x3 grid of samples that are offset by the provided sample size
    offsetStrings.clear();
    if (_filterType == "box")
    {
        for (int row = -1; row <= 1; row++)
        {
            for (int col = -1; col <= 1; col++)
            {
                offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
            }
        }
    }
    // TODO: Add Gaussian support
    else
    {
        for (int row = -1; row <= 1; row++)
        {
            for (int col = -1; col <= 1; col++)
            {
                offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
            }
        }
    }
}

bool BlurGlsl::acceptsInputType(const TypeDesc* type)
{
    // Float 1-4 is acceptable as input
    return ((type == Type::FLOAT && type->isScalar()) ||
            type->isFloat2() || type->isFloat3() || type->isFloat4());
}

void BlurGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    const SgInput* inInput = node.getInput("in");

    // Get intput type name string
    _inputTypeString.clear();
    if (acceptsInputType(inInput->type))
    {
        _inputTypeString = shadergen.getSyntax()->getTypeName(inInput->type);
    }

    const SgInput* filterTypeInput = node.getInput("filtertype");
    if (!inInput || !filterTypeInput || _inputTypeString.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
    }

    // Check for type of filter to apply
    //
    const string BOX_FILTER("box");
    const string GAUSSIAN_FILTER("gaussian");
    if (filterTypeInput->value)
    {
        if (filterTypeInput->value->getValueString() == GAUSSIAN_FILTER)
        {
            // TODO: Add in Gaussian support
            _filterType = GAUSSIAN_FILTER;
            _sampleCount = 9;
        }
        else 
        {
            _filterType = BOX_FILTER;
            _sampleCount = 9;
        }
    }

    // Check size of filter. TO-DO.
    //const SgInput* sizeInput = node.getInput("size");     

    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    {
        // Emit samples
        StringVec sampleStrings;
        emitInputSamplesUV(node, context, shadergen, shader, sampleStrings);

        string sampleName(node.getOutput()->name + "_samples");
        shader.addLine(_inputTypeString + " " + sampleName + "[" + std::to_string(_sampleCount) + "]");
        for (unsigned int i = 0; i < _sampleCount; i++)
        {
            shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
        }

        // Emit code to evaluate samples. Function to call depends on input type.
        //
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        _filterFunctionName = "sx_blur_" + _filterType  + "_" + _inputTypeString;
        shader.addStr(" = " + _filterFunctionName);
        shader.addStr("(" + sampleName + ")");
        shader.endLine();
    }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
