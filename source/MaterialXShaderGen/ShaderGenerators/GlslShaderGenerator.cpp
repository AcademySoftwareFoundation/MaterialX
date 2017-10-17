#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/GlslSyntax.h>

namespace MaterialX
{

GlslShaderGenerator::GlslShaderGenerator()
    : ShaderGenerator(std::make_shared<GlslSyntax>())
{
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    for (const SgNode& node : shader.getNodes())
    {
        // Emit only unconditional nodes, since any node within a conditional 
        // branch is emitted by the conditional node itself
        if (node.hasClassification(SgNode::Classification::TEXTURE) && !node.referencedConditionally())
        {
            emitFunctionCall(node, shader);
        }
    }
}

void GlslShaderGenerator::emitBsdf(const string& incident, const string& outgoing, Shader& shader, string& bsdf)
{
    unsigned char oldContext = shader.getContext();
    shader.setContext(Shader::Context::SCATTERING);

    vector<string> lightDirections = { incident, outgoing };
    const SgNode* last = nullptr;

    // Emit function calls for all BSDF nodes
    // The last node will hold the final result
    for (const SgNode& node : shader.getNodes())
    {
        if (node.hasClassification(SgNode::Classification::BSDF))
        {
            emitFunctionCall(node, shader, &lightDirections);
            last = &node;
        }
        else if (node.hasClassification(SgNode::Classification::LAYER))
        {
            emitFunctionCall(node, shader);
            last = &node;
        }
    }

    if (last)
    {
        bsdf = _syntax->getVariableName(*last->getNodePtr());
        if (last->hasClassification(SgNode::Classification::LAYER))
        {
            bsdf += ".bsdf";
        }
    }

    shader.setContext(oldContext);
}

void GlslShaderGenerator::emitSurfaceEmission(Shader& shader, string& emission)
{
    emission = "vec3(0.0)";

    unsigned char oldContext = shader.getContext();
    shader.setContext(Shader::Context::EMISSION);

    const SgNode* last = nullptr;

    // Emit function calls for all EDF nodes
    // The last node will hold the final result
    for (const SgNode& node : shader.getNodes())
    {
        if (node.hasClassification(SgNode::Classification::EDF) ||
            node.hasClassification(SgNode::Classification::LAYER))
        {
            emitFunctionCall(node, shader);
            last = &node;
        }
    }

    if (last)
    {
        emission = _syntax->getVariableName(*last->getNodePtr());
        if (last->hasClassification(SgNode::Classification::LAYER))
        {
            emission += ".edf";
        }
    }

    shader.setContext(oldContext);
}

}
