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

void UiNode::buildUiTokenMap()
{
    _uiTokenMap.clear(); // Assume we want clean slate

    const mx::NodePtr node = getNode();
    if (!node) return; // Early return as no further work is needed
    
    mx::ElementPtr currElem = node->asA<mx::Element>(); // Explicit upcast
    // Traverse upward using parent pointers to collect all active tokens in the current scope
    while (currElem)
    {
        if (mx::ConstInterfaceElementPtr currInterfaceElem = currElem->asA<mx::InterfaceElement>())
        {
            UiToken::applyTokenMapping(&_uiTokenMap, currInterfaceElem, currElem);

            // If the node is a nodegraph, check for tokens on corresponding nodedef
            if (mx::ConstNodeGraphPtr currNodeGraph = currElem->asA<mx::NodeGraph>())
            {
                if (mx::NodeDefPtr currNodedef = currNodeGraph->getNodeDef())
                {
                    UiToken::applyTokenMapping(&_uiTokenMap, currNodedef, currNodedef);
                }
            }
            
            // If the node is a custom node instance, check for tokens on corresponding nodedef
            if (mx::NodePtr currNode = currElem->asA<mx::Node>())
            {
                if (mx::NodeDefPtr currNodedef = currNode->getNodeDef())
                {
                    UiToken::applyTokenMapping(&_uiTokenMap, currNodedef, currNodedef);
                }
            }
        }
        currElem = currElem->getParent();
    }

    // Traverse through inputs and determine which tokens their value depends on
    for (const auto& input : node->getActiveInputs())
    {
        if (input->getType() != "filename")
            continue;

        mx::StringResolverPtr inputResolver = input->createStringResolver();
        const mx::StringMap& inputTokens = inputResolver->getFilenameSubstitutions();

        mx::StringMap inputTokensRenormalized;
        for (const auto& entry : inputTokens)
        {
            // Store tokens without excess delimiters
            inputTokensRenormalized[entry.first] = entry.first.substr(1, entry.first.size() - 2);
        }

        std::string inputValue = input->getValueString();
        if (inputValue.empty() && input->hasInterfaceName())
        {
            mx::InputPtr interfaceInput = input->getInterfaceInput();
            if (interfaceInput)
            {
                inputValue = interfaceInput->getValueString(); // Get value from referenced interface
            }
        }

        for (const auto& entry : inputTokens)
        {
            if (inputValue.find(entry.first) != std::string::npos)
            {
                // Ensure that the token exists in the pre-built map
                if (auto it = _uiTokenMap.find(inputTokensRenormalized[entry.first]); it != _uiTokenMap.end())
                {
                    it->second->addAffectedInput(input); // Append to affected inputs of corresponding entry in token map
                }
            }
        }
    }
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
