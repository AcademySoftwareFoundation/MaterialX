#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/GlslSyntax.h>

namespace MaterialX
{

GlslShaderGenerator::GlslShaderGenerator()
    : ShaderGenerator(std::make_shared<GlslSyntax>())
{
}

void GlslShaderGenerator::emitClosureInputs(Shader& shader)
{
    // Emit function calls for all non-closure nodes (texturing nodes)
    for (const SgNode& node : shader.getNodes())
    {
        // Emit only unconditional nodes, since any node within a conditional 
        // branch is emitted by the conditional node itself
        if (!node.hasClassification(SgNode::Classification::CLOSURE) && !node.referencedConditionally())
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
        else if (node.hasClassification(SgNode::Classification::SURFACE))
        {
            emitFunctionCall(node, shader);
            last = &node;
        }
    }

    if (last)
    {
        bsdf = _syntax->getVariableName(*last->getNodePtr());
        if (last->hasClassification(SgNode::Classification::SURFACE))
        {
            bsdf += ".bsdf";
        }
    }

    shader.setContext(oldContext);
}

void GlslShaderGenerator::emitSurfaceEmissionAndOpacity(Shader& shader, string& emission, string& opacity)
{
    unsigned char oldContext = shader.getContext();
    shader.setContext(Shader::Context::EMISSION);

    const SgNode* last = nullptr;

    // Emit function calls for all EDF nodes
    // The last node will hold the final result
    for (const SgNode& node : shader.getNodes())
    {
        if (node.hasClassification(SgNode::Classification::EDF) ||
            node.hasClassification(SgNode::Classification::SURFACE))
        {
            emitFunctionCall(node, shader);
            last = &node;
        }
    }

    if (last && last->hasClassification(SgNode::Classification::SURFACE))
    {
        emission = opacity = _syntax->getVariableName(*last->getNodePtr());
        emission += ".edf";
        opacity  += ".opacity";
    }

    shader.setContext(oldContext);
}

}
