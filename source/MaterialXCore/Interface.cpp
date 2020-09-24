//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>

namespace MaterialX
{

const string PortElement::NODE_NAME_ATTRIBUTE = "nodename";
const string PortElement::NODE_GRAPH_ATTRIBUTE = "nodegraph";
const string PortElement::OUTPUT_ATTRIBUTE = "output";
const string PortElement::CHANNELS_ATTRIBUTE = "channels";
const string InterfaceElement::NODE_DEF_ATTRIBUTE = "nodedef";
const string Input::DEFAULT_GEOM_PROP_ATTRIBUTE = "defaultgeomprop";
const string Output::DEFAULT_INPUT_ATTRIBUTE = "defaultinput";

// Map from type strings to swizzle pattern character sets.
const std::unordered_map<string, CharSet> PortElement::CHANNELS_CHARACTER_SET =
{
    { "float", { '0', '1', 'r', 'x' } },
    { "color2", { '0', '1', 'r', 'a' } },
    { "color3", { '0', '1', 'r', 'g', 'b' } },
    { "color4", { '0', '1', 'r', 'g', 'b', 'a' } },
    { "vector2", { '0', '1', 'x', 'y' } },
    { "vector3", { '0', '1', 'x', 'y', 'z' } },
    { "vector4", { '0', '1', 'x', 'y', 'z', 'w' } }
};

// Map from type strings to swizzle pattern lengths.
const std::unordered_map<string, size_t> PortElement::CHANNELS_PATTERN_LENGTH =
{
    { "float", 1 },
    { "color2", 2 },
    { "color3", 3 },
    { "color4", 4 },
    { "vector2", 2 },
    { "vector3", 3 },
    { "vector4", 4 }
};

//
// PortElement methods
//

void PortElement::setConnectedNode(NodePtr node)
{
    if (node)
    {
        setNodeName(node->getName());
    }
    else
    {
        removeAttribute(NODE_NAME_ATTRIBUTE);
    }
}

NodePtr PortElement::getConnectedNode() const
{
    ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
    return graph ? graph->getNode(getNodeName()) : nullptr;
}

void PortElement::setConnectedOutput(ConstOutputPtr output)
{
    if (output)
    {
        setOutputString(output->getName());
        ConstElementPtr parent = output->getParent();
        if (parent->isA<NodeGraph>())
        {
            setNodeGraphString(parent->getName());
        }
        else
        {
            removeAttribute(NODE_GRAPH_ATTRIBUTE);
        }
    }
    else
    {
        removeAttribute(OUTPUT_ATTRIBUTE);
        removeAttribute(NODE_GRAPH_ATTRIBUTE);
    }
}

OutputPtr PortElement::getConnectedOutput() const
{
    if (hasNodeGraphString())
    {
        NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeGraphString());
        return nodeGraph ? nodeGraph->getOutput(getOutputString()) : OutputPtr();
    }
    return getDocument()->getOutput(getOutputString());
}

bool PortElement::validate(string* message) const
{
    bool res = true;

    NodePtr connectedNode = getConnectedNode();
    if (hasNodeName() || hasOutputString())
    {
        NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeName());
        if (!nodeGraph)
        {
            validateRequire(connectedNode != nullptr, res, message, "Invalid port connection");
        }
    }
    if (connectedNode)
    {
        const string& outputString = getOutputString();
        if (!outputString.empty())
        {
            OutputPtr output = OutputPtr();
            if (hasNodeName())
            {
                NodeDefPtr nodeDef = connectedNode->getNodeDef();
                output = nodeDef ? nodeDef->getOutput(outputString) : OutputPtr();
                if (output)
                {
                    validateRequire(connectedNode->getType() == MULTI_OUTPUT_TYPE_STRING, res, message,
                        "Multi-output type expected in port connection");
                }
            }
            else if (hasNodeGraphString())
            {
                NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeGraphString());
                if (nodeGraph)
                {
                    output = nodeGraph->getOutput(outputString);
                    if (nodeGraph->getNodeDef())
                    {
                        validateRequire(nodeGraph->getOutputCount() > 1, res, message,
                            "Multi-output type expected in port connection");
                    }
                }
            }
            else
            {
                // Document has no concept of a multioutput type
                output = getDocument()->getOutput(outputString);
            }
            validateRequire(output != nullptr, res, message, "No output found for port connection");

            if (output)
            {
                if (hasChannels())
                {
                    bool valid = validChannelsString(getChannels(), output->getType(), getType());
                    validateRequire(valid, res, message, "Invalid channels string in port connection");
                }
                else
                {
                    validateRequire(getType() == output->getType(), res, message, "Mismatched types in port connection");
                }
            }
        }
        else if (hasChannels())
        {
            bool valid = validChannelsString(getChannels(), connectedNode->getType(), getType());
            validateRequire(valid, res, message, "Invalid channels string in port connection");
        }
        else
        {
            validateRequire(getType() == connectedNode->getType(), res, message, "Mismatched types in port connection");
        }
    }
    return ValueElement::validate(message) && res;
}

