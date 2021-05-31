//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderTranslator.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

//
// ShaderTranslator methods
//

ShaderTranslator::ShaderTranslator()
{
}

void ShaderTranslator::connectTranslationInputs(NodePtr shader, NodeDefPtr translationNodeDef)
{
    vector<InputPtr> origInputs = shader->getInputs();
    std::set<OutputPtr> origOutputs;
    for (InputPtr shaderInput : origInputs)
    {
        if (translationNodeDef->getInput(shaderInput->getName()))
        {
            OutputPtr output = shaderInput->getConnectedOutput();
            if (output)
            {
                InputPtr input = _translationNode->addInput(shaderInput->getName(), shaderInput->getType());
                input->setConnectedNode(_graph->getNode(output->getNodeName()));
                if (!shaderInput->getColorSpace().empty())
                {
                    input->setColorSpace(shaderInput->getColorSpace());
                }
                origOutputs.insert(output);
            }
            else if (!shaderInput->getValueString().empty())
            { 
                InputPtr input = _translationNode->addInput(shaderInput->getName(), shaderInput->getType());
                input->setValueString(shaderInput->getValueString());
                if (!shaderInput->getColorSpace().empty())
                {
                    input->setColorSpace(shaderInput->getColorSpace());
                }
                if (!shaderInput->getUnit().empty())
                {
                    input->setUnit(shaderInput->getUnit());
                    input->setUnitType(shaderInput->getUnitType());
                }
            }
            else
            {
                throw Exception("No associated output with " + shaderInput->getName());
            }
        }
    }

    for (InputPtr input : origInputs)
    {
        shader->removeInput(input->getName());
    }
    for (OutputPtr output : origOutputs)
    {
        _graph->removeOutput(output->getName());
    }
}

void ShaderTranslator::connectTranslationOutputs(NodePtr shader)
{
    DocumentPtr doc = shader->getDocument();
    InterfaceElementPtr implement = _translationNode->getImplementation();
    NodeGraphPtr translationGraph = implement ? implement->asA<NodeGraph>() : nullptr;
    if (!translationGraph)
    {
        throw Exception("No graph implementation for " + _translationNode->getCategory() + " was found");
    }

    // Iterate through outputs of the translation graph.
    for (OutputPtr translationGraphOutput : translationGraph->getOutputs())
    {
        // Convert output name to input name, using a hardcoded naming convention for now.
        string outputName = translationGraphOutput->getName();
        size_t pos = outputName.find("_out");
        if (pos == string::npos)
        {
            throw Exception("Translation graph output " + outputName + " does not end with '_out'");
        }
        string inputName = outputName.substr(0, pos);

        // Determine the node and output representing this translated stream.
        NodePtr translatedStreamNode = _translationNode;
        string translatedStreamOutput = outputName;

        // Nodes with world-space outputs are moved outside of their containing graph,
        // providing greater flexibility in texture baking.
        NodePtr worldSpaceNode = connectsToWorldSpaceNode(translationGraphOutput);
        if (worldSpaceNode)
        {
            InputPtr nodeInput = worldSpaceNode->getInput("in");
            if (nodeInput && nodeInput->hasInterfaceName())
            {
                InputPtr interfaceInput = _translationNode->getInput(nodeInput->getInterfaceName());
                if (interfaceInput)
                {
                    NodePtr sourceNode = interfaceInput->getConnectedNode();
                    if (sourceNode)
                    {
                        translatedStreamNode = _graph->addNode(worldSpaceNode->getCategory(), worldSpaceNode->getName(), worldSpaceNode->getType());
                        translatedStreamNode->setConnectedNode("in", sourceNode);
                        translatedStreamOutput = EMPTY_STRING;
                    }
                }
            }
        }

        // Create translated output.
        OutputPtr translatedOutput = _graph->getOutput(outputName);
        if (!translatedOutput)
        {
            translatedOutput = _graph->addOutput(outputName, translationGraphOutput->getType());
        }
        translatedOutput->setConnectedNode(translatedStreamNode);
        if (!translatedStreamOutput.empty())
        {
            translatedOutput->setOutputString(translatedStreamOutput);
        }

        // Add translated shader input.
        InputPtr translatedShaderInput = shader->getInput(inputName);
        if (!translatedShaderInput)
        {
            translatedShaderInput = shader->addInput(inputName, translationGraphOutput->getType());
        }
        translatedShaderInput->setConnectedOutput(translatedOutput);
    }
}

void ShaderTranslator::translateShader(NodePtr shader, const string& destCategory)
{
    if (!shader)
    {
        return;
    }

    const string& sourceCategory = shader->getCategory();
    if (sourceCategory == destCategory)
    {
        throw Exception("The source shader \"" + shader->getNamePath() + "\" category is already \"" + destCategory + "\"");
    }

    DocumentPtr doc = shader->getDocument();
    vector<OutputPtr> referencedOutputs = getConnectedOutputs(shader);
    if (!referencedOutputs.empty())
    {
        _graph = referencedOutputs[0]->getParent() ? referencedOutputs[0]->getParent()->asA<NodeGraph>() : nullptr;
    }
    if (!_graph)
    {
        _graph = doc->addNodeGraph();
    }

    string translateNodeString = sourceCategory + "_to_" + destCategory;
    vector<NodeDefPtr> matchingNodeDefs = doc->getMatchingNodeDefs(translateNodeString);
    if (matchingNodeDefs.empty())
    {
        throw Exception("Shader translation requires a translation nodedef named " + translateNodeString);
    }
    NodeDefPtr translationNodeDef = matchingNodeDefs[0];
    _translationNode = _graph->addNodeInstance(translationNodeDef);

    connectTranslationInputs(shader, translationNodeDef);
    shader->setCategory(destCategory);
    shader->removeAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE);
    connectTranslationOutputs(shader);

    _graph = nullptr;
    _translationNode = nullptr;
}

void ShaderTranslator::translateAllMaterials(DocumentPtr doc, string destCategory)
{
    vector<TypedElementPtr> materialNodes;
    std::unordered_set<ElementPtr> shaderOutputs;
    findRenderableMaterialNodes(doc, materialNodes, false, shaderOutputs);
    for (auto elem : materialNodes)
    {
        NodePtr materialNode = elem->asA<Node>();
        if (!materialNode)
        {
            continue;
        }
        std::unordered_set<NodePtr> shaderNodes = getShaderNodes(materialNode);
        for (auto shaderNode : shaderNodes)
        {
            translateShader(shaderNode, destCategory);
        }
    }
}

} // namespace MaterialX
