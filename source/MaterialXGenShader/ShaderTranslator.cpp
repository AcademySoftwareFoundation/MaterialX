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
    vector<InputPtr> origShaderInput = shader->getInputs();
    for (InputPtr shaderInput : origShaderInput)
    {
        if (translationNodeDef->getInput(shaderInput->getName()))
        {
            OutputPtr output = shaderInput->getConnectedOutput();
            if (output)
            {
                InputPtr input = _translationNode->addInput(shaderInput->getName(), shaderInput->getType());
                input->setConnectedNode(_graph->getNode(output->getNodeName()));
                if (shaderInput->getColorSpace() != EMPTY_STRING) {
                  input->setColorSpace(shaderInput->getColorSpace());
                }
                _graph->removeOutput(output->getName());
            }
            else if (shaderInput->getValueString() != EMPTY_STRING)
            { 
                InputPtr input = _translationNode->addInput(shaderInput->getName(), shaderInput->getType());
                input->setValueString(shaderInput->getValueString());
                if (shaderInput->getColorSpace() != EMPTY_STRING) {
                    input->setColorSpace(shaderInput->getColorSpace());
                }
                if (shaderInput->getUnit() != EMPTY_STRING) {
                    input->setUnit(shaderInput->getUnit());
                    input->setUnitType(shaderInput->getUnitType());
                }
            }
            else
            {
                throw Exception("No associated output with " + shaderInput->getName());
            }
        }

        shader->removeInput(shaderInput->getName());
    }
}

void ShaderTranslator::connectTranslationOutputs(NodePtr shader)
{
    StringSet categories;
    categories.insert("normalmap");

    DocumentPtr doc = shader->getDocument();
    vector<OutputPtr> outputs = doc->getNodeGraph("NG_" + _translationNode->getCategory())->getOutputs();
    for (OutputPtr translationGraphOutput : outputs)
    {
        string outputName = translationGraphOutput->getName();
        size_t pos = outputName.find("_out");
        if (pos == string::npos)
        {
            throw Exception("Translation graph output " + outputName + " does not end with '_out'");
        }
        string inputName = outputName.substr(0, pos);

        // Create the translated node, handling both normal map and simple dot cases.
        NodePtr translatedNode;
        if (connectsToNodeOfCategory(translationGraphOutput, categories))
        {
            NodePtr normalMapNode = translationGraphOutput->getConnectedNode();
            translatedNode = _graph->addNode(normalMapNode->getCategory(), normalMapNode->getName(), normalMapNode->getType());
            for (InputPtr input : normalMapNode->getInputs())
            {
                string inputNodeName;
                if (input->getInterfaceName() != EMPTY_STRING)
                {
                    InputPtr interfaceInput = _translationNode->getInput(input->getInterfaceName());
                    if (interfaceInput)
                    {
                        inputNodeName = interfaceInput->getNodeName();
                    }
                }
                else
                {
                    inputNodeName = input->getNodeName();
                }
                if (!inputNodeName.empty())
                {
                    InputPtr inputCopy = translatedNode->addInput(input->getName(), input->getType());
                    inputCopy->setNodeName(inputNodeName);
                }
            }
            if (!translatedNode->getInputCount())
            {
                _graph->removeNode(translatedNode->getName());
                continue;
            }
        }
        else
        {
            const string DOT_NAME = inputName + "_dot";
            translatedNode = _graph->getNode(DOT_NAME);
            if (!translatedNode)
            {
                translatedNode = _graph->addNode("dot", DOT_NAME, translationGraphOutput->getType());
            }
            InputPtr dotNodeInput = translatedNode->getInput("in");
            if (!dotNodeInput)
            {
                dotNodeInput = translatedNode->addInput("in", translationGraphOutput->getType());
            }
            if (translationGraphOutput->getNodeName() == EMPTY_STRING)
            {
                dotNodeInput->setValueString(translationGraphOutput->getValueString());
            }
            else
            {
                dotNodeInput->setConnectedNode(_translationNode);
                dotNodeInput->setOutputString(outputName);
            }
            if (!translationGraphOutput->getColorSpace().empty()) 
            {
                dotNodeInput->setColorSpace(translationGraphOutput->getColorSpace());
            }
            if (!translationGraphOutput->getUnit().empty())
            {
                dotNodeInput->setUnit(translationGraphOutput->getUnit());
                dotNodeInput->setUnitType(translationGraphOutput->getUnitType());
            }
        }

        // Create translated output.
        OutputPtr translatedOutput = _graph->getOutput(outputName);
        if (!translatedOutput)
        {
            translatedOutput = _graph->addOutput(outputName, translationGraphOutput->getType());
        }
        translatedOutput->setConnectedNode(translatedNode);

        // Add translated shaderInput.
        InputPtr translatedshaderInput = shader->getInput(inputName);
        if (!translatedshaderInput)
        {
            translatedshaderInput = shader->addInput(inputName, translationGraphOutput->getType());
        }
        translatedshaderInput->setConnectedOutput(translatedOutput);
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