bool PortElement::validChannelsCharacters(const string& channels, const string& sourceType)
{
    if (!CHANNELS_CHARACTER_SET.count(sourceType))
    {
        return false;
    }
    const CharSet& validCharSet = CHANNELS_CHARACTER_SET.at(sourceType);
    for (const char& channelChar : channels)
    {
        if (!validCharSet.count(channelChar))
        {
            return false;
        }
    }
    return true;
}

bool PortElement::validChannelsString(const string& channels, const string& sourceType, const string& destinationType)
{
    if (!validChannelsCharacters(channels, sourceType))
    {
        return false;
    }
    if (!CHANNELS_PATTERN_LENGTH.count(destinationType) ||
        CHANNELS_PATTERN_LENGTH.at(destinationType) != channels.size())
    {
        return false;
    }

    return true;
}

//
// Parameter methods
//

Edge Parameter::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (material && index < getUpstreamEdgeCount())
    {
        ConstElementPtr parent = getParent();
        ConstInterfaceElementPtr interface = parent ? parent->asA<InterfaceElement>() : nullptr;
        ConstNodeDefPtr nodeDef = interface ? interface->getDeclaration() : nullptr;
        if (nodeDef)
        {
            // Apply BindParam elements to the Parameter.
            for (ShaderRefPtr shaderRef : material->getActiveShaderRefs())
            {
                if (shaderRef->getNodeDef()->hasInheritedBase(nodeDef))
                {
                    for (BindParamPtr bindParam : shaderRef->getBindParams())
                    {
                        if (bindParam->getName() == getName() && bindParam->hasValue())
                        {
                            return Edge(getSelfNonConst(), nullptr, bindParam);
                        }
                    }
                }
            }
        }
    }

    return NULL_EDGE;
}

OutputPtr Parameter::getConnectedOutput() const
{
    OutputPtr output = OutputPtr();

    const string& outputString = getAttribute(PortElement::OUTPUT_ATTRIBUTE);
    if (!outputString.empty())
    {
        const string connectedGraph = getAttribute(PortElement::NODE_GRAPH_ATTRIBUTE);
        const string connectedNode = getAttribute(PortElement::NODE_NAME_ATTRIBUTE);

        // Look for an output in a nodegraph
        if (!connectedGraph.empty())
        {
            NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(connectedGraph);
            if (nodeGraph)
            {
                output = nodeGraph->getOutput(outputString);
            }
        }
        // Look for output on a node within a nodegraph
        else if (!connectedNode.empty())
        {
            ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
            NodePtr node = graph ? graph->getNode(connectedNode) : nullptr;
            if (node)
            {
                output = node->getOutput(outputString);
            }
        }
        // Look for a document level output
        else
        {
            output = getDocument()->getOutput(outputString);
        }
    }
    return output;
}

NodePtr Parameter::getConnectedNode() const
{
    OutputPtr output = getConnectedOutput();
    if (output)
    {
        return output->getConnectedNode();
    }
    const string connectedNode = getAttribute(PortElement::NODE_NAME_ATTRIBUTE);
    if (!connectedNode.empty())
    {
        ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
        NodePtr node = graph ? graph->getNode(connectedNode) : nullptr;
        if (node)
        {
            return node;
        }
    }
    return nullptr;
}

//
// Input methods
//

OutputPtr Input::getConnectedOutput() const
{
    const string& outputString = getOutputString();
    OutputPtr result = nullptr;

    // Look for an output in a nodegraph
    if (hasNodeGraphString())
    {
        NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeGraphString());
        if (nodeGraph)
        {
            std::vector<OutputPtr> outputs = nodeGraph->getOutputs();
            if (!outputs.empty())
            {
                if (outputString.empty())
                {
                    result = outputs[0];
                }
                else
                {
                    result = nodeGraph->getOutput(outputString);
                }
            }
        }
    }
    // Look for output on a node
    else if (hasNodeName())
    {
        ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
        NodePtr node = graph ? graph->getNode(getNodeName()) : nullptr;
        if (node)
        {
            std::vector<OutputPtr> outputs = node->getOutputs();
            if (!outputs.empty())
            {
                if (outputString.empty())
                {
                    result = outputs[0];
                }
                else
                {
                    result = node->getOutput(outputString);
                }
            }
        }
    }
    // Look for output in the document
    if (!result)
    {
        result = getDocument()->getOutput(outputString);
    }
    return result;
}

