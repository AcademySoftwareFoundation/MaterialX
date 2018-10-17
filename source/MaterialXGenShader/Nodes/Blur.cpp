#include <MaterialXGenShader/Nodes/Blur.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <cmath>

namespace MaterialX
{

string Blur::BOX_FILTER = "box";
string Blur::GAUSSIAN_FILTER = "gaussian";
string Blur::BOX_WEIGHT_FUNCTION = "sx_get_box_weights";
string Blur::GAUSSIAN_WEIGHT_FUNCTION = "sx_get_gaussian_weights";

Blur::Blur()
    : ParentClass()
    , _filterFunctionName(BOX_WEIGHT_FUNCTION)
    , _filterType(BOX_FILTER)
    , _inputTypeString("float")
{
    _filterSize = 2;
    _sampleCount = 1; 
    _sampleSizeFunctionUV.assign("sx_compute_sample_size_uv");
}

SgImplementationPtr Blur::create()
{
    return std::shared_ptr<Blur>(new Blur());
}

void Blur::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings)
{
    offsetStrings.clear();
 
    if (_sampleCount > 1)
    { 
        int w = static_cast<int>(_filterWidth) / 2;
        // Build a NxN grid of samples that are offset by the provided sample size
        for (int row = -w; row <= w; row++)
        {
            for (int col = -w; col <= w; col++)
            {
                offsetStrings.push_back(" + " + sampleSizeName + " * " + offsetTypeString + "(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
            }
        }
    }
    else
    {
        offsetStrings.push_back("");
    }
}

bool Blur::acceptsInputType(const TypeDesc* type)
{
    // Float 1-4 is acceptable as input
    return ((type == Type::FLOAT && type->isScalar()) ||
            type->isFloat2() || type->isFloat3() || type->isFloat4());
}

void Blur::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    const string IN_STRING("in");
    const SgInput* inInput = node.getInput(IN_STRING);

    // Get intput type name string
    _inputTypeString.clear();
    if (acceptsInputType(inInput->type))
    {
        _inputTypeString = shadergen.getSyntax()->getTypeName(inInput->type);
    }

    const string FILTER_TYPE_STRING("filtertype");
    const SgInput* filterTypeInput = node.getInput(FILTER_TYPE_STRING);
    if (!inInput || !filterTypeInput || _inputTypeString.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
    }

    // Compute width of filter. Default is 1 which just means one 1x1 upstream samples
    const string FILTER_SIZE_STRING("size");
    const SgInput* sizeInput = node.getInput(FILTER_SIZE_STRING);
    _filterWidth = 1;
    if (sizeInput)
    {
        float sizeInputValue = sizeInput->value->asA<float>();
        if (sizeInputValue > 0.0f)
        {
            if (sizeInputValue <= 0.333f)
            {
                _filterWidth = 3;
            }
            else if (sizeInputValue <= 0.666f)
            {
                _filterWidth = 5;
            }
            else
            {
                _filterWidth = 7;
            }
        }
    }

    // Sample count is square of filter size
    _sampleCount = _filterWidth*_filterWidth;

    // Check for type of filter to apply
    // Default to box filter
    //
    _filterType.clear();
    string weightFunction;
    if (_sampleCount > 1)
    {
        if (filterTypeInput->value)
        {
            // Use Gaussian filter.
            if (filterTypeInput->value->getValueString() == GAUSSIAN_FILTER)
            {
                _filterType = GAUSSIAN_FILTER;
                weightFunction = GAUSSIAN_WEIGHT_FUNCTION;
            }
            else
            {
                _filterType = BOX_FILTER;
                weightFunction = BOX_WEIGHT_FUNCTION;
            }
        }
    }

    BEGIN_SHADER_STAGE(shader, Shader::PIXEL_STAGE)
    {
        // Emit samples
        // Note: The maximum sample count SX_MAX_SAMPLE_COUNT is defined in the shader code and 
        // is assumed to be 49 (7x7 kernel). If this changes the filter size logic here 
        // needs to be adjusted.
        //
        StringVec sampleStrings;
        emitInputSamplesUV(node, context, shadergen, shader, sampleStrings);

        // There should always be at least 1 sample
        if (sampleStrings.empty())
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' cannot compute upstream samples");
        }

        if (_sampleCount > 1)
        {
            const string SX_MAX_SAMPLE_COUNT_STRING("SX_MAX_SAMPLE_COUNT");
            const string SX_CONVOLUTION_PREFIX_STRING("sx_convolution_");
            const string SAMPLES_POSTFIX_STRING("_samples");
            const string WEIGHT_POSTFIX_STRING("_weights");

            // Set up sample array
            string sampleName(node.getOutput()->name + SAMPLES_POSTFIX_STRING);
            shader.addLine(_inputTypeString + " " + sampleName + "[" + SX_MAX_SAMPLE_COUNT_STRING + "]");
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
            }

            // Set up weight array
            string weightName(node.getOutput()->name + WEIGHT_POSTFIX_STRING);
            shader.addLine("float " + weightName + "[" + SX_MAX_SAMPLE_COUNT_STRING + "]");
            shader.addLine(weightFunction + "(" + weightName + ", " + std::to_string(_filterWidth) + ")");

            // Emit code to evaluate using input sample and weight arrays. 
            // The function to call depends on input type.
            //
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(), true, false, shader);
            _filterFunctionName = SX_CONVOLUTION_PREFIX_STRING + _inputTypeString;
            shader.addStr(" = " + _filterFunctionName);
            shader.addStr("(" + sampleName + ", " +
                                weightName + ", " +
                                std::to_string(_sampleCount) +
                                ")");
            shader.endLine();
        }
        else
        {
            // This is just a pass-through of the upstream sample if any,
            // or the constant value on the node.
            //
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(), true, false, shader);
            shader.addStr(" = " + sampleStrings[0]);
            shader.endLine();
        }
    }
    END_SHADER_STAGE(shader, Shader::PIXEL_STAGE)
}

} // namespace MaterialX
