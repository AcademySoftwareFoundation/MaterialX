//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphRefactor.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>

#include <MaterialXCore/Document.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

bool isBsdfMixNode(ShaderNode* node)
{
    return node->hasClassification(ShaderNode::Classification::BSDF |
                                   ShaderNode::Classification::CLOSURE |
                                   ShaderNode::Classification::MIX);
}

bool isLayerWithMixTop(ShaderNode* node)
{
    if (!node->hasClassification(ShaderNode::Classification::LAYER))
    {
        return false;
    }
    ShaderInput* top = node->getInput("top");
    if (!top || !top->getConnection())
    {
        return false;
    }
    return isBsdfMixNode(top->getConnection()->getNode());
}

// Fold a mix weight into one side of a BSDF mix node, inserting a multiply
// node to combine the existing weight (connected or constant) with the mix
// weight.  Return the upstream output to connect to the replacement add node.
ShaderOutput* foldWeightIntoBsdf(ShaderGraph& graph, GenContext& context,
                                 ShaderInput* bsdfInput, ShaderOutput* weightSource,
                                 const string& namePrefix,
                                 NodeDefPtr mulFloatDef, NodeDefPtr mulBsdfDef)
{
    ShaderOutput* bsdfUpstream = bsdfInput->getConnection();
    if (!bsdfUpstream)
    {
        return nullptr;
    }

    ShaderNode* bsdfNode = bsdfUpstream->getNode();
    ShaderInput* weightInput = bsdfNode->getInput("weight");
    if (weightInput)
    {
        // Create a multiply node to combine existing weight with mix weight.
        string mulName = namePrefix + "_weight";
        ShaderNode* mulNode = graph.createNode(mulName, mulName, mulFloatDef, context);
        ShaderInput* mulIn1 = mulNode->getInput("in1");
        ShaderInput* mulIn2 = mulNode->getInput("in2");
        if (mulIn1 && mulIn2)
        {
            ShaderOutput* existingSource = weightInput->getConnection();
            if (existingSource)
            {
                weightInput->breakConnection();
                mulIn1->makeConnection(existingSource);
            }
            else if (weightInput->getValue())
            {
                mulIn1->setValue(weightInput->getValue());
            }
            else
            {
                mulIn1->setValue(Value::createValue<float>(1.0f));
            }
            mulIn2->makeConnection(weightSource);
            weightInput->makeConnection(mulNode->getOutput());
        }
        return bsdfUpstream;
    }
    else
    {
        // Wrap the BSDF with a BSDF*float multiply when no weight input exists.
        string mulName = namePrefix + "_mul";
        ShaderNode* mulNode = graph.createNode(mulName, mulName, mulBsdfDef, context);
        ShaderInput* mulIn1 = mulNode->getInput("in1");
        ShaderInput* mulIn2 = mulNode->getInput("in2");
        if (mulIn1 && mulIn2)
        {
            mulIn1->makeConnection(bsdfUpstream);
            mulIn2->makeConnection(weightSource);
            return mulNode->getOutput();
        }
        return bsdfUpstream;
    }
}

} // anonymous namespace

//
// NodeElisionRefactor
//

const string& NodeElisionRefactor::getName() const
{
    static const string name = "nodeElision";
    return name;
}

size_t NodeElisionRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    size_t numEdits = 0;
    for (ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            if (node->numInputs() != 1 || node->numOutputs() != 1)
            {
                // Constant node doesn't follow expected interface, cannot elide.
                continue;
            }
            // Constant nodes can be elided by moving their value downstream.
            bool canElide = context.getOptions().elideConstantNodes;
            if (!canElide)
            {
                // We always elide filename constant nodes regardless of the
                // option. See DOT below.
                ShaderInput* in = node->getInput("value");
                if (in && in->getType() == Type::FILENAME)
                {
                    canElide = true;
                }
            }
            if (canElide)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::DOT))
        {
            if (node->numOutputs() != 1)
            {
                // Dot node doesn't follow expected interface, cannot elide.
                continue;
            }
            // Filename dot nodes must be elided so they do not create extra samplers.
            ShaderInput* in = node->getInput("in");
            if (in && in->getType() == Type::FILENAME)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
    }
    return numEdits;
}

