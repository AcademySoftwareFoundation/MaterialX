#include <MaterialXGenShader/Nodes/BlurNode.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <cmath>

namespace MaterialX
{

    string BlurNode::BOX_FILTER = "box";
    string BlurNode::GAUSSIAN_FILTER = "gaussian";
    string BlurNode::BOX_WEIGHTS_VARIABLE = "c_box_filter_weights";
    string BlurNode::GAUSSIAN_WEIGHTS_VARIABLE = "c_gaussian_filter_weights";

    BlurNode::BlurNode()
        : ParentClass()
        , _weightArrayVariable(BOX_WEIGHTS_VARIABLE)
        , _filterType(BOX_FILTER)
        , _inputTypeString("float")
    {
        _filterSize = 2;
        _sampleCount = 1;
        _sampleSizeFunctionUV.assign("mx_compute_sample_size_uv");
    }

    ShaderNodeImplPtr BlurNode::create()
    {
        return std::shared_ptr<BlurNode>(new BlurNode());
    }

    void BlurNode::computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings)
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

    bool BlurNode::acceptsInputType(const TypeDesc* type)
    {
        // Float 1-4 is acceptable as input
        return ((type == Type::FLOAT && type->isScalar()) ||
            type->isFloat2() || type->isFloat3() || type->isFloat4());
    }

    void BlurNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
    {
        const string IN_STRING("in");
        const ShaderInput* inInput = node.getInput(IN_STRING);

        // Get intput type name string
        _inputTypeString.clear();
        if (acceptsInputType(inInput->type))
        {
            _inputTypeString = shadergen.getSyntax()->getTypeName(inInput->type);
        }

        const string FILTER_TYPE_STRING("filtertype");
        const ShaderInput* filterTypeInput = node.getInput(FILTER_TYPE_STRING);
        if (!inInput || !filterTypeInput || _inputTypeString.empty())
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
        }

        // Compute width of filter. Default is 1 which just means one 1x1 upstream samples
        const string FILTER_SIZE_STRING("size");
        const ShaderInput* sizeInput = node.getInput(FILTER_SIZE_STRING);
        _filterWidth = 1;
        unsigned int arrayOffset = 0;
        if (sizeInput)
        {
            float sizeInputValue = sizeInput->value->asA<float>();
            if (sizeInputValue > 0.0f)
            {
                if (sizeInputValue <= 0.333f)
                {
                    _filterWidth = 3;
                    arrayOffset = 1;
                }
                else if (sizeInputValue <= 0.666f)
                {
                    _filterWidth = 5;
                    arrayOffset = 10;
                }
                else
                {
                    _filterWidth = 7;
                    arrayOffset = 35;
                }
            }
        }

        // Sample count is square of filter size
        _sampleCount = _filterWidth*_filterWidth;

        // Check for type of filter to apply
        // Default to box filter
        //
        _filterType.clear();
        string weightArrayVariable;
        if (_sampleCount > 1)
        {
            if (filterTypeInput->value)
            {
                // Use Gaussian filter.
                if (filterTypeInput->value->getValueString() == GAUSSIAN_FILTER)
                {
                    _filterType = GAUSSIAN_FILTER;
                    weightArrayVariable = GAUSSIAN_WEIGHTS_VARIABLE;
                }
                else
                {
                    _filterType = BOX_FILTER;
                    weightArrayVariable = BOX_WEIGHTS_VARIABLE;
                }
            }
        }

        BEGIN_SHADER_STAGE(shader, Shader::PIXEL_STAGE)
        {
            // Emit samples
            // Note: The maximum sample count MX_MAX_SAMPLE_COUNT is defined in the shader code and 
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

            const ShaderOutput* output = node.getOutput();

            if (_sampleCount > 1)
            {
                const string MX_MAX_SAMPLE_COUNT_STRING("MX_MAX_SAMPLE_COUNT");
                const string MX_WEIGHT_ARRAY_SIZE_STRING("MX_WEIGHT_ARRAY_SIZE");
                const string MX_CONVOLUTION_PREFIX_STRING("mx_convolution_");
                const string SAMPLES_POSTFIX_STRING("_samples");
                const string WEIGHT_POSTFIX_STRING("_weights");

                // Set up sample array
                string sampleName(output->variable + SAMPLES_POSTFIX_STRING);
                shader.addLine(_inputTypeString + " " + sampleName + "[" + MX_MAX_SAMPLE_COUNT_STRING + "]");
                for (unsigned int i = 0; i < _sampleCount; i++)
                {
                    shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
                }

                // Emit code to evaluate using input sample and weight arrays. 
                // The function to call depends on input type.
                //
                shader.beginLine();
                shadergen.emitOutput(context, output, true, false, shader);
                shader.endLine();

                shader.beginLine();
                shader.addStr("if (");
                // If strings are support compare against string input,
                // other use int compare
                if (shadergen.getSyntax()->typeSupported(Type::STRING))
                {
                    shadergen.emitInput(context, filterTypeInput, shader);
                    shader.addStr(" == \"" + GAUSSIAN_FILTER + "\")");
                }
                else
                {
                    shadergen.emitInput(context, filterTypeInput, shader);
                    shader.addStr(" == 1)");
                }
                shader.endLine(false);

                shader.beginScope();
                {
                    string filterFunctionName = MX_CONVOLUTION_PREFIX_STRING + _inputTypeString;
                    shader.beginLine();
                    shader.addStr(output->variable);
                    shader.addStr(" = " + filterFunctionName);
                    shader.addStr("(" + sampleName + ", " +
                        GAUSSIAN_WEIGHTS_VARIABLE + ", " +
                        std::to_string(arrayOffset) + ", " +
                        std::to_string(_sampleCount) +
                        ")");
                    shader.endLine();
                }
                shader.endScope();
                shader.addLine("else");
                shader.beginScope();
                {
                    string filterFunctionName = MX_CONVOLUTION_PREFIX_STRING + _inputTypeString;
                    shader.beginLine();
                    shader.addStr(output->variable);
                    shader.addStr(" = " + filterFunctionName);
                    shader.addStr("(" + sampleName + ", " +
                        BOX_WEIGHTS_VARIABLE + ", " +
                        std::to_string(arrayOffset) + ", " +
                        std::to_string(_sampleCount) +
                        ")");
                    shader.endLine();
                }
                shader.endScope();
            }
            else
            {
                // This is just a pass-through of the upstream sample if any,
                // or the constant value on the node.
                //
                shader.beginLine();
                shadergen.emitOutput(context, output, true, false, shader);
                shader.addStr(" = " + sampleStrings[0]);
                shader.endLine();
            }
        }
        END_SHADER_STAGE(shader, Shader::PIXEL_STAGE)
    }

} // namespace MaterialX