NodePtr Input::getConnectedNode() const
{
    OutputPtr output = getConnectedOutput();
    if (output)
    {
        NodePtr node = output->getConnectedNode();
        if (node)
            return node;
    }
    if (hasNodeName())
    {
        ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
        NodePtr node = graph ? graph->getNode(getNodeName()) : nullptr;
        if (node)
        {
            return node;
        }
    }
    return PortElement::getConnectedNode();
}

Edge Input::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (material && index < getUpstreamEdgeCount())
    {
        ConstElementPtr parent = getParent();
        ConstInterfaceElementPtr interface = parent ? parent->asA<InterfaceElement>() : nullptr;
        ConstNodeDefPtr nodeDef = interface ? interface->getDeclaration() : nullptr;
        if (nodeDef)
        {
            // Apply BindInput elements to the Input.
            for (ShaderRefPtr shaderRef : material->getActiveShaderRefs())
            {
                if (shaderRef->getNodeDef()->hasInheritedBase(nodeDef))
                {
                    for (BindInputPtr bindInput : shaderRef->getBindInputs())
                    {
                        if (bindInput->getName() != getName())
                        {
                            continue;
                        }
                        OutputPtr output = bindInput->getConnectedOutput();
                        if (output)
                        {
                            return Edge(getSelfNonConst(), bindInput, output);
                        }
                        if (bindInput->hasValue())
                        {
                            return Edge(getSelfNonConst(), nullptr, bindInput);
                        }
                    }
                }
            }
        }
    }

    return NULL_EDGE;
}

GeomPropDefPtr Input::getDefaultGeomProp() const
{
    const string& defaultGeomProp = getAttribute(DEFAULT_GEOM_PROP_ATTRIBUTE);
    if (!defaultGeomProp.empty())
    {
        ConstDocumentPtr doc = getDocument();
        return doc->getChildOfType<GeomPropDef>(defaultGeomProp);
    }
    return nullptr;
}

bool Input::validate(string* message) const
{
    bool res = true;
    if (hasDefaultGeomPropString())
    {
        validateRequire(getDefaultGeomProp() != nullptr, res, message, "Invalid defaultgeomprop string");
    }
    return PortElement::validate(message) && res;
}

//
// Output methods
//

Edge Output::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (index < getUpstreamEdgeCount())
    {
        return Edge(getSelfNonConst(), nullptr, getConnectedNode());
    }

    return NULL_EDGE;
}

bool Output::hasUpstreamCycle() const
{
    try
    {
        for (Edge edge : traverseGraph()) { }
    }
    catch (ExceptionFoundCycle&)
    {
        return true;
    }
    return false;
}

bool Output::validate(string* message) const
{
    bool res = true;
    validateRequire(!hasUpstreamCycle(), res, message, "Cycle in upstream path");
    return PortElement::validate(message) && res;
}

//
// InterfaceElement methods
//

ParameterPtr InterfaceElement::getActiveParameter(const string& name) const
{
    for (ConstElementPtr elem : traverseInheritance())
    {
        ParameterPtr param = elem->asA<InterfaceElement>()->getParameter(name);
        if (param)
        {
            return param;
        }
    }
    return nullptr;
}

vector<ParameterPtr> InterfaceElement::getActiveParameters() const
{
    vector<ParameterPtr> activeParams;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<ParameterPtr> params = elem->asA<InterfaceElement>()->getParameters();
        activeParams.insert(activeParams.end(), params.begin(), params.end());
    }
    return activeParams;
}

InputPtr InterfaceElement::getActiveInput(const string& name) const
{
    for (ConstElementPtr elem : traverseInheritance())
    {
        InputPtr input = elem->asA<InterfaceElement>()->getInput(name);
        if (input)
        {
            return input;
        }
    }
    return nullptr;
}

vector<InputPtr> InterfaceElement::getActiveInputs() const
{
    vector<InputPtr> activeInputs;
    StringSet activeInputNamesSet;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<InputPtr> inputs = elem->asA<InterfaceElement>()->getInputs();
        for (const InputPtr& input : inputs)
        {
            if (input && activeInputNamesSet.insert(input->getName()).second)
            {
                activeInputs.push_back(input);
            }
        }
    }
    return activeInputs;
}

