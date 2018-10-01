#include <MaterialXGenGlsl/Nodes/HeightToNormalGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

    SgImplementationPtr HeightToNormalGlsl::create()
    {
        return std::make_shared<HeightToNormalGlsl>();
    }

    void HeightToNormalGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
    {
        // Screen size in pixels to be set by client.
        HwShader& shader = static_cast<HwShader&>(shader_);
        //const string OGSFX_SIZE_SEMANTIC("ViewportPixelSize"); -- This causes bad code to be emitted.
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS, Type::VECTOR2, "u_screenSize" /*, OGSFX_SIZE_SEMANTIC*/);
    }

    void HeightToNormalGlsl::emitFunctionDefinition(const SgNode& /*node*/, ShaderGenerator& shadergen_, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);
        GlslShaderGenerator shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        // Emit function signature.
        // Sobel filter computation. TODO: This make the filter operation settable.
        //
        const char* SOBEL_FILTER_SOURCE =
            "vec3 IM_heighttonormal_vector3_sx_glsl(float S[9], float _scale)\n"
            "{\n"
            "   float nx = S[0] - S[2] + (2.0*S[3]) - (2.0*S[5]) + S[6] - S[8];\n"
            "   float ny = S[0] + (2.0*S[1]) + S[2] - S[6] - (2.0*S[7]) - S[8];\n"
            "   float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);\n"
            "   return vec3(nx, ny, nz);\n"
            "}\n\n";

        shader.addBlock(SOBEL_FILTER_SOURCE, shadergen);

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

    void HeightToNormalGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        const SgInput* inInput = node.getInput("in");
        const SgInput* scaleInput = node.getInput("scale");

        if (!inInput || !scaleInput)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
        }

        string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

        string inputName = inInput->name;

        // We assume a set of 3 x 3 samples organized as follows:
        // 
        // ----+-----+----
        //  0  |  1  | 2
        // ----+-----+----
        //  3  |  4  | 5
        // ----+-----+----
        //  6  |  7  | 8
        // ----+-----+----
        //
        const unsigned int sampleSize = 9;
        std::vector<std::string> sampleStrings;

        // Compute the sample offsets relative to the centre sample:
        // TODO: For now we assume some small increment in u,v space, assuming that it is a 2D
        // uv space set of samples. More work is required here.
        float sampleOffsetX(0.0009765625f);
        float sampleOffsetY(0.0009765625f);
        string sampleOffsetXString(std::to_string(sampleOffsetX));
        string sampleOffsetYString(std::to_string(sampleOffsetY));
        vector<string> inputVec2Suffix;
        for (int row = -1; row <= 1; row++)
        {
            for (int col = -1; col <= 1; col++)
            {
                inputVec2Suffix.push_back(" + vec2(" + std::to_string(sampleOffsetX*float(col)) + "," + std::to_string(sampleOffsetY*float(row)) + ")");
            }
        }

        // Require an upstream node to sample
        string upstreamNodeName;
        SgNode* upstreamNode = nullptr;
        SgOutput* inConnection = inInput->connection;
        if (inConnection && inConnection->type->isScalar())
        {
            upstreamNode = inConnection->node;
            upstreamNodeName = inConnection->name;

            if (upstreamNode)
            {
                SgImplementation *impl = upstreamNode->getImplementation();
                if (impl)
                {
                    SgOutput* upstreamOutput = upstreamNode->getOutput();
                    if (upstreamOutput)
                    {
                        string outputName = upstreamOutput->name;

                        // Emit outputs for sample input 
                        const unsigned int CENTER_SAMPLE(4);
                        for (unsigned int i = 0; i < sampleSize; i++)
                        {
                            // Computation of the center sample has already been
                            // output so just use that output variable
                            if (i == CENTER_SAMPLE)
                            {
                                sampleStrings.push_back(outputName);
                            }
                            else
                            {
                                // Add an input name suffix. Only for 2d texcoord inputs
                                // for now.
                                std::vector<SgInput*> sampleInputs;
                                for (SgInput* input : upstreamNode->getInputs())
                                {
                                    if (input->name == "texcoord")
                                    {
                                        sampleInputs.push_back(input);
                                    }
                                }

                                if (!sampleInputs.empty())
                                {
                                    // Add input suffixes for the emit call
                                    for (auto sampleInput : sampleInputs)
                                    {
                                        context.addInputSuffix(sampleInput, inputVec2Suffix[i]);
                                    }

                                    // Add a output name suffix for the emit call
                                    string outputSuffix("_" + node.getOutput()->name + std::to_string(i));
                                    context.addOutputSuffix(upstreamOutput, outputSuffix);
                                    
                                    impl->emitFunctionCall(*upstreamNode, context, shadergen, shader);
                                    
                                    // Remove suffixes
                                    for (auto sampleInput : sampleInputs)
                                    {
                                        context.removeInputSuffix(sampleInput);
                                    }
                                    context.removeOutputSuffix(upstreamOutput);

                                    // Keep track of the output name with the suffix
                                    sampleStrings.push_back(outputName + outputSuffix);
                                }
                                else
                                {
                                    // On failure just call the unmodified function
                                    sampleStrings.push_back(outputName);
                                }
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
                throw ExceptionShaderGenError("No connection or value found on heighttonormal node '" + node.getName() + "'");
            }
        }

        // Build a set of samples with constant values
        if (sampleStrings.empty())
        {
            string inValueString = inInput->value->getValueString();
            for (unsigned int i = 0; i < sampleSize; i++)
            {
                sampleStrings.push_back(inValueString);
            }
        }

        // Dump out sample evaluation code
        //
        string sampleName(node.getOutput()->name + "_samples"); 
        shader.addLine("float " + sampleName + "[" + std::to_string(sampleSize) + "]");
        for (unsigned int i=0; i<sampleSize; i++)
        {
            shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
        }
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = IM_heighttonormal_vector3_sx_glsl");
        shader.addStr("(" + sampleName + ", " + scaleValueString + ")");
        shader.endLine();

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
