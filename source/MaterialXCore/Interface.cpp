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
const string InterfaceElement::NODE_DEF_ATTRIBUTE = "nodedef";

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
    for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
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

//
// Input methods
//

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

GeomPropPtr Input::getGeomProp() const
{
    // An input can only have a single geomprop,
    // so return the first one found.
    for (ElementPtr child : _childOrder)
    {
        GeomPropPtr geomprop = child->asA<GeomProp>();
        if (geomprop)
        {
            return geomprop;
        }
    }
    return nullptr;
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
    for (ConstElementPtr interface : traverseInheritance())
    {
        vector<ValueElementPtr> valueElems = interface->getChildrenOfType<ValueElement>();
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
