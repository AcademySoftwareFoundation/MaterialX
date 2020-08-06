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
        // read all the mltx files in the directory
        // or collect all the translations in a specific nodegroup
        // parse and then add it to a map
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
                int pos = node->getNodeString().find("_to_");
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

    bool ShaderTranslator::translateShader(ShaderRefPtr shaderRef, string destShader)
    {
        if (!shaderRef)
        {
            return false;
        }

        DocumentPtr doc = shaderRef->getDocument();
        string translateNodeString = shaderRef->getNodeString() + "_to_" + destShader;
        if (!_translationNodes.count(translateNodeString))
        {
            return false;
        }
        // Assuming the shaderref inputs all come from one nodegraph

        // get the nodegraph
        NodeGraphPtr ng = doc->getNodeGraph(shaderRef->getBindInputs()[0]->getAttribute(PortElement::NODE_GRAPH_ATTRIBUTE));
        _translationNode = ng->addNode(translateNodeString, "translation", MULTI_OUTPUT_TYPE_STRING);

        int size = shaderRef->getBindInputs().size();
        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {
            OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                InputPtr input = _translationNode->addInput(bindInput->getName(), bindInput->getType());
                input->setConnectedNode(ng->getNode(output->getNodeName()));
                input->setNodeName(output->getNodeName());

                ng->removeOutput(output->getName());
            }
        }

        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {
            shaderRef->removeBindInput(bindInput->getName());
        }


        shaderRef->setNodeString(destShader);
        vector<OutputPtr> outputs = doc->getNodeGraph("NG_" + translateNodeString)->getOutputs();
        for (OutputPtr newSrInput : doc->getNodeGraph("NG_" + translateNodeString)->getOutputs())
        {
            string outputName = newSrInput->getName();
            outputName = outputName.substr(0, outputName.find("_out"));
            NodePtr dotNode = ng->addNode("dot", outputName + "_dot", newSrInput->getType());
            InputPtr dotNodeInput = dotNode->addInput("in", newSrInput->getType());
            dotNodeInput->setConnectedNode(_translationNode);
            dotNodeInput->setOutputString(outputName + "_out");

            OutputPtr translatedOutput = ng->addOutput(outputName + "_out", newSrInput->getType());
            translatedOutput->setConnectedNode(dotNode);

            BindInputPtr translatedBindInput = shaderRef->addBindInput(outputName, newSrInput->getType());
            translatedBindInput->setConnectedOutput(translatedOutput);

        }
        return true;
    }

} // namespace MaterialX