OutputPtr InterfaceElement::getActiveOutput(const string& name) const
{
    for (ConstElementPtr elem : traverseInheritance())
    {
        OutputPtr output = elem->asA<InterfaceElement>()->getOutput(name);
        if (output)
        {
            return output;
        }
    }
    return nullptr;
}


vector<OutputPtr> InterfaceElement::getActiveOutputs() const
{
    vector<OutputPtr> activeOutputs;
    StringSet activeOutputNamesSet;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<OutputPtr> outputs = elem->asA<InterfaceElement>()->getOutputs();
        for (const OutputPtr& output : outputs)
        {
            if (output && activeOutputNamesSet.insert(output->getName()).second)
            {
                activeOutputs.push_back(output);
            }
        }
    }
    return activeOutputs;
}

TokenPtr InterfaceElement::getActiveToken(const string& name) const
{
    for (ConstElementPtr elem : traverseInheritance())
    {
        TokenPtr token = elem->asA<InterfaceElement>()->getToken(name);
        if (token)
        {
            return token;
        }
    }
    return nullptr;
}

vector<TokenPtr> InterfaceElement::getActiveTokens() const
{
    vector<TokenPtr> activeTokens;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<TokenPtr> tokens = elem->asA<InterfaceElement>()->getTokens();
        activeTokens.insert(activeTokens.end(), tokens.begin(), tokens.end());
    }
    return activeTokens;
}

ValueElementPtr InterfaceElement::getActiveValueElement(const string& name) const
{
    for (ConstElementPtr interface : traverseInheritance())
    {
        ValueElementPtr valueElem = interface->getChildOfType<ValueElement>(name);
        if (valueElem)
        {
            return valueElem;
        }
    }
    return nullptr;
}

vector<ValueElementPtr> InterfaceElement::getActiveValueElements() const
{
    vector<ValueElementPtr> activeValueElems;
    StringSet activeValueElemNamesSet;
    for (ConstElementPtr interface : traverseInheritance())
    {
        vector<ValueElementPtr> valueElems = interface->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& valueElem : valueElems)
        {
            if (valueElem && activeValueElemNamesSet.insert(valueElem->getName()).second)
            {
                activeValueElems.push_back(valueElem);
            }
        }
    }
    return activeValueElems;
}

ValuePtr InterfaceElement::getParameterValue(const string& name, const string& target) const
{
    ParameterPtr param = getParameter(name);
    if (param)
    {
        return param->getValue();
    }

    // Return the value, if any, stored in our declaration.
    ConstNodeDefPtr decl = getDeclaration(target);
    if (decl)
    {
        param = decl->getParameter(name);
        if (param)
        {
            return param->getValue();
        }
    }

    return ValuePtr();
}

ValuePtr InterfaceElement::getInputValue(const string& name, const string& target) const
{
    InputPtr input = getInput(name);
    if (input)
    {
        return input->getValue();
    }

    // Return the value, if any, stored in our declaration.
    ConstNodeDefPtr decl = getDeclaration(target);
    if (decl)
    {
        input = decl->getInput(name);
        if (input)
        {
            return input->getValue();
        }
    }

    return ValuePtr();
}

void InterfaceElement::registerChildElement(ElementPtr child)
{
    TypedElement::registerChildElement(child);
    if (child->isA<Parameter>())
    {
        _parameterCount++;
    }
    else if (child->isA<Input>())
    {
        _inputCount++;
    }
    else if (child->isA<Output>())
    {
        _outputCount++;
    }
}

void InterfaceElement::unregisterChildElement(ElementPtr child)
{
    TypedElement::unregisterChildElement(child);
    if (child->isA<Parameter>())
    {
        _parameterCount--;
    }
    else if (child->isA<Input>())
    {
        _inputCount--;
    }
    else if (child->isA<Output>())
    {
        _outputCount--;
    }
}

ConstNodeDefPtr InterfaceElement::getDeclaration(const string&) const
{
    return NodeDefPtr();
}

bool InterfaceElement::isTypeCompatible(ConstInterfaceElementPtr declaration) const
{
    if (getType() != declaration->getType())
    {
        return false;
    }
    for (ValueElementPtr value : getActiveValueElements())
    {
        ValueElementPtr declarationValue = declaration->getActiveValueElement(value->getName());
        if (!declarationValue ||
            declarationValue->getCategory() != value->getCategory() ||
            declarationValue->getType() != value->getType())
        {
            return false;
        }
    }
    return true;
}

} // namespace MaterialX
