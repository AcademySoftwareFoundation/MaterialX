//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>

namespace MaterialX
{

const string PortElement::NODE_NAME_ATTRIBUTE = "nodename";
const string PortElement::OUTPUT_ATTRIBUTE = "output";
const string InterfaceElement::VERSION_ATTRIBUTE = "version";
const string InterfaceElement::DEFAULT_VERSION_ATTRIBUTE = "isdefaultversion";

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
    for (ConstElementPtr elem : traverseAncestors())
    {
        ConstGraphElementPtr graph = elem->asA<GraphElement>();
        if (graph)
        {
            return graph->getNode(getNodeName());
        }
    }
    return NodePtr();
}

bool PortElement::validate(string* message) const
{
    bool res = true;
    if (hasNodeName())
    {
        validateRequire(getConnectedNode() != nullptr, res, message, "Invalid port connection");
    }
    if (getConnectedNode())
    {
        if (hasOutputString())
        {
            validateRequire(getConnectedNode()->getType() == MULTI_OUTPUT_TYPE_STRING, res, message, "Multi-output type expected in port connection");
            NodeDefPtr connectedNodeDef = getConnectedNode()->getNodeDef();
            if (connectedNodeDef)
            {
                OutputPtr output = connectedNodeDef->getOutput(getOutputString());
                validateRequire(output != nullptr, res, message, "Invalid output in port connection");
                if (output)
                {
                    validateRequire(getType() == output->getType(), res, message, "Mismatched output type in port connection");
                }
            }
        }
        else
        {
            validateRequire(getType() == getConnectedNode()->getType(), res, message, "Mismatched types in port connection");
        }
    }
    return ValueElement::validate(message) && res;
}

//
// Parameter methods
//

Edge Parameter::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (material && index < getUpstreamEdgeCount())
    {
        ConstNodeDefPtr nodeDef = getParent()->isA<Implementation>() ?
                                  getParent()->asA<Implementation>()->getNodeDef() :
                                  getParent()->asA<NodeDef>();
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

//
// Input methods
//

Edge Input::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (material && index < getUpstreamEdgeCount())
    {
        ConstNodeDefPtr nodeDef = getParent()->isA<Implementation>() ?
                                  getParent()->asA<Implementation>()->getNodeDef() :
                                  getParent()->asA<NodeDef>();
        if (nodeDef)
        {
            if (material)
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
    }

    return NULL_EDGE;
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
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<InputPtr> inputs = elem->asA<InterfaceElement>()->getInputs();
        activeInputs.insert(activeInputs.end(), inputs.begin(), inputs.end());
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
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<OutputPtr> outputs = elem->asA<InterfaceElement>()->getOutputs();
        activeOutputs.insert(activeOutputs.end(), outputs.begin(), outputs.end());
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

std::pair<int, int> InterfaceElement::getVersionIntegers() const
{
    string versionString = getVersionString();
    StringVec splitVersion = splitString(versionString, ".");
    try
    {
        if (splitVersion.size() == 2)
        {
            return {std::stoi(splitVersion[0]), std::stoi(splitVersion[1])};
        }
        else if (splitVersion.size() == 1)
        {
            return {std::stoi(splitVersion[0]), 0};
        }
    }
    catch (std::invalid_argument&)
    {
    }
    catch (std::out_of_range&)
    {
    }
    return {0, 0};
}

ValueElementPtr InterfaceElement::getActiveValueElement(const string& name) const
{
    for (ConstElementPtr elem : traverseInheritance())
    {
       ValueElementPtr valueElem = elem->asA<InterfaceElement>()->getChildOfType<ValueElement>(name);
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
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<ValueElementPtr> valueElems = elem->asA<InterfaceElement>()->getChildrenOfType<ValueElement>();
        activeValueElems.insert(activeValueElems.end(), valueElems.begin(), valueElems.end());
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
    NodeDefPtr decl = getDeclaration(target);
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
    NodeDefPtr decl = getDeclaration(target);
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

NodeDefPtr InterfaceElement::getDeclaration(const string& target) const
{
    if (isA<Node>())
    {
        return asA<Node>()->getNodeDef(target);
    }
    else if (isA<NodeGraph>())
    {
        return asA<NodeGraph>()->getNodeDef();
    }
    else if (isA<Implementation>())
    {
        return asA<Implementation>()->getNodeDef();
    }

    return NodeDefPtr();
}

bool InterfaceElement::isTypeCompatible(ConstInterfaceElementPtr rhs) const
{
    if (getType() != rhs->getType())
    {
        return false;
    }
    for (ParameterPtr param : getActiveParameters())
    {
        ParameterPtr matchingParam = rhs->getActiveParameter(param->getName());
        if (matchingParam && matchingParam->getType() != param->getType())
        {
            return false;
        }
    }
    for (InputPtr input : getActiveInputs())
    {
        InputPtr matchingInput = rhs->getActiveInput(input->getName());
        if (matchingInput && matchingInput->getType() != input->getType())
        {
            return false;
        }
    }
    return true;
}

bool InterfaceElement::isVersionCompatible(ConstNodeDefPtr nodeDef) const
{
    if (getVersionIntegers() == nodeDef->getVersionIntegers())
    {
        return true;
    }
    if (!hasVersionString() && nodeDef->getDefaultVersion())
    {
        return true;
    }
    return false;
}

} // namespace MaterialX
