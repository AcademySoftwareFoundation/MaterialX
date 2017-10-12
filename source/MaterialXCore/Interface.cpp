//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>

namespace MaterialX
{

const string PortElement::NODE_NAME_ATTRIBUTE = "nodename";
const string PortElement::CHANNELS_ATTRIBUTE = "channels";

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
        ConstNodeGraphPtr graph = elem->asA<NodeGraph>();
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
    if (getConnectedNode() && !hasChannels())
    {
        validateRequire(getType() == getConnectedNode()->getType(), res, message, "Mismatched types in port connection");
    }
    return ValueElement::validate(message) && res;
}

//
// Parameter methods
//

Edge Parameter::getUpstreamEdge(ConstMaterialPtr material, size_t index)
{
    if (material && index < getUpstreamEdgeCount())
    {
        if (getParent()->isA<NodeDef>())
        {
            // Apply BindParam elements to the Parameter.
            for (ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                if (shaderRef->getReferencedShaderDef() == getParent())
                {
                    for (BindParamPtr bindParam : shaderRef->getBindParams())
                    {
                        if (bindParam->getName() == getName() && bindParam->hasValue())
                        {
                            return Edge(getSelf(), nullptr, bindParam);
                        }
                    }
                }
            }
        }

        // Apply Override elements to the Parameter.
        OverridePtr override = material->getOverride(getPublicName());
        if (override)
        {
            return Edge(getSelf(), nullptr, override);
        }
    }

    return NULL_EDGE;
}

//
// Input methods
//

Edge Input::getUpstreamEdge(ConstMaterialPtr material, size_t index)
{
    if (material && index < getUpstreamEdgeCount())
    {
        if (getParent()->isA<NodeDef>())
        {
            if (material)
            {
                // Apply BindInput elements to the Input.
                for (ShaderRefPtr shaderRef : material->getShaderRefs())
                {
                    if (shaderRef->getReferencedShaderDef() == getParent())
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
                                return Edge(getSelf(), bindInput, output);
                            }
                            if (bindInput->hasValue())
                            {
                                return Edge(getSelf(), nullptr, bindInput);
                            }
                        }
                    }
                }
            }
        }

        // Apply Override elements to the Input.
        OverridePtr override = material->getOverride(getPublicName());
        if (override)
        {
            return Edge(getSelf(), nullptr, override);
        }
    }

    return NULL_EDGE;
}

//
// Output methods
//

Edge Output::getUpstreamEdge(ConstMaterialPtr material, size_t index)
{
    if (index < getUpstreamEdgeCount())
    {
        return Edge(getSelf(), nullptr, getConnectedNode());
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

const string& InterfaceElement::getParameterValueString(const string& name) const
{
    ParameterPtr param = getChildOfType<Parameter>(name);
    return param ? param->getValueString() : EMPTY_STRING;
}

ValuePtr InterfaceElement::getParameterValue(const string& name) const
{
    ParameterPtr param = getChildOfType<Parameter>(name);
    return param ? param->getValue() : ValuePtr();
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
}

} // namespace MaterialX
