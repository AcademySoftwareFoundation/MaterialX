//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#include <iostream>

namespace MaterialX
{

    //
    // ShaderTranslator methods
    //

    ShaderTranslator::ShaderTranslator(ConstDocumentPtr doc) :
        _doc(doc)
    {
        loadShadingTranslations();
    }

    void ShaderTranslator::loadShadingTranslations()
    {
        for (NodeDefPtr node : _doc->getNodeDefs())
        {
            if (node->getNodeGroup() == NodeDef::TRANSLATION_NODE_GROUP)
            {
                _translationNodes.insert(node->getNodeString());

                // Parsing translation nodes
                size_t pos = node->getNodeString().find("_to_");
                string start = node->getNodeString().substr(0, pos);
                string end = node->getNodeString().substr(pos + 4);
                std::unordered_map<string, StringSet>::const_iterator it = _shadingTranslations.find(start);

                if (it != _shadingTranslations.end())
                {
                    _shadingTranslations[start].insert(end);
                }
                else
                {
                    _shadingTranslations[start] = { end };
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

        DocumentPtr doc = shaderRef->getDocument();
        string translateNodeString = shaderRef->getNodeString() + "_to_" + destShader;
        if (!_translationNodes.count(translateNodeString))
        {
            return;
        }
        NodeGraphPtr ng = doc->getNodeGraph(shaderRef->getBindInputs()[0]->getAttribute(PortElement::NODE_GRAPH_ATTRIBUTE));
        _translationNode = ng->addNode(translateNodeString, "translation", MULTI_OUTPUT_TYPE_STRING);

        int size = shaderRef->getBindInputs().size();
        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {

            OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                NodeGraphPtr translationNg = output->getDocument()->getNodeGraph("NG_" + translateNodeString);
                OutputPtr translationOutput = translationNg->getOutput(bindInput->getName() + "_out");
                InputPtr input;
                InputPtr normalMapInput;
                if (connectsToNormalMapNode(translationOutput))
                {
                    NodePtr normalMapNode = ng->addNode("normalmap", bindInput->getName() + "_map", output->getType());
                    input = normalMapNode->addInput("in", output->getType());
                }
                else
                {
                    input = _translationNode->addInput(bindInput->getName(), bindInput->getType());
                }
                input->setConnectedNode(ng->getNode(output->getNodeName()));

                ng->removeOutput(output->getName());
            }
            else
            {
                std::cerr << "No associated output with " << bindInput->getName() << std::endl;
            }

            shaderRef->removeBindInput(bindInput->getName());
        }

        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {
            shaderRef->removeBindInput(bindInput->getName());
        }

        shaderRef->setNodeString(destShader);
        vector<OutputPtr> outputs = doc->getNodeGraph("NG_" + translateNodeString)->getOutputs();
        for (OutputPtr newSrInput : outputs)
        {
            string outputName = newSrInput->getName();
            outputName = outputName.substr(0, outputName.find("_out"));
            NodePtr outNode = ng->getNode(outputName + "_map");
            if (!outNode)
            {
                outNode = ng->addNode("dot", outputName + "_dot", newSrInput->getType());
                InputPtr dotNodeInput = outNode->addInput("in", newSrInput->getType());

                if (newSrInput->getNodeName() == EMPTY_STRING)
                {
                    dotNodeInput->setValueString(newSrInput->getValueString());
                }
                else
                {
                    dotNodeInput->setConnectedNode(_translationNode);
                    dotNodeInput->setOutputString(outputName + "_out");
                }
            }

            OutputPtr translatedOutput = ng->addOutput(outputName + "_out", newSrInput->getType());
            translatedOutput->setConnectedNode(outNode);

            BindInputPtr translatedBindInput = shaderRef->addBindInput(outputName, newSrInput->getType());
            translatedBindInput->setConnectedOutput(translatedOutput);
        }
    }

    void ShaderTranslator::translateAllMaterials(DocumentPtr doc, string destShader)
    {
        ShaderTranslatorPtr translator = ShaderTranslator::create(doc);
        vector<TypedElementPtr> renderableShaderRefs;
        std::unordered_set<OutputPtr> outputs;
        findRenderableShaderRefs(doc, renderableShaderRefs, false, outputs);
        for (TypedElementPtr elem : renderableShaderRefs)
        {
            ShaderRefPtr sr = elem ? elem->asA<ShaderRef>() : nullptr;
            string startShader = sr->getNodeString();
            string startName = sr->getName();
            if (translator->getAvailableTranslations(startShader).count(destShader))
            {
                translator->translateShader(sr, destShader);
                std::cout << "Successfully translated " << startName << " from " << startShader << 
                    " to " << destShader << std::endl;
            }
        }
    }

} // namespace MaterialX
