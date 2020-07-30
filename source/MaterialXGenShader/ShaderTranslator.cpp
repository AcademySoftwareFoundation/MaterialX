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

    void ShaderTranslator::connectToTranslationInputs(ShaderRefPtr shaderRef)
    {
        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {

            OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                NodeGraphPtr translationNg = output->getDocument()->getNodeGraph("NG_" + _translationNode->getCategory());
                OutputPtr translationOutput = translationNg->getOutput(bindInput->getName() + "_out");
                InputPtr input = _translationNode->addInput(bindInput->getName(), bindInput->getType());
                input->setConnectedNode(_ng->getNode(output->getNodeName()));

                _ng->removeOutput(output->getName());
            }
            else
            {
                std::cerr << "No associated output with " << bindInput->getName() << std::endl;
            }

            shaderRef->removeBindInput(bindInput->getName());
        }
    }

    void ShaderTranslator::insertOutputNgUpstreamElems(OutputPtr translatedOutput, OutputPtr ngOutput)
    {
        ElementPtr top = ngOutput->asA<Element>();
        vector<ElementPtr> upstreamElements;
        while (top)
        {
            upstreamElements.push_back(top);
            top = top->getUpstreamElement();
        }

        // rebuilding upstream node dependencies from translation output to input
        for (ElementPtr upstreamElem : upstreamElements)
        {
            if (upstreamElem == ngOutput->asA<Element>())
            {
                // connecting output to the upstream node (that will be copied over)
                translatedOutput->setNodeName(upstreamElem->asA<Output>()->getNodeName());
            }
            else
            {
                NodePtr node = upstreamElem->asA<Node>();
                // copy upstream node over
                NodePtr nodeCopy = _ng->addNode(node->getCategory(), node->getName(), node->getType());

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
    }

    void ShaderTranslator::connectTranslationOutputs(ShaderRefPtr shaderRef)
    {
        DocumentPtr doc = shaderRef->getDocument();
        vector<OutputPtr> outputs = doc->getNodeGraph("NG_" + _translationNode->getCategory())->getOutputs();
        for (OutputPtr translationNgOutput : outputs)
        {
            // Updating the shaderref sockets
            string outputName = translationNgOutput->getName();
            outputName = outputName.substr(0, outputName.find("_out"));
            OutputPtr translatedOutput = _ng->addOutput(outputName + "_out", translationNgOutput->getType());
            BindInputPtr translatedBindInput = shaderRef->addBindInput(outputName, translationNgOutput->getType());
            translatedBindInput->setConnectedOutput(translatedOutput);
            // if normals need to be transformed into world space
            if (connectsToNormalMapNode(translationNgOutput))
            {
                insertOutputNgUpstreamElems(translatedOutput, translationNgOutput);
            }
            else
            {
                // registering outputs from translation node
                NodePtr outNode = _ng->addNode("dot", outputName + "_dot", translationNgOutput->getType());
                translatedOutput->setConnectedNode(outNode);

                InputPtr dotNodeInput = outNode->addInput("in", translationNgOutput->getType());

                // if value does not need to be computed
                if (translationNgOutput->getNodeName() == EMPTY_STRING)
                {
                    dotNodeInput->setValueString(translationNgOutput->getValueString());
                }
                else
                {
                    dotNodeInput->setConnectedNode(_translationNode);
                    dotNodeInput->setOutputString(outputName + "_out");
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
        _ng = doc->getNodeGraph(shaderRef->getBindInputs()[0]->getAttribute(PortElement::NODE_GRAPH_ATTRIBUTE));
        _translationNode = _ng->addNode(translateNodeString, "translation", MULTI_OUTPUT_TYPE_STRING);

        connectToTranslationInputs(shaderRef);
        shaderRef->setNodeString(destShader);
        connectTranslationOutputs(shaderRef);
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
