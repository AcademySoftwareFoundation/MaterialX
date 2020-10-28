//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderTranslator.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

//
// ShaderTranslator methods
//

ShaderTranslator::ShaderTranslator()
{
}

void ShaderTranslator::connectToTranslationInputs(ShaderRefPtr shaderRef, NodeDefPtr translationNodeDef)
{
    vector<BindInputPtr> origBindInputs = shaderRef->getBindInputs();
    for (BindInputPtr bindInput : origBindInputs)
    {
        if (translationNodeDef->getInput(bindInput->getName()))
        {
            OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                InputPtr input = _translationNode->addInput(bindInput->getName(), bindInput->getType());
                input->setConnectedNode(_graph->getNode(output->getNodeName()));

                _graph->removeOutput(output->getName());
            }
            else if (bindInput->getValueString() != EMPTY_STRING)
            { 
                InputPtr input = _translationNode->addInput(bindInput->getName(), bindInput->getType());
                input->setValueString(bindInput->getValueString());
            }
            else
            {
                throw Exception("No associated output with " + bindInput->getName());
            }
        }

        shaderRef->removeBindInput(bindInput->getName());
    }
}

void ShaderTranslator::insertUpstreamDependencies(OutputPtr translatedOutput, OutputPtr graphOutput)
{
    vector<NodePtr> upstreamElements;

    for (Edge edge : graphOutput->traverseGraph())
    {
        ElementPtr upstreamElem = edge.getUpstreamElement();
        if (upstreamElem->isA<Node>())
        {
            upstreamElements.push_back(upstreamElem->asA<Node>());
        }
    }

    translatedOutput->setNodeName(graphOutput->getNodeName());

    // rebuilding upstream node dependencies from translation output to input
    for (NodePtr node : upstreamElements)
    {
        // copy upstream node over
        NodePtr nodeCopy = _graph->addNode(node->getCategory(), node->getName(), node->getType());

        // copying over input information
        for (InputPtr origInput : node->getInputs())
        {
            InputPtr inputCopy = nodeCopy->addInput(origInput->getName(), origInput->getType());
            if (origInput->getInterfaceName() != EMPTY_STRING)
            {
                // directly connecting node to what would have been interface input
                InputPtr interfaceInput = _translationNode->getInput(origInput->getInterfaceName());
                inputCopy->setNodeName(interfaceInput->getNodeName());
            }
            else
            {
                inputCopy->setNodeName(origInput->getNodeName());
            }
        }   
    }
}

void ShaderTranslator::connectTranslationOutputs(ShaderRefPtr shaderRef)
{
    DocumentPtr doc = shaderRef->getDocument();
    vector<OutputPtr> outputs = doc->getNodeGraph("NG_" + _translationNode->getCategory())->getOutputs();
    for (OutputPtr translationGraphOutput : outputs)
    {
        // Updating the shaderref sockets
        string outputName = translationGraphOutput->getName();
        size_t pos = outputName.find("_out");
        if (pos == string::npos)
        {
            throw Exception("Translation graph output " + outputName + " does not end with '_out'");
        }
        string inputName = outputName.substr(0, pos);
        OutputPtr translatedOutput = _graph->addOutput(outputName, translationGraphOutput->getType());
        BindInputPtr translatedBindInput = shaderRef->addBindInput(inputName, translationGraphOutput->getType());
        translatedBindInput->setConnectedOutput(translatedOutput);
        // if normals need to be transformed into world space
        if (connectsToNormalMapNode(translationGraphOutput))
        {
            insertUpstreamDependencies(translatedOutput, translationGraphOutput);
        }
        else
        {
            // registering outputs from translation node
            NodePtr outNode = _graph->addNode("dot", inputName + "_dot", translationGraphOutput->getType());
            translatedOutput->setConnectedNode(outNode);

            InputPtr dotNodeInput = outNode->addInput("in", translationGraphOutput->getType());

            // if value does not need to be computed
            if (translationGraphOutput->getNodeName() == EMPTY_STRING)
            {
                dotNodeInput->setValueString(translationGraphOutput->getValueString());
            }
            else
            {
                dotNodeInput->setConnectedNode(_translationNode);
                dotNodeInput->setOutputString(outputName);
            }
        }
    }
}

void ShaderTranslator::translateShader(ShaderRefPtr shaderRef, string destShader)
{
    if (!shaderRef)
    {
        return;
    }
    if (shaderRef->getNodeString() == destShader)
    {
        throw Exception("Both source and destination shader in translation are " + destShader);
    }

    DocumentPtr doc = shaderRef->getDocument();
    vector<OutputPtr> referencedOutputs = shaderRef->getReferencedOutputs();
    if (!referencedOutputs.empty())
    {
        _graph = referencedOutputs[0]->getParent()->asA<NodeGraph>();
    }
    if (!_graph)
    {
        _graph = doc->addNodeGraph();
    }

    string sourceShader = shaderRef->getNodeString();
    string translateNodeString = sourceShader + "_to_" + destShader;
    vector<NodeDefPtr> matchingNodeDefs = doc->getMatchingNodeDefs(translateNodeString);
    if (matchingNodeDefs.empty())
    {
        throw Exception("Shader translation requires a translation nodedef named " + translateNodeString);
    }
    NodeDefPtr translationNodeDef = matchingNodeDefs[0];
    _translationNode = _graph->addNodeInstance(translationNodeDef);

    connectToTranslationInputs(shaderRef, translationNodeDef);
    shaderRef->setNodeString(destShader);
    shaderRef->removeAttribute(ShaderRef::NODE_DEF_ATTRIBUTE);
    connectTranslationOutputs(shaderRef);
}

void ShaderTranslator::translateAllMaterials(DocumentPtr doc, string destShader)
{
    vector<TypedElementPtr> renderableShaderRefs;
    std::unordered_set<ElementPtr> processedSources;
    findRenderableShaderRefs(doc, renderableShaderRefs, false, processedSources);
    for (TypedElementPtr elem : renderableShaderRefs)
    {
        ShaderRefPtr shaderRef = elem ? elem->asA<ShaderRef>() : nullptr;
        translateShader(shaderRef, destShader);
    }
}

} // namespace MaterialX
