#ifndef MATERIALX_GRAPH_H
#define MATERIALX_GRAPH_H

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif



#include <MaterialXGraphEditor/RenderView.h>
#include <MaterialXGraphEditor/UiNode.h>

#include <MaterialXRender/Util.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <imfilebrowser.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <stack>
#include <algorithm>



namespace ed = ax::NodeEditor;
namespace mx = MaterialX;
class UiNode;
class Pin;
using UiNodePtr = std::shared_ptr<UiNode>;
using RenderViewPtr = std::shared_ptr<RenderView>;
struct Link
{
    int id;
    int _startAttr, _endAttr;
    Link() :
        _startAttr(-1),
        _endAttr(-1)
    {
        static int _id = 0;
        id = ++_id;
    }
};

class Edge
{
  public:
    Edge(UiNodePtr uiDown, UiNodePtr uiUp, mx::InputPtr input) :
        _uiDown(uiDown),
        _uiUp(uiUp),
        _input(input)
    {
    }
    mx::InputPtr getInput()
    {
        return _input;
    }
    UiNodePtr getDown()
    {
        return _uiDown;
    }
    UiNodePtr getUp()
    {
        return _uiUp;
    }
    std::string getInputName()
    {
        if (_input != nullptr)
        {
            return _input->getName();
        }
        else
        {
            return "";
        }
    }
    UiNodePtr _uiDown;
    UiNodePtr _uiUp;
    mx::InputPtr _input;
};

class Graph
{
  public:
    Graph(const std::string& materialFilename, const mx::FileSearchPath& searchPath, const mx::FilePathVec& libraryFolders);
    void initialize();
    void drawGraph(ImVec2 mousePos);
    mx::DocumentPtr loadDocument(std::string filename);
    RenderViewPtr _renderer;
    ~Graph(){};

  private:
    void loadStandardLibraries();
    void buildUiBaseGraph(const std::vector<mx::NodeGraphPtr>& nodeGraphs, const std::vector<mx::NodePtr>& docNodes, const std::vector<mx::InputPtr>& inputNodes);
    void buildUiNodeGraph(const mx::NodeGraphPtr& nodeGraphs);
    void linkGraph();
    void connectLinks();
    int findLinkPosition(int id);
    ImVec2 layoutPosition(UiNodePtr node, ImVec2 pos, bool initialLayout, int level);
    void layoutInputs();
    ImColor getPinColor(std::string type);
    //arguments for node to set, input in that node
    void setConstant(UiNodePtr node, mx::InputPtr& input);
    int findNode(std::string name, std::string type);
    bool edgeExists(Edge edge);
    void addNode(std::string category, std::string name, std::string type);
    void writeText(std::string filename, mx::FilePath filePath);
    bool linkExists(Link newLink);
    //input to update, value to update, if the update will modify uniform or not, if the input is a ui element in a nodegraph node or input ui node
    void updateMaterials(mx::InputPtr input, mx::ValuePtr value, bool genShader);
    std::vector<int> findLinkId(int attrId);
    int findNode(int nodeId);
    void DrawPinIcon(std::string type, bool connected, int alpha);
    Pin getPin(ed::PinId id);
    void createInputPin(int attrId, mx::InputPtr input);
    std::vector<int> createNodes(bool nodegraph);
    int getNodeId(ed::PinId pinId);
    void buildGroupNode(UiNodePtr node);
    void AddLink(ed::PinId inputPinId, ed::PinId outputPinId);
    void deleteLink(ed::LinkId deletedLinkId);
    void deleteNode(UiNodePtr node);
    void outputPin(UiNodePtr node, int attrId, std::string outputPin, std::string);
    void savePosition();
    bool checkPosition(UiNodePtr node);
    void findYSpacing(float startPos);
    float totalHeight(int level);
    void setYSpacing(int level, float startingPos);
    float findAvgY(const std::vector<UiNodePtr>& nodes);
    ed::PinId getOutputPin(UiNodePtr node, UiNodePtr inputNode, Pin input);
    void setRenderMaterial(UiNodePtr node);
    void upNodeGraph();
    void propertyEditor();
    void setDefaults(mx::InputPtr input);
    void addExtraNodes();
    void copyInputs();
    void createEdge(UiNodePtr upNode, UiNodePtr downNode, mx::InputPtr connectingInput);
    void positionPasteBin(ImVec2  pos);
    void copyNodeGraph(UiNodePtr origGraph, UiNodePtr copyGraph);
    void copyUiNode(UiNodePtr node);
    void cutNodes();
    void deleteLinkInfo(int startAtrr, int endAttr);
    void setUiNodeInfo(UiNodePtr node, std::string type, std::string category);
    void graphButtons();
    void addNodePopup(bool cursor);
    void searchNodePopup(bool cursor);
    bool readOnly();
    void readOnlyPopup();
    void selectMaterial(UiNodePtr node);
    void handleRenderViewInputs(ImVec2 minValue, float width, float height);
    
    mx::InputPtr findInput(mx::InputPtr input, std::string name);
    // document and intializing information
    mx::FilePath _materialFilename;
    mx::DocumentPtr _graphDoc;
    mx::StringSet _xincludeFiles;

    mx::FileSearchPath _searchPath;
    mx::FilePathVec _libraryFolders;
    mx::DocumentPtr _stdLib;

    // image information
    mx::ImagePtr _image;
    mx::ImageHandlerPtr _imageHandler;

    // containers of node informatin
    std::vector<UiNodePtr> _graphNodes;
    std::vector<Pin> _currPins;
    std::vector<Link> _currLinks;
    std::vector<Link> _newLinks;
    std::vector<Edge> _currEdge;

   
    // current nodes and nodegraphs
    UiNodePtr _currUiNode;
    UiNodePtr _prevUiNode;
    mx::GraphElementPtr _currGraphElem;
    UiNodePtr _currRenderNode;
    std::vector<std::string> _currGraphName;

    // for adding new nodes
    std::unordered_map<std::string, std::vector<mx::NodeDefPtr>> _nodesToAdd;
    std::unordered_map < std::string, std::vector<std::vector < std::string >>> _extraNodes;

    // stack to dive into and out of node graphs
    std::stack<std::vector<UiNodePtr>> _graphStack;

    // map to group and layout nodes
    std::unordered_map<int, std::vector<UiNodePtr>> _levelMap;

    //map for copied nodes
    std::map<UiNodePtr, UiNodePtr> _copiedNodes;

    bool _initial;
    bool _delete;

    ImGui::FileBrowser _fileDialog;
    ImGui::FileBrowser _fileDialogSave;
    ImGui::FileBrowser _fileDialogConstant;

    bool _isNodeGraph;

    int _graphTotalSize;
    bool _popup;
    int _searchNodeId;
    bool _addNewNode;
    bool _ctrlClick;
    bool _isCut;
    bool _autoLayout;

    //used for filtering pins when connecting links
    std::string _pinFilterType;

    GLFWwindow* _window;
};

#endif