//
// PremultipliedBsdfAddRefactor
//

const string& PremultipliedBsdfAddRefactor::getName() const
{
    static const string name = "premultipliedBsdfAdd";
    return name;
}

size_t PremultipliedBsdfAddRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    if (!context.getOptions().premultipliedBsdfAdd)
    {
        return 0;
    }

    // Look up all required node definitions up front.
    ConstDocumentPtr doc = graph.getDocument();
    NodeDefPtr invertDef = doc->getNodeDef("ND_invert_float");
    NodeDefPtr mulFloatDef = doc->getNodeDef("ND_multiply_float");
    NodeDefPtr mulBsdfDef = doc->getNodeDef("ND_multiply_bsdfF");
    NodeDefPtr addBsdfDef = doc->getNodeDef("ND_add_bsdf");
    if (!invertDef || !mulFloatDef || !mulBsdfDef || !addBsdfDef)
    {
        return 0;
    }

    // Collect mix nodes with connected weights (avoid modifying the graph while iterating).
    vector<ShaderNode*> mixNodes;
    for (ShaderNode* node : graph.getNodes())
    {
        if (isBsdfMixNode(node))
        {
            ShaderInput* mix = node->getInput("mix");
            if (mix && mix->getConnection())
            {
                mixNodes.push_back(node);
            }
        }
    }

    size_t numEdits = 0;

    for (ShaderNode* mixNode : mixNodes)
    {
        ShaderInput* fgInput = mixNode->getInput("fg");
        ShaderInput* bgInput = mixNode->getInput("bg");
        ShaderInput* mixInput = mixNode->getInput("mix");
        ShaderOutput* mixOutput = mixNode->getOutput();

        if (!fgInput || !bgInput || !mixInput || !mixOutput)
        {
            continue;
        }

        ShaderOutput* mixWeightSource = mixInput->getConnection();

        // Create an invert node to compute (1 - mix).
        string invertName = mixNode->getName() + "_mix_inv";
        ShaderNode* invertNode = graph.createNode(invertName, invertName, invertDef, context);
        ShaderInput* invertIn = invertNode->getInput("in");
        if (invertIn)
        {
            invertIn->makeConnection(mixWeightSource);
        }
        ShaderOutput* invertOutput = invertNode->getOutput();

        // Fold mix weights into each BSDF side.
        string namePrefix = mixNode->getName();
        ShaderOutput* fgUpstream = foldWeightIntoBsdf(graph, context, fgInput, mixWeightSource,
                                                      namePrefix + "_fg", mulFloatDef, mulBsdfDef);
        ShaderOutput* bgUpstream = foldWeightIntoBsdf(graph, context, bgInput, invertOutput,
                                                      namePrefix + "_bg", mulFloatDef, mulBsdfDef);

        // Create an add node to replace the mix.
        string addName = mixNode->getName() + "_add";
        ShaderNode* addNode = graph.createNode(addName, addName, addBsdfDef, context);
        ShaderInput* addIn1 = addNode->getInput("in1");
        ShaderInput* addIn2 = addNode->getInput("in2");
        if (!addIn1 || !addIn2)
        {
            continue;
        }

        if (fgUpstream)
        {
            addIn1->makeConnection(fgUpstream);
        }
        if (bgUpstream)
        {
            addIn2->makeConnection(bgUpstream);
        }

        // Rewire all downstream connections from the mix output to the add output.
        graph.replaceOutput(mixOutput, addNode->getOutput());

        // Disconnect the mix node's inputs.
        fgInput->breakConnection();
        bgInput->breakConnection();
        mixInput->breakConnection();

        ++numEdits;
    }

    return numEdits;
}

//
// DistributeLayerOverMixRefactor
//

const string& DistributeLayerOverMixRefactor::getName() const
{
    static const string name = "distributeLayerOverMix";
    return name;
}

