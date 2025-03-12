//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Document.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Return the NodeDef associated with a legacy ShaderRef element.
NodeDefPtr getShaderNodeDef(ElementPtr shaderRef)
{
    if (shaderRef->hasAttribute(NodeDef::NODE_DEF_ATTRIBUTE))
    {
        string nodeDefString = shaderRef->getAttribute(NodeDef::NODE_DEF_ATTRIBUTE);
        ConstDocumentPtr doc = shaderRef->getDocument();
        NodeDefPtr child = doc->getNodeDef(shaderRef->getQualifiedName(nodeDefString));
        return child ? child : doc->getNodeDef(nodeDefString);
    }
    if (shaderRef->hasAttribute(NodeDef::NODE_ATTRIBUTE))
    {
        string nodeString = shaderRef->getAttribute(NodeDef::NODE_ATTRIBUTE);
        string type = shaderRef->getAttribute(TypedElement::TYPE_ATTRIBUTE);
        string target = shaderRef->getAttribute(InterfaceElement::TARGET_ATTRIBUTE);
        string version = shaderRef->getAttribute(InterfaceElement::VERSION_ATTRIBUTE);
        vector<NodeDefPtr> nodeDefs = shaderRef->getDocument()->getMatchingNodeDefs(shaderRef->getQualifiedName(nodeString));
        vector<NodeDefPtr> secondary = shaderRef->getDocument()->getMatchingNodeDefs(nodeString);
        nodeDefs.insert(nodeDefs.end(), secondary.begin(), secondary.end());
        for (NodeDefPtr nodeDef : nodeDefs)
        {
            if (targetStringsMatch(nodeDef->getTarget(), target) &&
                nodeDef->isVersionCompatible(version) &&
                (type.empty() || nodeDef->getType() == type))
            {
                return nodeDef;
            }
        }
    }
    return NodeDefPtr();
}

// Copy an input between nodes, maintaining all existing bindings.
void copyInputWithBindings(NodePtr sourceNode, const string& sourceInputName,
                           NodePtr destNode, const string& destInputName)
{
    InputPtr sourceInput = sourceNode->getInput(sourceInputName);
    if (sourceInput)
    {
        InputPtr destInput = destNode->getInput(destInputName);
        if (!destInput)
        {
            destInput = destNode->addInput(destInputName, sourceInput->getType());
        }
        destInput->copyContentFrom(sourceInput);
    }
}

} // anonymous namespace

