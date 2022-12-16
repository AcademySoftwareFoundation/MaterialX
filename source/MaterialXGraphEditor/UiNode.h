#ifndef MATERIALX_UINODE_H
#define MATERIALX_UINODE_H

#include <MaterialXGraphEditor/Graph.h>

#include <MaterialXCore/Unit.h>

#include <imgui_node_editor.h>
#include <widgets.h>

namespace mx = MaterialX;
namespace ed = ax::NodeEditor;

class Edge;
class Pin;

class UiNode
{
    using UiNodePtr = std::shared_ptr<UiNode>;

  public:
    UiNode();
    UiNode(const std::string name, const int id);
    UiNode(const int id);
    ~UiNode(){};

    std::string getName()
    {
        return _name;
    }
    ImVec2 getPos()
    {
        return _nodePos;
    }
    int getInputConnect()
    {
        return _inputNodeNum;
    }
    int getId()
    {
        return _id;
    }
    std::vector<UiNodePtr> getOutputConnections()
    {
        return _outputConnections;
    }
    mx::NodePtr getNode()
    {
        return _currNode;
    }
    mx::InputPtr getInput()
    {
        return _currInput;
    }
    mx::OutputPtr getOutput()
    {
        return _currOutput;
    }

    void setName(const std::string &newName)
    {
        _name = newName;
    }
    void setPos(ImVec2 pos)
    {
        _nodePos = pos;
    }
    void setInputNodeNum(int num)
    {
        _inputNodeNum += num;
    }
    void setNode(mx::NodePtr node)
    {
        _currNode = node;
    }
    void setInput(mx::InputPtr input)
    {
        _currInput = input;
    }
    void setOutput(mx::OutputPtr output)
    {
        _currOutput = output;
    }
    void setOutputConnection(UiNodePtr connections)
    {
        _outputConnections.push_back(connections);
    }
    void setMessage(const std::string &message)
    {
        _message = message;
    }

    std::string getMessage()
    {
        return _message;
    }

    void setCategory(const std::string &category)
    {
        _category = category;
    }

    std::string getCategory()
    {
        return _category;
    }

    void setType(const std::string &type)
    {
        _type = type;
    }

    std::string getType()
    {
        return _type;
    }
    mx::NodeGraphPtr getNodeGraph()
    {
        return _currNodeGraph;
    }

    void setNodeGraph(mx::NodeGraphPtr nodeGraph)
    {
        _currNodeGraph = nodeGraph;
    }

    friend bool operator==(const UiNodePtr& lhs, const UiNodePtr& rhs)
    {
        return lhs->getName() == rhs->getName();
    }

    bool operator()(const UiNodePtr& node1, const UiNodePtr& node2) const
    {
        return (node1->_level < node2->_level);
    }

    // functions
    UiNodePtr getConnectedNode(std::string name);
    float getAverageY();
    float getMinX();
    int getEdgeIndex(int id);
    std::vector<Edge> edges;
    std::vector<Pin> inputPins;
    std::vector<Pin> outputPins;
    void removeOutputConnection(std::string);
    mx::ElementPtr getMxElement();
    int _level;

  private:
    int _id;
    ImVec2 _nodePos;
    std::string _name;
    int _inputNodeNum;
    std::vector<std::pair<int, std::string>> _inputs;
    // used only for nodegraph nodes
    std::vector<std::pair<int, std::string>> _outputs;
    std::vector<UiNodePtr> _outputConnections;
    mx::NodePtr _currNode;
    mx::InputPtr _currInput;
    mx::OutputPtr _currOutput;
    std::string _category;
    std::string _message;
    std::string _type;
    mx::NodeGraphPtr _currNodeGraph;
};

struct LinkInfo
{
    ed::LinkId _id;
    ed::PinId _inputId;
    ed::PinId _outputId;
};

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
};

enum class NodeType
{
    Math
};

class Pin
{
  public:
    Pin(int id, const char* name, std::string type, std::shared_ptr<UiNode> node, ed::PinKind kind, mx::InputPtr input, mx::OutputPtr output) :
        _pinId(id), _name(name), _type(type), _pinNode(node), _kind(kind), _input(input), _output(output), _connected(false)
    {
    }
    void setConnected(bool connected)
    {
        _connected = connected;
    }
    bool getConnected()
    {
        return _connected;
    }
    ed::PinId _pinId;
    std::string _name;
    std::string _type;
    std::shared_ptr<UiNode> _pinNode;
    ed::PinKind _kind;
    mx::InputPtr _input;
    mx::OutputPtr _output;
    bool _connected;
};

#endif
