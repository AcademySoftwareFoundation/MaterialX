//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GRAPH_H
#define MATERIALX_GRAPH_H

#include <MaterialXGraphEditor/FileDialog.h>
#include <MaterialXGraphEditor/RenderView.h>
#include <MaterialXGraphEditor/UiNode.h>

#include <imgui_node_editor.h>

#include <stack>

namespace ed = ax::NodeEditor;
namespace mx = MaterialX;

// A link connects two pins and includes a unique id and the ids of the two pins it connects
// Based off Link struct from ImGui Node Editor blueprints-examples.cpp
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

class Graph
{
  public:
    Graph(const std::string& materialFilename,
          const std::string& meshFilename,
          const mx::FileSearchPath& searchPath,
          const mx::FilePathVec& libraryFolders,
          int viewWidth,
          int viewHeight);

    mx::DocumentPtr loadDocument(mx::FilePath filename);
    void drawGraph(ImVec2 mousePos);

    RenderViewPtr getRenderer()
    {
        return _renderer;
    }

    void setFontScale(float val)
    {
        _fontScale = val;
    }

    ~Graph(){};

  private:
    mx::ElementPredicate getElementPredicate() const;
    void loadStandardLibraries();
    void createNodeUIList(mx::DocumentPtr doc);
    void buildUiBaseGraph(mx::DocumentPtr doc);
    void buildUiNodeGraph(const mx::NodeGraphPtr& nodeGraphs);
    void buildGroupNode(UiNodePtr node);

    // handling link information
    void linkGraph();
    void connectLinks();
    int findLinkPosition(int id);
    std::vector<int> findLinkId(int attrId);
    bool linkExists(Link newLink);
    void AddLink(ed::PinId inputPinId, ed::PinId outputPinId);
    void deleteLink(ed::LinkId deletedLinkId);
    void deleteLinkInfo(int startAtrr, int endAttr);

    // functions for the layout of the nodes
    ImVec2 layoutPosition(UiNodePtr node, ImVec2 pos, bool initialLayout, int level);
    void layoutInputs();
    void findYSpacing(float startPos);
    float totalHeight(int level);
    void setYSpacing(int level, float startingPos);
    float findAvgY(const std::vector<UiNodePtr>& nodes);

    // pin information
    void setPinColor();
    void DrawPinIcon(std::string type, bool connected, int alpha);
    UiPinPtr getPin(ed::PinId id);
    void drawInputPin(UiPinPtr pin);
    ed::PinId getOutputPin(UiNodePtr node, UiNodePtr inputNode, UiPinPtr input);
    void drawOutputPins(UiNodePtr node, const std::string& longestInputLabel);
    void addNodeGraphPins();

    // UiNode functions
    std::vector<int> createNodes(bool nodegraph);
    int getNodeId(ed::PinId pinId);
    int findNode(int nodeId);
    int findNode(const std::string& name, const std::string& type);
    void addNode(const std::string& category, const std::string& name, const std::string& type);
    void deleteNode(UiNodePtr node);
    void setUiNodeInfo(UiNodePtr node, const std::string& type, const std::string& category);

    // UiEdge functions
    bool edgeExists(UiEdge edge);
    void createEdge(UiNodePtr upNode, UiNodePtr downNode, mx::InputPtr connectingInput);
    void removeEdge(int downNode, int upNode, UiPinPtr pin);

    void writeText(std::string filename, mx::FilePath filePath);
    void savePosition();
    bool checkPosition(UiNodePtr node);

    void addNodeInput(UiNodePtr node, mx::InputPtr& input);
    mx::InputPtr findInput(mx::InputPtr input, std::string name);

    // travel up from inside a node graph
    void upNodeGraph();

    // property editor information
    void setConstant(UiNodePtr node, mx::InputPtr& input, const mx::UIProperties& uiProperties);
    void propertyEditor();
    void setDefaults(mx::InputPtr input);

    // set up Ui information for add node popup
    void addExtraNodes();

    // copy and paste functions
    void copyInputs();
    void positionPasteBin(ImVec2 pos);
    void copyNodeGraph(UiNodePtr origGraph, UiNodePtr copyGraph);
    void copyUiNode(UiNodePtr node);

    // renderview window and buttons
    void graphButtons();

    // popup information
    void addNodePopup(bool cursor);
    void searchNodePopup(bool cursor);
    bool readOnly();
    void readOnlyPopup();
    void shaderPopup();

    // modifying materials
    void updateMaterials(mx::InputPtr input = nullptr, mx::ValuePtr value = nullptr);
    void selectMaterial(UiNodePtr node);
    void handleRenderViewInputs(ImVec2 minValue, float width, float height);
    void setRenderMaterial(UiNodePtr node);

    // File I/O
    void clearGraph();
    void loadGraphFromFile();
    void saveGraphToFile();
    void loadGeometry();

    mx::StringVec _geomFilter;
    mx::StringVec _mtlxFilter;
    mx::StringVec _imageFilter;

    // Help
    void showHelp() const;

    RenderViewPtr _renderer;

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
    std::vector<UiPinPtr> _currPins;
    std::vector<Link> _currLinks;
    std::vector<Link> _newLinks;
    std::vector<UiEdge> _currEdge;
    std::unordered_map<UiNodePtr, std::vector<UiPinPtr>> _downstreamInputs;
    std::unordered_map<std::string, ImColor> _pinColor;

    // current nodes and nodegraphs
    UiNodePtr _currUiNode;
    UiNodePtr _prevUiNode;
    mx::GraphElementPtr _currGraphElem;
    UiNodePtr _currRenderNode;
    std::vector<std::string> _currGraphName;

    // for adding new nodes
    std::unordered_map<std::string, std::vector<mx::NodeDefPtr>> _nodesToAdd;
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> _extraNodes;

    // stacks to dive into and out of node graphs
    std::stack<std::vector<UiNodePtr>> _graphStack;
    std::stack<std::vector<UiPinPtr>> _pinStack;
    // this stack keeps track of the graph total size
    std::stack<int> _sizeStack;

    // map to group and layout nodes
    std::unordered_map<int, std::vector<UiNodePtr>> _levelMap;

    // map for copied nodes
    std::map<UiNodePtr, UiNodePtr> _copiedNodes;

    bool _initial;
    bool _delete;

    // file dialog information
    FileDialog _fileDialog;
    FileDialog _fileDialogSave;
    FileDialog _fileDialogImage;
    FileDialog _fileDialogGeom;


    bool _isNodeGraph;

    int _graphTotalSize;

    // popup up variables
    bool _popup;
    bool _shaderPopup;
    int _searchNodeId;
    bool _addNewNode;
    bool _ctrlClick;
    bool _isCut;
    // auto layout button clicked
    bool _autoLayout;

    // used when updating materials
    int _frameCount;
    // used for filtering pins when connecting links
    std::string _pinFilterType;

    // DPI scaling for fonts
    float _fontScale = 1.0f;
};

#endif