size_t DistributeLayerOverMixRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    if (!context.getOptions().distributeLayerOverBsdfMix)
    {
        return 0;
    }

    // Look up all required node definitions up front.
    ConstDocumentPtr doc = graph.getDocument();
    NodeDefPtr layerBsdfDef = doc->getNodeDef("ND_layer_bsdf");
    NodeDefPtr mixBsdfDef = doc->getNodeDef("ND_mix_bsdf");
    if (!layerBsdfDef || !mixBsdfDef)
    {
        return 0;
    }

    // Collect layer nodes to process (avoid modifying the graph while iterating).
    vector<ShaderNode*> layerNodes;
    for (ShaderNode* node : graph.getNodes())
    {
        if (isLayerWithMixTop(node))
        {
            layerNodes.push_back(node);
        }
    }

    size_t numEdits = 0;

    for (ShaderNode* layerNode : layerNodes)
    {
        ShaderInput* topInput = layerNode->getInput("top");
        ShaderInput* baseInput = layerNode->getInput("base");
        ShaderOutput* layerOutput = layerNode->getOutput();

        if (!topInput || !baseInput || !layerOutput)
        {
            continue;
        }

        ShaderOutput* topConnection = topInput->getConnection();
        if (!topConnection)
        {
            continue;
        }

        ShaderNode* mixNode = topConnection->getNode();
        ShaderInput* fgInput = mixNode->getInput("fg");
        ShaderInput* bgInput = mixNode->getInput("bg");
        ShaderInput* mixInput = mixNode->getInput("mix");

        if (!fgInput || !bgInput || !mixInput)
        {
            continue;
        }

        ShaderOutput* fgUpstream = fgInput->getConnection();
        ShaderOutput* bgUpstream = bgInput->getConnection();
        ShaderOutput* baseUpstream = baseInput->getConnection();
        ShaderOutput* mixWeightSource = mixInput->getConnection();

        // Create layer(fg, base).
        string layer1Name = layerNode->getName() + "_tf";
        ShaderNode* layer1 = graph.createNode(layer1Name, layer1Name, layerBsdfDef, context);
        ShaderInput* layer1Top = layer1->getInput("top");
        ShaderInput* layer1Base = layer1->getInput("base");
        if (layer1Top && fgUpstream)
        {
            layer1Top->makeConnection(fgUpstream);
        }
        if (layer1Base && baseUpstream)
        {
            layer1Base->makeConnection(baseUpstream);
        }

        // Create layer(bg, base).
        string layer2Name = layerNode->getName() + "_notf";
        ShaderNode* layer2 = graph.createNode(layer2Name, layer2Name, layerBsdfDef, context);
        ShaderInput* layer2Top = layer2->getInput("top");
        ShaderInput* layer2Base = layer2->getInput("base");
        if (layer2Top && bgUpstream)
        {
            layer2Top->makeConnection(bgUpstream);
        }
        if (layer2Base && baseUpstream)
        {
            layer2Base->makeConnection(baseUpstream);
        }

        // Create mix(layer1, layer2, w).
        string newMixName = layerNode->getName();
        ShaderNode* newMixNode = graph.createNode(newMixName + "_mix", newMixName + "_mix", mixBsdfDef, context);
        ShaderInput* newFg = newMixNode->getInput("fg");
        ShaderInput* newBg = newMixNode->getInput("bg");
        ShaderInput* newMix = newMixNode->getInput("mix");
        if (newFg)
        {
            newFg->makeConnection(layer1->getOutput());
        }
        if (newBg)
        {
            newBg->makeConnection(layer2->getOutput());
        }
        if (newMix && mixWeightSource)
        {
            newMix->makeConnection(mixWeightSource);
        }
        else if (newMix && mixInput->getValue())
        {
            newMix->setValue(mixInput->getValue());
        }

        // Rewire downstream connections from the old layer to the new mix.
        graph.replaceOutput(layerOutput, newMixNode->getOutput());

        // Disconnect the old layer node's inputs.
        topInput->breakConnection();
        baseInput->breakConnection();

        ++numEdits;
    }

    return numEdits;
}

MATERIALX_NAMESPACE_END
