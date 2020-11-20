//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/MaterialNode.h>

namespace MaterialX
{
bool convertMaterialsToNodes(DocumentPtr doc)
{
    bool modified = false;

    vector<MaterialPtr> materials = doc->getMaterials();
    for (auto m : materials)
    {
        // See if a node of this name has already been created.
        // Should not occur otherwise there are duplicate existing
        // Material elements.
        string materialName = m->getName();
        if (doc->getNode(materialName))
        {
            throw Exception("Material node already exists: " + materialName);
        }

        // Create a temporary name for the material element
        // so the new node can reuse the existing name.
        string validName = doc->createValidChildName(materialName + "1");
        m->setName(validName);

        // Create a new material node
        NodePtr materialNode = nullptr;

        ShaderRefPtr sr;
        // Only include the shader refs explicitly specified on the material instance
        vector<ShaderRefPtr> srs = m->getShaderRefs();
        for (size_t i = 0; i < srs.size(); i++)
        {
            sr = srs[i];

            // See if shader has been created already.
            // Should not occur as the shaderref is a uniquely named
            // child of a uniquely named material element, but the two combined
            // may have been used for another node instance which not a shader node.
            string shaderNodeName = materialName + "_" + sr->getName();
            NodePtr existingShaderNode = doc->getNode(shaderNodeName);
            if (existingShaderNode)
            {
                const string& existingType = existingShaderNode->getType();
                if (existingType == VOLUME_SHADER_TYPE_STRING ||
                    existingType == SURFACE_SHADER_TYPE_STRING ||
                    existingType == DISPLACEMENT_SHADER_TYPE_STRING)
                {
                    throw Exception("Shader node already exists: " + shaderNodeName);
                }
                else
                {
                    shaderNodeName = doc->createValidChildName(shaderNodeName);
                }
            }

            modified = true;

            // Find the shader type if defined
            string shaderNodeType = SURFACE_SHADER_TYPE_STRING;
            NodeDefPtr nodeDef = sr->getNodeDef();
            if (nodeDef)
            {
                shaderNodeType = nodeDef->getType();
            }

            // Add in a new shader node
            const string shaderNodeCategory = sr->getNodeString();
            NodePtr shaderNode = doc->addNode(shaderNodeCategory, shaderNodeName, shaderNodeType);
            shaderNode->setSourceUri(sr->getSourceUri());

            for (auto valueElement : sr->getChildrenOfType<ValueElement>())
            {
                ElementPtr portChild = nullptr;

                // Copy over bindinputs as inputs, and bindparams as params
                if (valueElement->isA<BindInput>())
                {
                    portChild = shaderNode->addInput(valueElement->getName(), valueElement->getType());
                }
                else if (valueElement->isA<BindParam>())
                {
                    portChild = shaderNode->addInput(valueElement->getName(), valueElement->getType());
                }
                if (portChild)
                {
                    // Copy over attributes.
                    // Note: We preserve inputs which have nodegraph connections,
                    // as well as top level output connections.
                    portChild->copyContentFrom(valueElement);
                }
            }

            // Copy over any bindtokens as tokens
            for (auto bt : sr->getBindTokens())
            {
                TokenPtr token = shaderNode->addToken(bt->getName());
                token->copyContentFrom(bt);
            }

            // Create a new material node if not already created and
            // add a reference from the material node to the new shader node
            if (!materialNode)
            {
                // Set the type of material based on current assumption that
                // surfaceshaders + displacementshaders result in a surfacematerial
                // while a volumeshader means a volumematerial needs to be created.
                string materialNodeCategory =
                    (shaderNodeType != VOLUME_SHADER_TYPE_STRING) ? SURFACE_MATERIAL_NODE_STRING
                    : VOLUME_MATERIAL_NODE_STRING;
                materialNode = doc->addNode(materialNodeCategory, materialName, MATERIAL_TYPE_STRING);
                materialNode->setSourceUri(m->getSourceUri());
                // Note: Inheritance does not get transfered to the node we do
                // not perform the following:
                //      - materialNode->setInheritString(m->getInheritString());
            }
            // Create input to replace each shaderref. Use shaderref name as unique
            // input name.
            InputPtr shaderInput = materialNode->addInput(shaderNodeType, shaderNodeType);
            shaderInput->setNodeName(shaderNode->getName());
            // Make sure to copy over any target and version information from the shaderref.
            if (!sr->getTarget().empty())
            {
                shaderInput->setTarget(sr->getTarget());
            }
            if (!sr->getVersionString().empty())
            {
                shaderInput->setVersionString(sr->getVersionString());
            }
        }

        // Remove existing material element
        doc->removeChild(m->getName());
    }
    return modified;
}


std::unordered_set<NodePtr> getShaderNodes(const NodePtr& materialNode, const string& nodeType, const string& target)
{
    ElementPtr parent = materialNode->getParent();
    if (!parent)
    {
        throw Exception("Could not find a parent for material node '" + (materialNode ? materialNode->getNamePath() : EMPTY_STRING) + "'");
    }

    std::unordered_set<NodePtr> shaderNodes;

    std::vector<InputPtr> inputs = materialNode->getActiveInputs();
    if (inputs.empty())
    {
        // Try to find material nodes in the implementation graph if any.
        // If a target is specified the nodedef for the given target is searched for.
        NodeDefPtr materialNodeDef = materialNode->getNodeDef(target);
        if (materialNodeDef)
        {
            InterfaceElementPtr impl = materialNodeDef->getImplementation();
            if (impl->isA<NodeGraph>())
            {
                NodeGraphPtr implGraph = impl->asA<NodeGraph>();
                for (auto defOutput : materialNodeDef->getOutputs())
                {
                    if (defOutput->getType() == MATERIAL_TYPE_STRING)
                    {
                        OutputPtr implGraphOutput = implGraph->getOutput(defOutput->getName());
                        for (GraphIterator it = implGraphOutput->traverseGraph().begin(); it != GraphIterator::end(); ++it)
                        {
                            ElementPtr upstreamElem = it.getUpstreamElement();
                            if (!upstreamElem)
                            {
                                it.setPruneSubgraph(true);
                                continue;
                            }
                            NodePtr upstreamNode = upstreamElem->asA<Node>();
                            if (upstreamNode && upstreamNode->getType() == MATERIAL_TYPE_STRING)
                            {
                                std::unordered_set<NodePtr> newShaderNodes = getShaderNodes(upstreamNode, nodeType, target);
                                if (!newShaderNodes.empty())
                                {
                                    shaderNodes.insert(newShaderNodes.begin(), newShaderNodes.end());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (const InputPtr& input : inputs) 
    {
        // Scan for a node directly connected to the input.
        //
        const string& inputShader = input->getNodeName();
        if (!inputShader.empty())
        {
            NodePtr shaderNode = parent->getChildOfType<Node>(inputShader);
            if (shaderNode)
            {
                if (!nodeType.empty() && shaderNode->getType() != nodeType)
                {
                    continue;
                }
                
                if (!target.empty())
                {
                    NodeDefPtr nodeDef = shaderNode->getNodeDef(target);
                    if (!nodeDef)
                    {
                        continue;
                    }
                }
                shaderNodes.insert(shaderNode);
            }
        }

        // Check upstream nodegraph connected to the input.
        // If no explicit output name given then scan all outputs on the nodegraph.
        //
        else
        {
            const string& inputGraph = input->getNodeGraphString();
            if (!inputGraph.empty())
            {
                NodeGraphPtr nodeGraph = parent->getChildOfType<NodeGraph>(inputGraph);
                if (nodeGraph)
                {
                    const string& nodeGraphOutput = input->getOutputString();
                    std::vector<OutputPtr> outputs;
                    if (!nodeGraphOutput.empty())
                    {
                        outputs.push_back(nodeGraph->getOutput(nodeGraphOutput));
                    }
                    else
                    {
                        outputs = nodeGraph->getOutputs();
                    }
                    for (OutputPtr output : outputs)
                    {
                        NodePtr upstreamNode = output->getConnectedNode();
                        if (upstreamNode)
                        {
                            if (!target.empty())
                            {
                                NodeDefPtr nodeDef = upstreamNode->getNodeDef(target);
                                if (!nodeDef)
                                {
                                    continue;
                                }
                            }
                            shaderNodes.insert(upstreamNode);
                        }
                    }
                }
            }
        }
    }
    return shaderNodes;
}

vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom)
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : materialNode->getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterialNode() == materialNode)
            {
                if (geomStringsMatch(geom, matAssign->getActiveGeom()))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
                CollectionPtr coll = matAssign->getCollection();
                if (coll && coll->matchesGeomString(geom))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
            }
        }
    }
    return matAssigns;
}

vector<OutputPtr> getConnectedOutputs(const NodePtr& node)
{
    vector<OutputPtr> outputVec;
    std::set<OutputPtr> outputSet;
    for (InputPtr input : node->getInputs())
    {
        OutputPtr output = input->getConnectedOutput();
        if (output && !outputSet.count(output))
        {
            outputVec.push_back(output);
            outputSet.insert(output);
        }
    }
    return outputVec;
}

} // namespace MaterialX