void Document::upgradeVersion()
{
    std::pair<int, int> documentVersion = getVersionIntegers();
    std::pair<int, int> expectedVersion(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION);
    if (documentVersion >= expectedVersion)
    {
        return;
    }
    int majorVersion = documentVersion.first;
    int minorVersion = documentVersion.second;

    // Upgrade from v1.22 to v1.23
    if (majorVersion == 1 && minorVersion == 22)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "vector")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Vector3>());
            }
        }
        minorVersion = 23;
    }

    // Upgrade from v1.23 to v1.24
    if (majorVersion == 1 && minorVersion == 23)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getCategory() == "shader" && elem->hasAttribute("shadername"))
            {
                elem->setAttribute(NodeDef::NODE_ATTRIBUTE, elem->getAttribute("shadername"));
                elem->removeAttribute("shadername");
            }
            for (ElementPtr child : getChildrenOfType<Element>("assign"))
            {
                elem->changeChildCategory(child, "materialassign");
            }
        }
        minorVersion = 24;
    }

    // Upgrade from v1.24 to v1.25
    if (majorVersion == 1 && minorVersion == 24)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->isA<Input>() && elem->hasAttribute("graphname"))
            {
                elem->setAttribute("opgraph", elem->getAttribute("graphname"));
                elem->removeAttribute("graphname");
            }
        }
        minorVersion = 25;
    }

    // Upgrade from v1.25 to v1.26
    if (majorVersion == 1 && minorVersion == 25)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getCategory() == "constant")
            {
                ElementPtr param = elem->getChild("color");
                if (param)
                {
                    param->setName("value");
                }
            }
        }
        minorVersion = 26;
    }

    // Upgrade from v1.26 to v1.34
    if (majorVersion == 1 && minorVersion == 26)
    {
        // Upgrade elements in place.
        for (ElementPtr elem : traverseTree())
        {
            ElementVec origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (child->getCategory() == "opgraph")
                {
                    elem->changeChildCategory(child, "nodegraph");
                }
                else if (child->getCategory() == "shader")
                {
                    NodeDefPtr nodeDef = elem->changeChildCategory(child, "nodedef")->asA<NodeDef>();
                    if (nodeDef->hasAttribute("shadertype"))
                    {
                        nodeDef->setType(SURFACE_SHADER_TYPE_STRING);
                    }
                    if (nodeDef->hasAttribute("shaderprogram"))
                    {
                        nodeDef->setNodeString(nodeDef->getAttribute("shaderprogram"));
                    }
                }
                else if (child->getCategory() == "shaderref")
                {
                    if (child->hasAttribute("shadertype"))
                    {
                        child->setAttribute(TypedElement::TYPE_ATTRIBUTE, SURFACE_SHADER_TYPE_STRING);
                        child->removeAttribute("shadertype");
                    }
                }
                else if (child->getCategory() == "parameter")
                {
                    if (child->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "opgraphnode")
                    {
                        if (elem->isA<Node>())
                        {
                            InputPtr input = elem->changeChildCategory(child, "input")->asA<Input>();
                            input->setNodeName(input->getAttribute("value"));
                            input->removeAttribute("value");
                            if (input->getConnectedNode())
                            {
                                input->setType(input->getConnectedNode()->getType());
                            }
                            else
                            {
                                input->setType(getTypeString<Color3>());
                            }
                        }
                        else if (elem->isA<Output>())
                        {
                            if (child->getName() == "in")
                            {
                                elem->setAttribute("nodename", child->getAttribute("value"));
                            }
                            elem->removeChild(child->getName());
                        }
                    }
                }
            }
        }

        // Assign nodedef names to shaderrefs.
        for (ElementPtr mat : getChildrenOfType<Element>("material"))
        {
            for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
            {
                if (!getShaderNodeDef(shaderRef))
                {
                    NodeDefPtr nodeDef = getNodeDef(shaderRef->getName());
                    if (nodeDef)
                    {
                        shaderRef->setAttribute(NodeDef::NODE_DEF_ATTRIBUTE, nodeDef->getName());
                        shaderRef->setAttribute(NodeDef::NODE_ATTRIBUTE, nodeDef->getNodeString());
                    }
                }
            }
        }

        // Move connections from nodedef inputs to bindinputs.
        ElementVec materials = getChildrenOfType<Element>("material");
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            for (InputPtr input : nodeDef->getActiveInputs())
            {
                if (input->hasAttribute("opgraph") && input->hasAttribute("graphoutput"))
                {
                    for (ElementPtr mat : materials)
                    {
                        for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
                        {
                            if (getShaderNodeDef(shaderRef) == nodeDef && !shaderRef->getChild(input->getName()))
                            {
                                ElementPtr bindInput = shaderRef->addChildOfCategory("bindinput", input->getName());
                                bindInput->setAttribute(TypedElement::TYPE_ATTRIBUTE, input->getType());
                                bindInput->setAttribute("nodegraph", input->getAttribute("opgraph"));
                                bindInput->setAttribute("output", input->getAttribute("graphoutput"));
                            }
                        }
                    }
                    input->removeAttribute("opgraph");
                    input->removeAttribute("graphoutput");
                }
            }
        }

        // Combine udim assignments into udim sets.
        for (GeomInfoPtr geomInfo : getGeomInfos())
        {
            for (ElementPtr child : geomInfo->getChildrenOfType<Element>("geomattr"))
            {
                geomInfo->changeChildCategory(child, "geomprop");
            }
        }
        if (getGeomPropValue("udim") && !getGeomPropValue("udimset"))
        {
            StringSet udimSet;
            for (GeomInfoPtr geomInfo : getGeomInfos())
            {
                for (GeomPropPtr geomProp : geomInfo->getGeomProps())
                {
                    if (geomProp->getName() == "udim")
                    {
                        udimSet.insert(geomProp->getValueString());
                    }
                }
            }

            string udimSetString;
            for (const string& udim : udimSet)
            {
                if (udimSetString.empty())
                {
                    udimSetString = udim;
                }
                else
                {
                    udimSetString += ", " + udim;
                }
            }

            GeomInfoPtr udimSetInfo = addGeomInfo();
            udimSetInfo->setGeomPropValue(UDIM_SET_PROPERTY, udimSetString, getTypeString<StringVec>());
        }

        minorVersion = 34;
    }

    // Upgrade from v1.34 to v1.35
    if (majorVersion == 1 && minorVersion == 34)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "matrix")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Matrix44>());
            }
            if (elem->hasAttribute("default") && !elem->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
            {
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, elem->getAttribute("default"));
                elem->removeAttribute("default");
            }

            MaterialAssignPtr matAssign = elem->asA<MaterialAssign>();
            if (matAssign)
            {
                matAssign->setMaterial(matAssign->getName());
            }
        }
        minorVersion = 35;
    }

    // Upgrade from v1.35 to v1.36
    if (majorVersion == 1 && minorVersion == 35)
    {
        for (ElementPtr elem : traverseTree())
        {
            LookPtr look = elem->asA<Look>();
            GeomInfoPtr geomInfo = elem->asA<GeomInfo>();

            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == GEOMNAME_TYPE_STRING &&
                elem->getAttribute(ValueElement::VALUE_ATTRIBUTE) == "*")
            {
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, UNIVERSAL_GEOM_NAME);
            }
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING)
            {
                StringMap stringMap;
                stringMap["%UDIM"] = UDIM_TOKEN;
                stringMap["%UVTILE"] = UV_TILE_TOKEN;
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, replaceSubstrings(elem->getAttribute(ValueElement::VALUE_ATTRIBUTE), stringMap));
            }

            ElementVec origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (elem->getCategory() == "material" && child->getCategory() == "override")
                {
                    for (ElementPtr shaderRef : elem->getChildrenOfType<Element>("shaderref"))
                    {
                        NodeDefPtr nodeDef = getShaderNodeDef(shaderRef);
                        if (nodeDef)
                        {
                            for (ValueElementPtr activeValue : nodeDef->getActiveValueElements())
                            {
                                if (activeValue->getAttribute("publicname") == child->getName() &&
                                    !shaderRef->getChild(child->getName()))
                                {
                                    if (activeValue->getCategory() == "parameter")
                                    {
                                        ElementPtr bindParam = shaderRef->addChildOfCategory("bindparam", activeValue->getName());
                                        bindParam->setAttribute(TypedElement::TYPE_ATTRIBUTE, activeValue->getType());
                                        bindParam->setAttribute(ValueElement::VALUE_ATTRIBUTE, child->getAttribute("value"));
                                    }
                                    else if (activeValue->isA<Input>())
                                    {
                                        ElementPtr bindInput = shaderRef->addChildOfCategory("bindinput", activeValue->getName());
                                        bindInput->setAttribute(TypedElement::TYPE_ATTRIBUTE, activeValue->getType());
                                        bindInput->setAttribute(ValueElement::VALUE_ATTRIBUTE, child->getAttribute("value"));
                                    }
                                }
                            }
                        }
                    }
                    elem->removeChild(child->getName());
                }
                else if (elem->getCategory() == "material" && child->getCategory() == "materialinherit")
                {
                    elem->setInheritString(child->getAttribute("material"));
                    elem->removeChild(child->getName());
                }
                else if (look && child->getCategory() == "lookinherit")
                {
                    elem->setInheritString(child->getAttribute("look"));
                    elem->removeChild(child->getName());
                }
            }
        }
        minorVersion = 36;
    }

    // Upgrade from 1.36 to 1.37
    if (majorVersion == 1 && minorVersion == 36)
    {
        // Convert type attributes to child outputs.
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            InterfaceElementPtr interfaceElem = std::static_pointer_cast<InterfaceElement>(nodeDef);
            if (interfaceElem && interfaceElem->hasType())
            {
                string type = interfaceElem->getAttribute(TypedElement::TYPE_ATTRIBUTE);
                OutputPtr outputPtr;
                if (!type.empty() && type != MULTI_OUTPUT_TYPE_STRING)
                {
                    outputPtr = interfaceElem->getOutput("out");
                    if (!outputPtr)
                    {
                        outputPtr = interfaceElem->addOutput("out", type);
                    }
                }
                interfaceElem->removeAttribute(TypedElement::TYPE_ATTRIBUTE);

                const string& defaultInput = interfaceElem->getAttribute(Output::DEFAULT_INPUT_ATTRIBUTE);
                if (outputPtr && !defaultInput.empty())
                {
                    outputPtr->setAttribute(Output::DEFAULT_INPUT_ATTRIBUTE, defaultInput);
                }
                interfaceElem->removeAttribute(Output::DEFAULT_INPUT_ATTRIBUTE);
            }
        }

        // Remove legacy shader nodedefs.
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            if (nodeDef->hasAttribute("shadertype"))
            {
                for (ElementPtr mat : getChildrenOfType<Element>("material"))
                {
                    for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
                    {
                        if (shaderRef->getAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE) == nodeDef->getName())
                        {
                            shaderRef->removeAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE);
                        }
                    }
                }
                removeNodeDef(nodeDef->getName());
            }
        }

        // Convert geometric attributes to geometric properties.
        for (GeomInfoPtr geomInfo : getGeomInfos())
        {
            for (ElementPtr child : geomInfo->getChildrenOfType<Element>("geomattr"))
            {
                geomInfo->changeChildCategory(child, "geomprop");
            }
        }
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }

            if (node->getCategory() == "geomattrvalue")
            {
                node->setCategory("geompropvalue");
                if (node->hasAttribute("attrname"))
                {
                    node->setAttribute("geomprop", node->getAttribute("attrname"));
                    node->removeAttribute("attrname");
                }
            }
        }

        vector<NodePtr> unusedNodes;
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }
            const string& nodeCategory = node->getCategory();

            // Change category from "invert to "invertmatrix" for matrix invert nodes
            if (nodeCategory == "invert" &&
                (node->getType() == getTypeString<Matrix33>() || node->getType() == getTypeString<Matrix44>()))
            {
                node->setCategory("invertmatrix");
            }

            // Change category from "rotate" to "rotate2d" or "rotate3d" nodes
            else if (nodeCategory == "rotate")
            {
                node->setCategory((node->getType() == getTypeString<Vector2>()) ? "rotate2d" : "rotate3d");
            }

            // Convert "compare" node to "ifgreatereq".
            else if (nodeCategory == "compare")
            {
                node->setCategory("ifgreatereq");
                InputPtr intest = node->getInput("intest");
                if (intest)
                {
                    intest->setName("value1");
                }
                ElementPtr cutoff = node->getChild("cutoff");
                if (cutoff)
                {
                    cutoff = node->changeChildCategory(cutoff, "input");
                    cutoff->setName("value2");
                }
                InputPtr in1 = node->getInput("in1");
                InputPtr in2 = node->getInput("in2");
                if (in1 && in2)
                {
                    in1->setName(createValidChildName("temp"));
                    in2->setName("in1");
                    in1->setName("in2");
                }
            }

            // Change nodes with category "transform[vector|point|normal]",
            // which are not fromspace/tospace variants, to "transformmatrix"
            else if (nodeCategory == "transformpoint" ||
                     nodeCategory == "transformvector" ||
                     nodeCategory == "transformnormal")
            {
                if (!node->getChild("fromspace") && !node->getChild("tospace"))
                {
                    node->setCategory("transformmatrix");
                }
            }

            // Convert "combine" to "combine2", "combine3" or "combine4"
            else if (nodeCategory == "combine")
            {
                if (node->getChild("in4"))
                {
                    node->setCategory("combine4");
                }
                else if (node->getChild("in3"))
                {
                    node->setCategory("combine3");
                }
                else
                {
                    node->setCategory("combine2");
                }
            }

            // Convert "separate" to "separate2", "separate3" or "separate4"
            else if (nodeCategory == "separate")
            {
                InputPtr in = node->getInput("in");
                if (in)
                {
                    const string& inType = in->getType();
                    if (inType == getTypeString<Vector4>() || inType == getTypeString<Color4>())
                    {
                        node->setCategory("separate4");
                    }
                    else if (inType == getTypeString<Vector3>() || inType == getTypeString<Color3>())
                    {
                        node->setCategory("separate3");
                    }
                    else
                    {
                        node->setCategory("separate2");
                    }
                }
            }

            // Convert backdrop nodes to backdrop elements
            else if (nodeCategory == "backdrop")
            {
                BackdropPtr backdrop = addBackdrop(node->getName());
                for (ElementPtr child : node->getChildrenOfType<Element>("parameter"))
                {
                    if (child->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
                    {
                        backdrop->setAttribute(child->getName(), child->getAttribute(ValueElement::VALUE_ATTRIBUTE));
                    }
                }
                unusedNodes.push_back(node);
            }
        }
        for (NodePtr node : unusedNodes)
        {
            node->getParent()->removeChild(node->getName());
        }

        minorVersion = 37;
    }

    // Upgrade from 1.37 to 1.38
    if (majorVersion == 1 && minorVersion == 37)
    {
        // Convert color2 types to vector2
        const StringMap COLOR2_CHANNEL_MAP = { { "r", "x" }, { "a", "y" } };
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "color2")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Vector2>());
                NodePtr parentNode = elem->getParent()->asA<Node>();
                if (!parentNode)
                {
                    continue;
                }

                for (PortElementPtr port : parentNode->getDownstreamPorts())
                {
                    if (port->hasAttribute("channels"))
                    {
                        string channels = port->getAttribute("channels");
                        channels = replaceSubstrings(channels, COLOR2_CHANNEL_MAP);
                        port->setAttribute("channels", channels);
                    }
                    if (port->hasOutputString())
                    {
                        string output = port->getOutputString();
                        output = replaceSubstrings(output, COLOR2_CHANNEL_MAP);
                        port->setOutputString(output);
                    }
                }

                ElementPtr channels = parentNode->getChild("channels");
                if (channels && channels->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
                {
                    string value = channels->getAttribute(ValueElement::VALUE_ATTRIBUTE);
                    value = replaceSubstrings(value, COLOR2_CHANNEL_MAP);
                    channels->setAttribute(ValueElement::VALUE_ATTRIBUTE, value);
                }
            }
        }

        // Convert material elements to material nodes
        for (ElementPtr mat : getChildrenOfType<Element>("material"))
        {
            NodePtr materialNode = nullptr;

            for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
            {
                NodeDefPtr nodeDef = getShaderNodeDef(shaderRef);

                // Get the shader node type and category, using the shader nodedef if present.
                string shaderNodeType = nodeDef ? nodeDef->getType() : SURFACE_SHADER_TYPE_STRING;
                string shaderNodeCategory = nodeDef ? nodeDef->getNodeString() : shaderRef->getAttribute(NodeDef::NODE_ATTRIBUTE);

                // Add the shader node.
                string shaderNodeName = createValidChildName(shaderRef->getName());
                NodePtr shaderNode = addNode(shaderNodeCategory, shaderNodeName, shaderNodeType);

                // Copy attributes to the shader node.
                string nodeDefString = shaderRef->getAttribute(NodeDef::NODE_DEF_ATTRIBUTE);
                string target = shaderRef->getAttribute(InterfaceElement::TARGET_ATTRIBUTE);
                string version = shaderRef->getAttribute(InterfaceElement::VERSION_ATTRIBUTE);
                if (!nodeDefString.empty())
                {
                    shaderNode->setNodeDefString(nodeDefString);
                }
                if (!target.empty())
                {
                    shaderNode->setTarget(target);
                }
                if (!version.empty())
                {
                    shaderNode->setVersionString(version);
                }
                shaderNode->setSourceUri(shaderRef->getSourceUri());

                // Copy child elements to the shader node.
                for (ElementPtr child : shaderRef->getChildren())
                {
                    ElementPtr newChild;
                    if (child->getCategory() == "bindinput" || child->getCategory() == "bindparam")
                    {
                        newChild = shaderNode->addInput(child->getName());
                    }
                    else if (child->getCategory() == "bindtoken")
                    {
                        newChild = shaderNode->addToken(child->getName());
                    }
                    if (newChild)
                    {
                        newChild->copyContentFrom(child);
                    }
                }

                // Create a material node if needed, making a connection to the new shader node.
                if (!materialNode)
                {
                    materialNode = addMaterialNode(createValidName("temp"), shaderNode);
                    materialNode->setSourceUri(mat->getSourceUri());
                }

                // Assign additional shader inputs to the material as needed.
                if (!materialNode->getInput(shaderNodeType))
                {
                    InputPtr shaderInput = materialNode->addInput(shaderNodeType, shaderNodeType);
                    shaderInput->setNodeName(shaderNode->getName());
                }
            }

            // Remove the material element, transferring its name and attributes to the material node.
            removeChild(mat->getName());
            if (materialNode)
            {
                materialNode->setName(mat->getName());
                for (const string& attr : mat->getAttributeNames())
                {
                    if (!materialNode->hasAttribute(attr))
                    {
                        materialNode->setAttribute(attr, mat->getAttribute(attr));
                    }
                }
            }
        }

        // Define BSDF node pairs.
        using StringPair = std::pair<string, string>;
        const StringPair DIELECTRIC_BRDF = { "dielectric_brdf", "dielectric_bsdf" };
        const StringPair DIELECTRIC_BTDF = { "dielectric_btdf", "dielectric_bsdf" };
        const StringPair GENERALIZED_SCHLICK_BRDF = { "generalized_schlick_brdf", "generalized_schlick_bsdf" };
        const StringPair CONDUCTOR_BRDF = { "conductor_brdf", "conductor_bsdf" };
        const StringPair SHEEN_BRDF = { "sheen_brdf", "sheen_bsdf" };
        const StringPair DIFFUSE_BRDF = { "diffuse_brdf", "oren_nayar_diffuse_bsdf" };
        const StringPair BURLEY_DIFFUSE_BRDF = { "burley_diffuse_brdf", "burley_diffuse_bsdf" };
        const StringPair DIFFUSE_BTDF = { "diffuse_btdf", "translucent_bsdf" };
        const StringPair SUBSURFACE_BRDF = { "subsurface_brdf", "subsurface_bsdf" };
        const StringPair THIN_FILM_BRDF = { "thin_film_brdf", "thin_film_bsdf" };

        // Function for upgrading old nested layering setup
        // to new setup with layer operators.
        auto upgradeBsdfLayering = [](NodePtr node)
        {
            InputPtr base = node->getInput("base");
            if (base)
            {
                NodePtr baseNode = base->getConnectedNode();
                if (baseNode)
                {
                    GraphElementPtr parent = node->getParent()->asA<GraphElement>();
                    // Rename the top bsdf node, and give its old name to the layer operator
                    // so we don't need to update any connection references.
                    const string oldName = node->getName();
                    node->setName(oldName + "__layer_top");
                    NodePtr layer = parent->addNode("layer", oldName, "BSDF");
                    InputPtr layerTop = layer->addInput("top", "BSDF");
                    InputPtr layerBase = layer->addInput("base", "BSDF");
                    layerTop->setConnectedNode(node);
                    layerBase->setConnectedNode(baseNode);
                }
                node->removeInput("base");
            }
        };

        // Storage for inputs found connected downstream from artistic_ior node.
        vector<InputPtr> artisticIorConnections, artisticExtConnections;

        // Update all nodes.
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }
            const string& nodeCategory = node->getCategory();
            if (nodeCategory == "atan2")
            {
                InputPtr input = node->getInput("in1");
                InputPtr input2 = node->getInput("in2");
                if (input && input2)
                {
                    input->setName(EMPTY_STRING);
                    input2->setName("in1");
                    input->setName("in2");
                }
                else
                {
                    if (input)
                    {
                        input->setName("in2");
                    }
                    if (input2)
                    {
                        input2->setName("in1");
                    }
                }
            }
            else if (nodeCategory == "rotate3d")
            {
                ElementPtr axis = node->getChild("axis");
                if (axis)
                {
                    node->changeChildCategory(axis, "input");
                }
            }
            else if (nodeCategory == DIELECTRIC_BRDF.first)
            {
                node->setCategory(DIELECTRIC_BRDF.second);
                upgradeBsdfLayering(node);
            }
            else if (nodeCategory == DIELECTRIC_BTDF.first)
            {
                node->setCategory(DIELECTRIC_BTDF.second);
                node->removeInput("interior");
                InputPtr mode = node->addInput("scatter_mode", STRING_TYPE_STRING);
                mode->setValueString("T");
            }
            else if (nodeCategory == GENERALIZED_SCHLICK_BRDF.first)
            {
                node->setCategory(GENERALIZED_SCHLICK_BRDF.second);
                upgradeBsdfLayering(node);
            }
            else if (nodeCategory == SHEEN_BRDF.first)
            {
                node->setCategory(SHEEN_BRDF.second);
                upgradeBsdfLayering(node);
            }
            else if (nodeCategory == THIN_FILM_BRDF.first)
            {
                node->setCategory(THIN_FILM_BRDF.second);
                upgradeBsdfLayering(node);
            }
            else if (nodeCategory == CONDUCTOR_BRDF.first)
            {
                node->setCategory(CONDUCTOR_BRDF.second);

                // Create an artistic_ior node to convert from artistic to physical parameterization.
                GraphElementPtr parent = node->getParent()->asA<GraphElement>();
                NodePtr artisticIor = parent->addNode("artistic_ior", node->getName() + "__artistic_ior", "multioutput");
                OutputPtr artisticIor_ior = artisticIor->addOutput("ior", "color3");
                OutputPtr artisticIor_extinction = artisticIor->addOutput("extinction", "color3");

                // Copy inputs and bindings from conductor node to artistic_ior node.
                copyInputWithBindings(node, "reflectivity", artisticIor, "reflectivity");
                copyInputWithBindings(node, "edge_color", artisticIor, "edge_color");

                // Update the parameterization on the conductor node
                // and connect it to the artistic_ior node.
                node->removeInput("reflectivity");
                node->removeInput("edge_color");
                InputPtr ior = node->addInput("ior", "color3");
                ior->setNodeName(artisticIor->getName());
                ior->setOutputString(artisticIor_ior->getName());
                InputPtr extinction = node->addInput("extinction", "color3");
                extinction->setNodeName(artisticIor->getName());
                extinction->setOutputString(artisticIor_extinction->getName());
            }
            else if (nodeCategory == DIFFUSE_BRDF.first)
            {
                node->setCategory(DIFFUSE_BRDF.second);
            }
            else if (nodeCategory == BURLEY_DIFFUSE_BRDF.first)
            {
                node->setCategory(BURLEY_DIFFUSE_BRDF.second);
            }
            else if (nodeCategory == DIFFUSE_BTDF.first)
            {
                node->setCategory(DIFFUSE_BTDF.second);
            }
            else if (nodeCategory == SUBSURFACE_BRDF.first)
            {
                node->setCategory(SUBSURFACE_BRDF.second);
            }
            else if (nodeCategory == "artistic_ior")
            {
                OutputPtr ior = node->getOutput("ior");
                if (ior)
                {
                    ior->setType("color3");
                }
                OutputPtr extinction = node->getOutput("extinction");
                if (extinction)
                {
                    extinction->setType("color3");
                }
            }

            // Search for connections to artistic_ior with vector3 type.
            // If found we must insert a conversion node color3->vector3
            // since the outputs of artistic_ior is now color3.
            // Save the inputs here and insert the conversion nodes below,
            // since we can't modify the graph while traversing it.
            for (InputPtr input : node->getInputs())
            {
                if (input->getOutputString() == "ior" && input->getType() == "vector3")
                {
                    NodePtr connectedNode = input->getConnectedNode();
                    if (connectedNode && connectedNode->getCategory() == "artistic_ior")
                    {
                        artisticIorConnections.push_back(input);
                    }
                }
                else if (input->getOutputString() == "extinction" && input->getType() == "vector3")
                {
                    NodePtr connectedNode = input->getConnectedNode();
                    if (connectedNode && connectedNode->getCategory() == "artistic_ior")
                    {
                        artisticExtConnections.push_back(input);
                    }
                }
            }
        }

        // Insert conversion nodes for artistic_ior connections found above.
        for (InputPtr input : artisticIorConnections)
        {
            NodePtr artisticIorNode = input->getConnectedNode();
            ElementPtr node = input->getParent();
            GraphElementPtr parent = node->getParent()->asA<GraphElement>();
            NodePtr convert = parent->addNode("convert", node->getName() + "__convert_ior", "vector3");
            InputPtr convertInput = convert->addInput("in", "color3");
            convertInput->setNodeName(artisticIorNode->getName());
            convertInput->setOutputString("ior");
            input->setNodeName(convert->getName());
            input->removeAttribute(PortElement::OUTPUT_ATTRIBUTE);
        }
        for (InputPtr input : artisticExtConnections)
        {
            NodePtr artisticIorNode = input->getConnectedNode();
            ElementPtr node = input->getParent();
            GraphElementPtr parent = node->getParent()->asA<GraphElement>();
            NodePtr convert = parent->addNode("convert", node->getName() + "__convert_extinction", "vector3");
            InputPtr convertInput = convert->addInput("in", "color3");
            convertInput->setNodeName(artisticIorNode->getName());
            convertInput->setOutputString("extinction");
            input->setNodeName(convert->getName());
            input->removeAttribute(PortElement::OUTPUT_ATTRIBUTE);
        }

        // Make it so that interface names and nodes in a nodegraph are not duplicates
        // If they are, rename the nodes.
        for (NodeGraphPtr nodegraph : getNodeGraphs())
        {
            // Clear out any erroneously set version
            nodegraph->removeAttribute(InterfaceElement::VERSION_ATTRIBUTE);

            StringSet interfaceNames;
            for (auto child : nodegraph->getChildren())
            {
                NodePtr node = child->asA<Node>();
                if (node)
                {
                    for (ValueElementPtr elem : node->getChildrenOfType<ValueElement>())
                    {
                        const string& interfaceName = elem->getInterfaceName();
                        if (!interfaceName.empty())
                        {
                            interfaceNames.insert(interfaceName);
                        }
                    }
                }
            }
            for (string interfaceName : interfaceNames)
            {
                NodePtr node = nodegraph->getNode(interfaceName);
                if (node)
                {
                    string newNodeName = nodegraph->createValidChildName(interfaceName);
                    vector<MaterialX::PortElementPtr> downstreamPorts = node->getDownstreamPorts();
                    for (MaterialX::PortElementPtr downstreamPort : downstreamPorts)
                    {
                        if (downstreamPort->getNodeName() == interfaceName)
                        {
                            downstreamPort->setNodeName(newNodeName);
                        }
                    }
                    node->setName(newNodeName);
                }
            }
        }

        // Convert parameters to inputs, applying uniform markings to converted inputs
        // of nodedefs.
        for (ElementPtr elem : traverseTree())
        {
            if (elem->isA<InterfaceElement>())
            {
                for (ElementPtr param : elem->getChildrenOfType<Element>("parameter"))
                {
                    InputPtr input = elem->changeChildCategory(param, "input")->asA<Input>();
                    if (elem->isA<NodeDef>())
                    {
                        input->setIsUniform(true);
                    }
                }
            }
        }

        minorVersion = 38;
    }

    // Upgrade from 1.38 to 1.39
    if (majorVersion == 1 && minorVersion == 38)
    {
        const std::unordered_map<char, size_t> CHANNEL_INDEX_MAP =
        {
            { 'r', 0 }, { 'g', 1 }, { 'b', 2 }, { 'a', 3 },
            { 'x', 0 }, { 'y', 1 }, { 'z', 2 }, { 'w', 3 }
        };
        const std::unordered_map<char, float> CHANNEL_CONSTANT_MAP =
        {
            { '0', 0.0f }, { '1', 1.0f }
        };
        const std::unordered_map<string, size_t> CHANNEL_COUNT_MAP =
        {
            { "float", 1 },
            { "color3", 3 }, { "color4", 4 },
            { "vector2", 2 }, { "vector3", 3 }, { "vector4", 4 }
        };
        const std::array<std::pair<string, size_t>, 10> CHANNEL_CONVERT_PATTERNS =
        { {
            { "rgb", 3 }, { "rgb", 4 }, { "rgba", 4 },
            { "xyz", 3 }, { "xyz", 4 }, { "xyzw", 4 },
            { "rr", 1 }, { "rrr", 1 },
            { "xx", 1 }, { "xxx", 1 }
        } };
        const std::array<std::pair<StringSet, string>, 3> CHANNEL_ATTRIBUTE_PATTERNS =
        { {
            { { "xx", "xxx", "xxxx" }, "float" },
            { { "xyz", "x", "y", "z" }, "vector3" },
            { { "rgba", "a" }, "color4" }
        } };

        // Convert channels attributes to legacy swizzle nodes, which are then converted
        // to modern nodes in a second pass.
        for (ElementPtr elem : traverseTree())
        {
            PortElementPtr port = elem->asA<PortElement>();
            if (!port)
            {
                continue;
            }

            const string& channelString = port->getAttribute("channels");
            if (channelString.empty())
            {
                continue;
            }

            // Determine the upstream type.
            ElementPtr parent = port->getParent();
            GraphElementPtr graph = port->getAncestorOfType<GraphElement>();
            NodePtr upstreamNode = port->getConnectedNode();
            string upstreamType = upstreamNode ? upstreamNode->getType() : EMPTY_STRING;
            if (upstreamType.empty() || upstreamType == MULTI_OUTPUT_TYPE_STRING)
            {
                for (const auto& pair : CHANNEL_ATTRIBUTE_PATTERNS)
                {
                    if (pair.first.count(channelString))
                    {
                        upstreamType = pair.second;
                        break;
                    }
                }
                if (upstreamType.empty() || upstreamType == MULTI_OUTPUT_TYPE_STRING)
                {
                    upstreamType = (port->getType() == "color3") ? "color4" : "color3";
                }
            }

            // Ignore the channels string for purely scalar connections.
            if (upstreamType == getTypeString<float>() && port->getType() == getTypeString<float>())
            {
                port->removeAttribute("channels");
                continue;
            }

            // Create the new swizzle node.
            NodePtr swizzleNode = graph->addNode("swizzle", graph->createValidChildName("swizzle"), port->getType());
            int childIndex = (parent->getParent() == graph) ? graph->getChildIndex(parent->getName()) : graph->getChildIndex(port->getName());
            if (childIndex != -1)
            {
                graph->setChildIndex(swizzleNode->getName(), childIndex);
            }
            InputPtr in = swizzleNode->addInput("in");
            in->copyContentFrom(port);
            in->removeAttribute("channels");
            in->setType(upstreamType);
            swizzleNode->setInputValue("channels", channelString);

            // Connect the original port to this node.
            port->setConnectedNode(swizzleNode);
            port->removeAttribute(PortElement::OUTPUT_ATTRIBUTE);
            port->removeAttribute(PortElement::INTERFACE_NAME_ATTRIBUTE);
            port->removeAttribute("channels");

            // Update any nodegraph reference
            if (graph)
            {
                const string& portNodeGraphString = port->getNodeGraphString();
                if (!portNodeGraphString.empty())
                {
                    const string& graphName = graph->getName();
                    if (graphName.empty())
                    {
                        port->removeAttribute(PortElement::NODE_GRAPH_ATTRIBUTE);
                    }
                    else if (graphName != portNodeGraphString)
                    {
                        port->setNodeGraphString(graphName);
                    }
                }
            }
        }

        // Update all nodes.
        vector<NodePtr> unusedNodes;
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }
            const string& nodeCategory = node->getCategory();
            if (nodeCategory == "layer")
            {
                // Convert layering of thin_film_bsdf nodes to thin-film parameters on the affected BSDF nodes.
                NodePtr top = node->getConnectedNode("top");
                NodePtr base = node->getConnectedNode("base");
                if (top && base && top->getCategory() == "thin_film_bsdf")
                {
                    // Apply thin-film parameters to all supported BSDF's upstream.
                    const StringSet BSDF_WITH_THINFILM = { "dielectric_bsdf", "conductor_bsdf", "generalized_schlick_bsdf" };
                    for (Edge edge : node->traverseGraph())
                    {
                        NodePtr upstream = edge.getUpstreamElement()->asA<Node>();
                        if (upstream && BSDF_WITH_THINFILM.count(upstream->getCategory()))
                        {
                            InputPtr scatterMode = upstream->getInput("scatter_mode");
                            if (!scatterMode || scatterMode->getValueString() != "T")
                            {
                                copyInputWithBindings(top, "thickness", upstream, "thinfilm_thickness");
                                copyInputWithBindings(top, "ior", upstream, "thinfilm_ior");
                            }
                        }
                    }

                    // Bypass the thin-film layer operator.
                    vector<MaterialX::PortElementPtr> downstreamPorts = node->getDownstreamPorts();
                    for (auto port : downstreamPorts)
                    {
                        port->setNodeName(base->getName());
                    }

                    // Mark original nodes as unused.
                    unusedNodes.push_back(node);
                    unusedNodes.push_back(top);
                }
            }
            else if (nodeCategory == "subsurface_bsdf")
            {
                InputPtr radiusInput = node->getInput("radius");
                if (radiusInput && radiusInput->getType() == "vector3")
                {
                    GraphElementPtr graph = node->getAncestorOfType<GraphElement>();
                    NodePtr convertNode = graph->addNode("convert", graph->createValidChildName("convert"), "color3");
                    copyInputWithBindings(node, "radius", convertNode, "in");
                    radiusInput->setConnectedNode(convertNode);
                    radiusInput->setType("color3");
                }
            }
            else if (nodeCategory == "switch")
            {
                // Upgrade switch nodes from 5 to 10 inputs, handling the fallback behavior for
                // constant "which" values that were previously out of range.
                InputPtr which = node->getInput("which");
                if (which && which->hasValue())
                {
                    auto whichValue = which->getValue();
                    if (whichValue->isA<int>() && whichValue->asA<int>() >= 5)
                    {
                        which->setValue(0);
                    }
                    else if (whichValue->isA<float>() && whichValue->asA<float>() >= 5)
                    {
                        which->setValue(0.0);
                    }
                }
            }
            else if (nodeCategory == "swizzle")
            {
                InputPtr inInput = node->getInput("in");
                InputPtr channelsInput = node->getInput("channels");
                if (inInput &&
                    CHANNEL_COUNT_MAP.count(inInput->getType()) &&
                    CHANNEL_COUNT_MAP.count(node->getType()))
                {
                    string channelString = channelsInput ? channelsInput->getValueString() : EMPTY_STRING;
                    string sourceType = inInput->getType();
                    string destType = node->getType();
                    size_t sourceChannelCount = CHANNEL_COUNT_MAP.at(sourceType);
                    size_t destChannelCount = CHANNEL_COUNT_MAP.at(destType);

                    // Resolve the invalid case of having both a connection and a value
                    // by removing the value attribute.
                    if (inInput->hasValue())
                    {
                        if (inInput->hasNodeName() || inInput->hasNodeGraphString() || inInput->hasInterfaceName())
                        {
                            inInput->removeAttribute(ValueElement::VALUE_ATTRIBUTE);
                        }
                    }

                    if (inInput->hasValue())
                    {
                        // Replace swizzle with constant.
                        node->setCategory("constant");
                        string valueString = inInput->getValueString();
                        StringVec origValueTokens = splitString(valueString, ARRAY_VALID_SEPARATORS);
                        StringVec newValueTokens;
                        for (size_t i = 0; i < destChannelCount; i++)
                        {
                            if (i < channelString.size())
                            {
                                if (CHANNEL_INDEX_MAP.count(channelString[i]))
                                {
                                    size_t index = CHANNEL_INDEX_MAP.at(channelString[i]);
                                    if (index < origValueTokens.size())
                                    {
                                        newValueTokens.push_back(origValueTokens[index]);
                                    }
                                }
                                else if (CHANNEL_CONSTANT_MAP.count(channelString[i]))
                                {
                                    newValueTokens.push_back(std::to_string(CHANNEL_CONSTANT_MAP.at(channelString[i])));
                                }
                                else
                                {
                                    newValueTokens.push_back(origValueTokens[0]);
                                }
                            }
                            else
                            {
                                newValueTokens.push_back(origValueTokens[0]);
                            }
                        }
                        InputPtr valueInput = node->addInput("value", node->getType());
                        valueInput->setValueString(joinStrings(newValueTokens, ", "));
                        node->removeInput(inInput->getName());
                    }
                    else if (destChannelCount == 1)
                    {
                        // Replace swizzle with extract.
                        node->setCategory("extract");
                        if (!channelString.empty() && CHANNEL_INDEX_MAP.count(channelString[0]))
                        {
                            node->setInputValue("index", (int) CHANNEL_INDEX_MAP.at(channelString[0]));
                        }
                    }
                    else if (sourceType != destType && std::find(CHANNEL_CONVERT_PATTERNS.begin(), CHANNEL_CONVERT_PATTERNS.end(),
                             std::make_pair(channelString, sourceChannelCount)) != CHANNEL_CONVERT_PATTERNS.end())
                    {
                        // Replace swizzle with convert.
                        node->setCategory("convert");
                    }
                    else if (sourceChannelCount == 1)
                    {
                        // Replace swizzle with combine.
                        node->setCategory("combine" + std::to_string(destChannelCount));
                        for (size_t i = 0; i < destChannelCount; i++)
                        {
                            InputPtr combineInInput = node->addInput(std::string("in") + std::to_string(i + 1), "float");
                            if (i < channelString.size() && CHANNEL_CONSTANT_MAP.count(channelString[i]))
                            {
                                combineInInput->setValue(CHANNEL_CONSTANT_MAP.at(channelString[i]));
                            }
                            else
                            {
                                copyInputWithBindings(node, inInput->getName(), node, combineInInput->getName());
                            }
                        }
                        node->removeInput(inInput->getName());
                    }
                    else
                    {
                        // Replace swizzle with separate and combine.
                        GraphElementPtr graph = node->getAncestorOfType<GraphElement>();
                        NodePtr separateNode = graph->addNode(std::string("separate") + std::to_string(sourceChannelCount),
                                                              graph->createValidChildName("separate"), MULTI_OUTPUT_TYPE_STRING);
                        int childIndex = graph->getChildIndex(node->getName());
                        if (childIndex != -1)
                        {
                            graph->setChildIndex(separateNode->getName(), childIndex);
                        }
                        node->setCategory("combine" + std::to_string(destChannelCount));
                        for (size_t i = 0; i < destChannelCount; i++)
                        {
                            InputPtr combineInInput = node->addInput(std::string("in") + std::to_string(i + 1), "float");
                            if (i < channelString.size())
                            {
                                if (CHANNEL_INDEX_MAP.count(channelString[i]))
                                {
                                    combineInInput->setConnectedNode(separateNode);
                                    combineInInput->setOutputString(std::string("out") + channelString[i]);
                                }
                                else if (CHANNEL_CONSTANT_MAP.count(channelString[i]))
                                {
                                    combineInInput->setValue(CHANNEL_CONSTANT_MAP.at(channelString[i]));
                                }
                            }
                            else
                            {
                                combineInInput->setConnectedNode(separateNode);
                                combineInInput->setOutputString(combineInInput->isColorType() ? "outr" : "outx");
                            }
                        }
                        copyInputWithBindings(node, inInput->getName(), separateNode, "in");
                        node->removeInput(inInput->getName());
                    }

                    // Remove the channels input from the converted node.
                    if (channelsInput)
                    {
                        node->removeInput(channelsInput->getName());
                    }
                }
            }
            else if (nodeCategory == "atan2")
            {
                InputPtr input1 = node->getInput("in1");
                if (input1)
                {
                    input1->setName("iny");
                }
                InputPtr input2 = node->getInput("in2");
                if (input2)
                {
                    input2->setName("inx");
                }
            }
            else if (nodeCategory == "normalmap")
            {
                InputPtr space = node->getInput("space");
                if (space && space->getValueString() == "object")
                {
                    // Replace object-space normalmap with a graph.
                    GraphElementPtr graph = node->getAncestorOfType<GraphElement>();
                    NodePtr multiply = graph->addNode("multiply", graph->createValidChildName("multiply"), "vector3");
                    copyInputWithBindings(node, "in", multiply, "in1");
                    multiply->setInputValue("in2", 2.0f);
                    NodePtr subtract = graph->addNode("subtract", graph->createValidChildName("subtract"), "vector3");
                    subtract->addInput("in1", "vector3")->setConnectedNode(multiply);
                    subtract->setInputValue("in2", 1.0f);
                    node->setCategory("normalize");
                    vector<InputPtr> origInputs = node->getInputs();
                    for (InputPtr input : origInputs)
                    {
                        node->removeChild(input->getName());
                    }
                    node->addInput("in", "vector3")->setConnectedNode(subtract);

                    // Update nodedef name if present.
                    if (node->hasNodeDefString())
                    {
                        node->setNodeDefString("ND_normalize_vector3");
                    }
                }
                else
                {
                    // Clear tangent-space input.
                    node->removeInput("space");

                    // If the normal or tangent inputs are set and the bitangent input is not, 
                    // the bitangent should be set to normalize(cross(N, T))
                    InputPtr normalInput = node->getInput("normal");
                    InputPtr tangentInput = node->getInput("tangent");
                    InputPtr bitangentInput = node->getInput("bitangent");
                    if ((normalInput || tangentInput) && !bitangentInput)
                    {
                        GraphElementPtr graph = node->getAncestorOfType<GraphElement>();
                        NodePtr crossNode = graph->addNode("crossproduct", graph->createValidChildName("normalmap_cross"), "vector3");
                        copyInputWithBindings(node, "normal", crossNode, "in1");
                        copyInputWithBindings(node, "tangent", crossNode, "in2");

                        NodePtr normalizeNode = graph->addNode("normalize", graph->createValidChildName("normalmap_cross_norm"), "vector3");
                        normalizeNode->addInput("in", "vector3")->setConnectedNode(crossNode);

                        node->addInput("bitangent", "vector3")->setConnectedNode(normalizeNode);
                    }

                    // Update nodedef name if present.
                    if (node->getNodeDefString() == "ND_normalmap")
                    {
                        node->setNodeDefString("ND_normalmap_float");
                    }
                }
            }
        }
        for (NodePtr node : unusedNodes)
        {
            node->getParent()->removeChild(node->getName());
        }

        minorVersion = 39;
    }

    std::pair<int, int> upgradedVersion(majorVersion, minorVersion);
    if (upgradedVersion == expectedVersion)
    {
        setVersionIntegers(majorVersion, minorVersion);
    }
}

MATERIALX_NAMESPACE_END
