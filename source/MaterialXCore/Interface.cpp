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
const string PortElement::OUTPUT_ATTRIBUTE = "output";
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
                if (output && !hasChannels())
                {
                    validateRequire(getType() == output->getType(), res, message, "Mismatched output type in port connection");
                }
            }
        }
        else if (!hasChannels())
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
            for (ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                if (shaderRef->getNodeDef() == nodeDef)
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

        // Apply Override elements to the Parameter.
        OverridePtr override = material->getOverride(getPublicName());
        if (override)
        {
            return Edge(getSelfNonConst(), nullptr, override);
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
                for (ShaderRefPtr shaderRef : material->getShaderRefs())
                {
                    if (shaderRef->getNodeDef() == nodeDef)
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

        // Apply Override elements to the Input.
        OverridePtr override = material->getOverride(getPublicName());
        if (override)
        {
            return Edge(getSelfNonConst(), nullptr, override);
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

ValuePtr InterfaceElement::getParameterValue(const string& name) const
{
    ParameterPtr param = getChildOfType<Parameter>(name);
    return param ? param->getValue() : ValuePtr();
}

ValuePtr InterfaceElement::getInputValue(const string& name) const
{
    InputPtr input = getChildOfType<Input>(name);
    return input ? input->getValue() : ValuePtr();
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

bool InterfaceElement::isTypeCompatible(InterfaceElementPtr rhs) const
{
    if (getType() != rhs->getType())
    {
        return false;
    }
    for (ParameterPtr param : getParameters())
    {
        ParameterPtr matchingParam = rhs->getParameter(param->getName());
        if (matchingParam && matchingParam->getType() != param->getType())
        {
            return false;
        }
    }
    for (InputPtr input : getInputs())
    {
        InputPtr matchingInput = rhs->getInput(input->getName());
        if (matchingInput && matchingInput->getType() != input->getType())
        {
            return false;
        }
    }
    return true;
}

} // namespace MaterialX
