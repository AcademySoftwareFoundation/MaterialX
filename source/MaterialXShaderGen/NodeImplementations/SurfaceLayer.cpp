#include <MaterialXShaderGen/NodeImplementations/SurfaceLayer.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>


namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(SurfaceLayerGlsl, "surfacelayer", "glsl", "")

namespace
{
    static const string kFunctions =
        "void sx_surfacelayer_scattering(BSDF bsdf, surfacelayer base, vec3 weight, out surfacelayer result)\n"
        "{\n"
        "    result.bsdf.fr = bsdf.fr * weight + base.bsdf.fr * (1 - weight);\n"
        "    result.bsdf.ft = bsdf.ft * weight + base.bsdf.ft * (1 - weight);\n"
        "}\n"
        "\n"
        "void sx_surfacelayer_emission(vec3 edf, surfacelayer base, vec3 weight, out surfacelayer result)\n"
        "{\n"
        "    result.edf = edf * weight + base.edf * (1 - weight);\n"
        "}\n"
        "\n";

    static const string kScatteringFunctionName = "sx_surfacelayer_scattering";
    static const vector<string> kScatteringInputs = {"bsdf", "base", "weight" };

    static const string kEmissionFunctionName = "sx_surfacelayer_emission";
    static const vector<string> kEmissionInputs = { "edf", "base", "weight" };
}

void SurfaceLayerGlsl::emitFunction(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kFunctions);
}

void SurfaceLayerGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    const string* functionName;
    const vector<string>* inputNames;

    if (shader.getContext() == Shader::Context::SCATTERING)
    {
        functionName = &kScatteringFunctionName;
        inputNames = &kScatteringInputs;
    }
    else if (shader.getContext() == Shader::Context::EMISSION)
    {
        functionName = &kEmissionFunctionName;
        inputNames = &kEmissionInputs;
    }
    else
    {
        throw ExceptionShaderGenError("Unsupported context for node 'surfacelayer'");
    }

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node.getNode(), true, shader);
    shader.endLine();

    shader.beginLine();

    // Emit function name
    shader.addStr(*functionName + "(");

    // Emit function inputs
    string delim = "";
    for (const string& inputName : *inputNames)
    {
        // Find the input port on the node instance
        ValueElementPtr input = node.getNode().getChildOfType<ValueElement>(inputName);
        if (!input)
        {
            // Not found so used default on the node def
            input = node.getNodeDef().getChildOfType<ValueElement>(inputName);
            if (!input)
            {
                throw ExceptionShaderGenError("Nodedef for node 'surfacelayer' has no input named '" + inputName  + "'");
            }
        }

        shader.addStr(delim);
        shadergen.emitInput(*input, shader);
        delim = ", ";
    }

    // Emit function output
    shader.addStr(delim);
    shadergen.emitOutput(node.getNode(), false, shader);

    // End function call
    shader.addStr(")");
    shader.endLine();
}

} // namespace MaterialX
