#include <MaterialXGenShader/Nodes/ConvolutionNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace
{
    const std::vector<float> GAUSSIAN_WEIGHT_ARRAY = {
        // 1x1
        1.0,
        // 3x3
        0.077847f,	0.123317f,	0.077847f,
        0.123317f,	0.195346f,	0.123317f,
        0.077847f,	0.123317f,	0.077847f,
        // 5x5
        0.003765f,	0.015019f,	0.023792f,	0.015019f,	0.003765f,
        0.015019f,	0.059912f,	0.094907f,	0.059912f,	0.015019f,
        0.023792f,	0.094907f,	0.150342f,	0.094907f,	0.023792f,
        0.015019f,	0.059912f,	0.094907f,	0.059912f,	0.015019f,
        0.003765f,	0.015019f,	0.023792f,	0.015019f,	0.003765f,
        // 7x7
        0.000036f,	0.000363f,	0.001446f,	0.002291f,	0.001446f,	0.000363f,	0.000036f,
        0.000363f,	0.003676f,	0.014662f,	0.023226f,	0.014662f,	0.003676f,	0.000363f,
        0.001446f,	0.014662f,	0.058488f,	0.092651f,	0.058488f,	0.014662f,	0.001446f,
        0.002291f,	0.023226f,	0.092651f,	0.146768f,	0.092651f,	0.023226f,	0.002291f,
        0.001446f,	0.014662f,	0.058488f,	0.092651f,	0.058488f,	0.014662f,	0.001446f,
        0.000363f,	0.003676f,	0.014662f,	0.023226f,	0.014662f,	0.003676f,	0.000363f,
        0.000036f,	0.000363f,	0.001446f,	0.002291f,	0.001446f,	0.000363f,	0.000036f
    };
}

namespace MaterialX
{
ConvolutionNode::ConvolutionNode()
    : _sampleCount(1)
    , _filterSize(1.0f)
    , _filterOffset(0.0f)
{
}

void ConvolutionNode::createVariables(const ShaderNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader)
{
    // Create constant for box weights
    const float boxWeight3x3 = 1.0f / 9.0f;
    const float boxWeight5x5 = 1.0f / 25.0f;
    const float boxWeight7x7 = 1.0f / 49.0f;
    vector<float> boxWeightArray;
    boxWeightArray.push_back(1.0f);
    for (unsigned int i = 0; i < 9; i++)
    {
        boxWeightArray.push_back(boxWeight3x3);
    }
    for (unsigned int i = 0; i < 25; i++)
    {
        boxWeightArray.push_back(boxWeight5x5);
    }
    for (unsigned int i = 0; i < 49; i++)
    {
        boxWeightArray.push_back(boxWeight7x7);
    }
    shader.createConstant(Shader::PIXEL_STAGE, 
        Type::FLOATARRAY, "c_box_filter_weights", EMPTY_STRING, Value::createValue<vector<float>>(boxWeightArray));

    // Create constant for Gaussian weights
 
    shader.createConstant(Shader::PIXEL_STAGE,
        Type::FLOATARRAY, "c_gaussian_filter_weights", EMPTY_STRING, Value::createValue<vector<float>>(GAUSSIAN_WEIGHT_ARRAY));
}

void ConvolutionNode::emitInputSamplesUV(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader, StringVec& sampleStrings)
{
    sampleStrings.clear();

    // Check for an upstream node to sample
    const ShaderInput* inInput = node.getInput("in");
    ShaderOutput* inConnection = inInput ? inInput->connection : nullptr;

    if (inConnection && inConnection->type && acceptsInputType(inConnection->type))
    {
        ShaderNode* upstreamNode = inConnection->node;
        if (upstreamNode && upstreamNode->hasClassification(ShaderNode::Classification::SAMPLE2D))
        {
            ShaderNodeImpl *impl = upstreamNode->getImplementation();
            if (impl)
            {
                ShaderOutput* upstreamOutput = upstreamNode->getOutput();
                if (upstreamOutput)
                {
                    string outputName = upstreamOutput->name;

                    // Find out which input needs to be sampled multiple times
                    // If the sample count is 1 then the sample code has already been emitted
                    ShaderInput* samplingInput = (_sampleCount > 1) ? upstreamNode->getSamplingInput() : nullptr;

                    // TODO: For now we only support uv space sampling
                    if (samplingInput && samplingInput->type != Type::VECTOR2)
                    {
                        samplingInput = nullptr;
                    }

                    if (samplingInput)
                    {
                        // This is not exposed. Assume a filter size of 1 with no offset

                        // Emit code to compute sample size
                        //
                        string sampleInputValue;
                        shadergen.getInput(context, samplingInput, sampleInputValue);

                        const string sampleSizeName(node.getOutput()->name + "_sample_size");
                        const string vec2TypeString = shadergen.getSyntax()->getTypeName(Type::VECTOR2);
                        string sampleCall(vec2TypeString + " " + sampleSizeName + " = " +
                            _sampleSizeFunctionUV + "(" +
                            sampleInputValue + "," +
                            std::to_string(_filterSize) + "," +
                            std::to_string(_filterOffset) + ");"
                        );
                        shader.addLine(sampleCall);

                        // Build the sample offset strings. This is dependent on
                        // the derived class to determine where samples are located
                        // and to generate the strings required to offset from the center
                        // sample. The sample size is passed over.
                        //
                        StringVec inputVec2Suffix;
                        computeSampleOffsetStrings(sampleSizeName, vec2TypeString, inputVec2Suffix);

                        // Emit outputs for sample input 
                        for (unsigned int i = 0; i < _sampleCount; i++)
                        {
                            // Add an input name suffix. 
                            context.addInputSuffix(samplingInput, inputVec2Suffix[i]);

                            // Add a output name suffix for the emit call
                            string outputSuffix("_" + node.getOutput()->name + std::to_string(i));
                            context.addOutputSuffix(upstreamOutput, outputSuffix);

                            impl->emitFunctionCall(*upstreamNode, context, shadergen, shader);

                            // Remove suffixes
                            context.removeInputSuffix(samplingInput);
                            context.removeOutputSuffix(upstreamOutput);

                            // Keep track of the output name with the suffix
                            sampleStrings.push_back(outputName + outputSuffix);
                        }
                    }
                    else
                    {
                        // Use the same input for all samples.
                        // Note: This does not recomputed the output, but instead it reuses
                        // the output variable.
                        for (unsigned int i = 0; i < _sampleCount; i++)
                        {
                            // Call the unmodified function
                            sampleStrings.push_back(outputName);
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (!inInput->value)
        {
            throw ExceptionShaderGenError("No connection or value found on node: '" + node.getName() + "'");
        }
    }

    // Build a set of samples with constant values
    if (sampleStrings.empty())
    {

        if (inInput->type->isScalar())
        {
            string scalarValueString = inInput->value->getValueString();
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                sampleStrings.push_back(scalarValueString);
            }
        }
        else
        {
            string typeString = shadergen.getSyntax()->getTypeName(inInput->type);
            string inValueString = typeString + "(" + inInput->value->getValueString() + ")";
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                sampleStrings.push_back(inValueString);
            }
        }
    }
} 

} // namespace MaterialX
