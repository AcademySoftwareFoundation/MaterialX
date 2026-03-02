//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/UiNode.h>

namespace
{

const int INVALID_POS = -10000;

} // anonymous namespace

//
// UiPin methods
//

const std::string& UiPin::getName() const
{
    return _element ? _element->getName() : mx::EMPTY_STRING;
}

const std::string& UiPin::getType() const
{
    if (_element)
    {
        return _element->getType();
    }

    // Derive type from parent node for synthetic output pins.
    if (_pinNode)
    {
        if (auto input = _pinNode->getInput())
        {
            return input->getType();
        }
        if (auto output = _pinNode->getOutput())
        {
            return output->getType();
        }
    }

    return mx::EMPTY_STRING;
}

mx::InputPtr UiPin::getInput() const
{
    return _element ? _element->asA<mx::Input>() : nullptr;
}

mx::OutputPtr UiPin::getOutput() const
{
    return _element ? _element->asA<mx::Output>() : nullptr;
}

//
// UiNode methods
//

UiNode::UiNode() :
    _id(0),
    _nodePos(INVALID_POS, INVALID_POS),
    _showAllInputs(false),
    _showOutputsInEditor(true)
{
}

UiNode::UiNode(const std::string& name, int id) :
    _id(id),
    _nodePos(INVALID_POS, INVALID_POS),
    _name(name),
    _showAllInputs(false),
    _showOutputsInEditor(true)
{
}

mx::NodePtr UiNode::getNode() const
{
    return _element ? _element->asA<mx::Node>() : nullptr;
}

mx::InputPtr UiNode::getInput() const
{
    return _element ? _element->asA<mx::Input>() : nullptr;
}

mx::OutputPtr UiNode::getOutput() const
{
    return _element ? _element->asA<mx::Output>() : nullptr;
}

mx::NodeGraphPtr UiNode::getNodeGraph() const
{
    return _element ? _element->asA<mx::NodeGraph>() : nullptr;
}

// return the uiNode connected with input name
UiNodePtr UiNode::getConnectedNode(const std::string& name)
{
    for (const UiEdge& edge : _edges)
    {
        if (edge.getInputName() == name)
        {
            return edge.getUp();
        }
        else if (edge.getUp()->getName() == name)
        {
            return edge.getUp();
        }
    }
    for (const UiEdge& edge : _edges)
    {
        if (edge.getInputName().empty())
        {
            return edge.getUp();
        }
    }
    return nullptr;
}

int UiNode::getEdgeIndex(int id, UiPinPtr pin)
{
    mx::InputPtr pinInput = pin ? pin->getInput() : nullptr;
    int count = 0;
    for (const UiEdge& edge : _edges)
    {
        if ((edge.getUp()->getId() == id || edge.getDown()->getId() == id) &&
            pinInput == edge.getInput())
        {
            return count;
        }
        count++;
    }
    return -1;
}

bool UiNode::eraseEdge(int id, UiPinPtr pin)
{
    int num = getEdgeIndex(id, pin);
    if (num == -1)
    {
        return false;
    }
    _edges.erase(_edges.begin() + num);
    return true;
}

void UiNode::removeOutputConnection(const std::string& name)
{
    for (size_t i = 0; i < _outputConnections.size(); i++)
    {
        if (_outputConnections[i]->getName() == name)
        {
            _outputConnections.erase(_outputConnections.begin() + i);
            break;
        }
    }
}
