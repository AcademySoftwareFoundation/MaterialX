//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_UINODE_H
#define MATERIALX_UINODE_H

#include <MaterialXCore/Unit.h>

#include <imgui_node_editor.h>

namespace mx = MaterialX;
namespace ed = ax::NodeEditor;

class UiNode;
class UiPin;

using UiNodePtr = std::shared_ptr<UiNode>;
using UiPinPtr = std::shared_ptr<UiPin>;

// An edge between two UiNodes, storing the two nodes and connecting input.
class UiEdge
{
  public:
    UiEdge(UiNodePtr uiUp, UiNodePtr uiDown, mx::InputPtr input) :
        _uiUp(uiUp),
        _uiDown(uiDown),
        _input(input)
    {
    }
    mx::InputPtr getInput() const
    {
        return _input;
    }
    UiNodePtr getUp() const
    {
        return _uiUp;
    }
    UiNodePtr getDown() const
    {
        return _uiDown;
    }
    std::string getInputName() const
    {
        if (_input != nullptr)
        {
            return _input->getName();
        }
        else
        {
            return mx::EMPTY_STRING;
        }
    }

  private:
    UiNodePtr _uiUp;
    UiNodePtr _uiDown;
    mx::InputPtr _input;
};

// A connectable input or output pin of a UiNode.
class UiPin
{
  public:
    UiPin(int id, std::shared_ptr<UiNode> node, ed::PinKind kind, mx::PortElementPtr element) :
        _pinId(id),
        _pinNode(node),
        _kind(kind),
        _element(element),
        _connected(false)
    {
    }

    // Accessor methods
    ed::PinId getPinId() const { return _pinId; }
    const std::string& getName() const;
    const std::string& getType() const;
    std::shared_ptr<UiNode> getUiNode() const { return _pinNode; }
    ed::PinKind getKind() const { return _kind; }
    mx::PortElementPtr getElement() const { return _element; }
    void setElement(mx::PortElementPtr element) { _element = element; }
    mx::InputPtr getInput() const;
    mx::OutputPtr getOutput() const;

    void setConnected(bool connected)
    {
        _connected = connected;
    }

    bool getConnected() const
    {
        return _connected;
    }

    void addConnection(UiPinPtr pin)
    {
        if (!pin)
        {
            return;
        }

        for (size_t i = 0; i < _connections.size(); i++)
        {
            if (_connections[i]->getPinId() == pin->getPinId())
            {
                return;
            }
        }
        _connections.push_back(pin);
    }

    void deleteConnection(UiPinPtr pin)
    {
        if (!pin)
        {
            return;
        }

        // Remove pin from our connection list.
        for (size_t i = 0; i < _connections.size(); i++)
        {
            if (_connections[i]->getPinId() == pin->getPinId())
            {
                _connections.erase(_connections.begin() + i);
                break;
            }
        }
        if (_connections.size() == 0)
        {
            _connected = false;
        }

        // Remove ourselves from pin's connection list.
        for (size_t i = 0; i < pin->_connections.size(); i++)
        {
            if (pin->_connections[i]->getPinId() == _pinId)
            {
                pin->_connections.erase(pin->_connections.begin() + i);
                break;
            }
        }
        if (pin->_connections.size() == 0)
        {
            pin->setConnected(false);
        }
    }

    const std::vector<UiPinPtr>& getConnections() const
    {
        return _connections;
    }

  private:
    ed::PinId _pinId;
    std::shared_ptr<UiNode> _pinNode;
    ed::PinKind _kind;
    mx::PortElementPtr _element;
    std::vector<UiPinPtr> _connections;
    bool _connected;
};

// The visual representation of a node in a graph.
class UiNode
{
  public:
    UiNode();
    UiNode(const std::string& name, int id);
    ~UiNode() = default;

    std::string getName() const
    {
        return _name;
    }
    ImVec2 getPos() const
    {
        return _nodePos;
    }
    int getId() const
    {
        return _id;
    }
    const std::vector<UiNodePtr>& getOutputConnections() const
    {
        return _outputConnections;
    }

    // Element accessors
    mx::ElementPtr getElement() const { return _element; }
    mx::NodePtr getNode() const;
    mx::InputPtr getInput() const;
    mx::OutputPtr getOutput() const;
    mx::NodeGraphPtr getNodeGraph() const;

    // Element setters
    void setElement(mx::ElementPtr element) { _element = element; }
    void setNode(mx::NodePtr node) { _element = node; }
    void setInput(mx::InputPtr input) { _element = input; }
    void setOutput(mx::OutputPtr output) { _element = output; }
    void setNodeGraph(mx::NodeGraphPtr nodeGraph) { _element = nodeGraph; }

    void setName(const std::string& newName)
    {
        _name = newName;
    }
    void setPos(ImVec2 pos)
    {
        _nodePos = pos;
    }
    void setOutputConnection(UiNodePtr connections)
    {
        _outputConnections.push_back(connections);
    }

    void setMessage(const std::string& message)
    {
        _message = message;
    }

    const std::string& getMessage() const
    {
        return _message;
    }

    void setCategory(const std::string& category)
    {
        _category = category;
    }

    const std::string& getCategory() const
    {
        return _category;
    }

    void setType(const std::string& type)
    {
        _type = type;
    }

    const std::string& getType() const
    {
        return _type;
    }

    // Pin collection accessors
    std::vector<UiPinPtr>& getInputPins() { return _inputPins; }
    const std::vector<UiPinPtr>& getInputPins() const { return _inputPins; }
    std::vector<UiPinPtr>& getOutputPins() { return _outputPins; }
    const std::vector<UiPinPtr>& getOutputPins() const { return _outputPins; }

    // Edge collection accessors
    std::vector<UiEdge>& getEdges() { return _edges; }
    const std::vector<UiEdge>& getEdges() const { return _edges; }

    // Layout/display accessors
    bool getShowAllInputs() const { return _showAllInputs; }
    void setShowAllInputs(bool show) { _showAllInputs = show; }
    bool getShowOutputsInEditor() const { return _showOutputsInEditor; }
    void setShowOutputsInEditor(bool show) { _showOutputsInEditor = show; }

    UiNodePtr getConnectedNode(const std::string& name);
    int getEdgeIndex(int id, UiPinPtr pin);
    bool eraseEdge(int id, UiPinPtr pin);
    void removeOutputConnection(const std::string& name);

  private:
    int _id;
    ImVec2 _nodePos;
    std::string _name;
    std::vector<UiNodePtr> _outputConnections;
    mx::ElementPtr _element;
    std::string _category;
    std::string _message;
    std::string _type;

    std::vector<UiPinPtr> _inputPins;
    std::vector<UiPinPtr> _outputPins;
    std::vector<UiEdge> _edges;

    bool _showAllInputs;
    bool _showOutputsInEditor;
};

#endif
