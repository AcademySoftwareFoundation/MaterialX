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

void ShaderTranslator::connectTranslationInputs(ShaderRefPtr shaderRef, NodeDefPtr translationNodeDef)
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

void ShaderTranslator::connectTranslationOutputs(ShaderRefPtr shaderRef)
{
    StringSet categories;
    categories.insert("normalmap");

    DocumentPtr doc = shaderRef->getDocument();
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
            translatedNode = _graph->addNode("dot", inputName + "_dot", translationGraphOutput->getType());
            InputPtr dotNodeInput = translatedNode->addInput("in", translationGraphOutput->getType());
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

        // Create translated output.
        OutputPtr translatedOutput = _graph->addOutput(outputName, translationGraphOutput->getType());
        translatedOutput->setConnectedNode(translatedNode);

        // Add translated bindinput.
        BindInputPtr translatedBindInput = shaderRef->addBindInput(inputName, translationGraphOutput->getType());
        translatedBindInput->setConnectedOutput(translatedOutput);
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

    connectTranslationInputs(shaderRef, translationNodeDef);
    shaderRef->setNodeString(destShader);
    shaderRef->removeAttribute(ShaderRef::NODE_DEF_ATTRIBUTE);
    connectTranslationOutputs(shaderRef);
}

void ShaderTranslator::translateAllMaterials(DocumentPtr doc, string destShader)
{
    vector<TypedElementPtr> renderableShaderRefs;
    std::unordered_set<ElementPtr> outputs;
    findRenderableShaderRefs(doc, renderableShaderRefs, false, outputs);
    for (TypedElementPtr elem : renderableShaderRefs)
    {
        ShaderRefPtr shaderRef = elem ? elem->asA<ShaderRef>() : nullptr;
        translateShader(shaderRef, destShader);
    }
}

} // namespace MaterialX
