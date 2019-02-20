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

const string ConvolutionNode::SAMPLE2D_INPUT = "texcoord";
const string ConvolutionNode::SAMPLE3D_INPUT = "position";

ConvolutionNode::ConvolutionNode()
{
}

void ConvolutionNode::createVariables(Shader& shader, const ShaderNode&, ShaderGenerator&, GenContext&) const
{
    ShaderStage& stage = shader.getStage(MAIN_STAGE);
    VariableBlock& constants = stage.getConstantBlock();

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
    constants.add(Type::FLOATARRAY, "c_box_filter_weights", EMPTY_STRING, Value::createValue<vector<float>>(boxWeightArray));

    // Create constant for Gaussian weights
    constants.add(Type::FLOATARRAY, "c_gaussian_filter_weights", EMPTY_STRING, Value::createValue<vector<float>>(GAUSSIAN_WEIGHT_ARRAY));
}

/// Get input which is used for sampling. If there is none
/// then a null pointer is returned.
const ShaderInput* ConvolutionNode::getSamplingInput(const ShaderNode& node) const
{
    // Determine if this input can be sampled
    if (node.hasClassification(ShaderNode::Classification::SAMPLE2D))
    {
        const ShaderInput* input = node.getInput(SAMPLE2D_INPUT);
        return input->type == Type::VECTOR2 ? input : nullptr;
    }
    else if (node.hasClassification(ShaderNode::Classification::SAMPLE3D))
    {
        const ShaderInput* input = node.getInput(SAMPLE3D_INPUT);
        return input->type == Type::VECTOR3 ? input : nullptr;
    }
    return nullptr;
}

void ConvolutionNode::emitInputSamplesUV(ShaderStage& stage, const ShaderNode& node, 
                                         ShaderGenerator& shadergen, GenContext& context, 
                                         unsigned int sampleCount, unsigned int filterWidth, 
                                         float filterSize, float filterOffset,
                                         const string& sampleSizeFunctionUV, StringVec& sampleStrings) const
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
            const ShaderNodeImpl& impl = upstreamNode->getImplementation();
            ShaderOutput* upstreamOutput = upstreamNode->getOutput();
            if (upstreamOutput)
            {
                // Find out which input needs to be sampled multiple times
                // If the sample count is 1 then the sample code has already been emitted
                const ShaderInput* samplingInput = (sampleCount > 1) ? getSamplingInput(*upstreamNode) : nullptr;

                // TODO: For now we only support uv space sampling
                if (samplingInput && samplingInput->type != Type::VECTOR2)
                {
                    samplingInput = nullptr;
                }

                if (samplingInput)
                {
                    // This is not exposed. Assume a filter size of 1 with no offset

                    const ShaderOutput* output = node.getOutput();

                    // Emit code to compute sample size
                    //
                    string sampleInputValue;
                    shadergen.getInput(context, samplingInput, sampleInputValue);

                    const string sampleSizeName(output->variable + "_sample_size");
                    const string vec2TypeString = shadergen.getSyntax()->getTypeName(Type::VECTOR2);
                    string sampleCall(vec2TypeString + " " + sampleSizeName + " = " +
                        sampleSizeFunctionUV + "(" +
                        sampleInputValue + "," +
                        std::to_string(filterSize) + "," +
                        std::to_string(filterOffset) + ");"
                    );
                    shadergen.emitLine(stage, sampleCall);

                    // Build the sample offset strings. This is dependent on
                    // the derived class to determine where samples are located
                    // and to generate the strings required to offset from the center
                    // sample. The sample size is passed over.
                    //
                    StringVec inputVec2Suffix;
                    if (sampleCount > 1)
                    {
                        computeSampleOffsetStrings(sampleSizeName, vec2TypeString,
                            filterWidth, inputVec2Suffix);
                    }

                    // Emit outputs for sample input 
                    for (unsigned int i = 0; i < sampleCount; i++)
                    {
                        // Add an input name suffix. 
                        context.addInputSuffix(samplingInput, inputVec2Suffix[i]);

                        // Add a output name suffix for the emit call
                        string outputSuffix("_" + output->variable + std::to_string(i));
                        context.addOutputSuffix(upstreamOutput, outputSuffix);

                        impl.emitFunctionCall(stage, *upstreamNode, shadergen, context);

                        // Remove suffixes
                        context.removeInputSuffix(samplingInput);
                        context.removeOutputSuffix(upstreamOutput);

                        // Keep track of the output name with the suffix
                        sampleStrings.push_back(upstreamOutput->variable + outputSuffix);
                    }
                }
                else
                {
                    // Use the same input for all samples.
                    // Note: This does not recomputed the output, but instead it reuses
                    // the output variable.
                    for (unsigned int i = 0; i < sampleCount; i++)
                    {
                        // Call the unmodified function
                        sampleStrings.push_back(upstreamOutput->variable);
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
            string scalarValueString = inInput->value ? inInput->value->getValueString() : "1";
            for (unsigned int i = 0; i < sampleCount; i++)
            {
                sampleStrings.push_back(scalarValueString);
            }
        }
        else
        {
            string typeString = shadergen.getSyntax()->getTypeName(inInput->type);
            string inValueString = typeString + "(" + inInput->value->getValueString() + ")";
            for (unsigned int i = 0; i < sampleCount; i++)
            {
                sampleStrings.push_back(inValueString);
            }
        }
    }
} 

} // namespace MaterialX
