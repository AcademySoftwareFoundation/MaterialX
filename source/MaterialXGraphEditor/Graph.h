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

class MenuItem
{
  public:
    MenuItem(const std::string& name, const std::string& type, const std::string& category, const std::string& group) :
        name(name), type(type), category(category), group(group) { }

    // getters
    std::string getName() const { return name; }
    std::string getType() const { return type; }
    std::string getCategory() const { return category; }
    std::string getGroup() const { return group; }

    // setters
    void setName(const std::string& newName) { this->name = newName; }
    void setType(const std::string& newType) { this->type = newType; }
    void setCategory(const std::string& newCategory) { this->category = newCategory; }
    void setGroup(const std::string& newGroup) { this->group = newGroup; }

  private:
    std::string name;
    std::string type;
    std::string category;
    std::string group;
};

namespace ax {
namespace NodeEditor {
    enum class PinKind;
}
}

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

// A container which sorts input and output pins by type for a given node.
// "inputsByType" maps a type name (ie. int) to the names of all of the input 
// pins of that type for some node. "outputsByType" does the same for output pins.
struct PinOrganizer
{
    std::unordered_map<std::string, std::vector<std::string>> inputsByType;
    std::unordered_map<std::string, std::vector<std::string>> outputsByType;
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

    // Generate node UI from nodedefs
    void createNodeUIList(mx::DocumentPtr doc);

    // Build UiNode nodegraph upon loading a document
    void buildUiBaseGraph(mx::DocumentPtr doc);

    // Build UiNode node graph upon diving into a nodegraph node
    void buildUiNodeGraph(const mx::NodeGraphPtr& nodeGraphs);

    // Based on the comment node in the ImGui Node Editor blueprints-example.cpp.
    void buildGroupNode(UiNodePtr node);

    // Connect links via connected nodes in UiNodePtr
    void linkGraph();

    // Connect all links via the graph editor library
    void connectLinks();

    // Find link position in current links vector from link id
    int findLinkPosition(int id);

    // Find link from attribute id
    std::vector<int> findLinkId(int attrId);

    // Check if link exists in the current link vector
    bool linkExists(Link newLink);

    // Add link to nodegraph and set up connections between UiNodes and
    // MaterialX Nodes to update shader
    // startPinId - where the link was initiated
    // endPinId - where the link was ended
    void addLink(ed::PinId startPinId, ed::PinId endPinId);

    // Delete link from current link vector and remove any connections in
    // UiNode or MaterialX Nodes to update shader
    void deleteLink(ed::LinkId deletedLinkId);

    void deleteLinkInfo(int startAtrr, int endAttr);

    // Layout the x-position by assigning the node levels based on its distance from the first node
    ImVec2 layoutPosition(UiNodePtr node, ImVec2 pos, bool initialLayout, int level);

    // Extra layout pass for inputs and nodes that do not attach to an output node
    void layoutInputs();

    void findYSpacing(float startPos);
    float totalHeight(int level);
    void setYSpacing(int level, float startingPos);
    float findAvgY(const std::vector<UiNodePtr>& nodes);

    // Return pin color based on the type of the value of that pin
    void setPinColor();

    // Based on the pin icon function in the ImGui Node Editor blueprints-example.cpp
    void drawPinIcon(std::string type, bool connected, int alpha);

    UiPinPtr getPin(ed::PinId id);
    void drawInputPin(UiPinPtr pin);

    // Return output pin needed to link the inputs and outputs
    ed::PinId getOutputPin(UiNodePtr node, UiNodePtr inputNode, UiPinPtr input);

    void drawOutputPins(UiNodePtr node, const std::string& longestInputLabel);

    // Create pins for outputs/inputs added while inside the node graph
    void addNodeGraphPins();

    std::vector<int> createNodes(bool nodegraph);
    int getNodeId(ed::PinId pinId);

    // Find node location in graph nodes vector from node id
    int findNode(int nodeId);

    // Return node position in _graphNodes from node name and type to account for
    // input/output UiNodes with same names as MaterialX nodes
    int findNode(const std::string& name, const std::string& type);

    // Add node to graphNodes based on nodedef information
    void addNode(const std::string& category, const std::string& name, const std::string& type);

    void deleteNode(UiNodePtr node);

    // Build the initial graph of a loaded document including shader, material and nodegraph node
    void setUiNodeInfo(UiNodePtr node, const std::string& type, const std::string& category);

    // Check if edge exists in edge vector
    bool edgeExists(UiEdge edge);

    void createEdge(UiNodePtr upNode, UiNodePtr downNode, mx::InputPtr connectingInput);

    // Remove node edge based on connecting input
    void removeEdge(int downNode, int upNode, UiPinPtr pin);

    void saveDocument(mx::FilePath filePath);

    // Set position attributes for nodes which changed position
    void savePosition();

    // Check if node has already been assigned a position
    bool checkPosition(UiNodePtr node);

    // Add input pointer to node based on input pin
    void addNodeInput(UiNodePtr node, mx::InputPtr& input);

    mx::InputPtr findInput(mx::InputPtr input, const std::string& name);
    void upNodeGraph();

    // Set the value of the selected node constants in the node property editor
    void setConstant(UiNodePtr node, mx::InputPtr& input, const mx::UIProperties& uiProperties);

    void propertyEditor();
    void setDefaults(mx::InputPtr input);

    // Setup UI information for add node popup
    void addExtraNodes();

    void copyInputs();

    // Set position of pasted nodes based on original node position
    void positionPasteBin(ImVec2 pos);

    void copyNodeGraph(UiNodePtr origGraph, UiNodePtr copyGraph);
    void copyUiNode(UiNodePtr node);

    void graphButtons();

    /// @brief Constructs the popup menu to add a new node to the graph
    /// @param originPin An optional pointer to the pin currently being clicked (if any). 
    ///                  \p originPin will be non-null if a user tries to add a node by 
    ///                  dragging a new link off of an existing pin. That pin is thus the
    ///                  "origin" of the new node.
    void addNodePopup(UiPinPtr originPin = nullptr);

    void searchNodePopup();
    bool isPinHovered();
    void addPinPopup();
    bool readOnly();
    void readOnlyPopup();

    // Compiling shaders message
    void shaderPopup();

    void updateMaterials(mx::InputPtr input = nullptr, mx::ValuePtr value = nullptr);
    void selectMaterial(UiNodePtr node);

    // Allow for camera manipulation of render view window
    void handleRenderViewInputs();

    // Set the node to display in render view based on selected node or nodegraph
    void setRenderMaterial(UiNodePtr node);

    void clearGraph();
    void loadGraphFromFile(bool prompt);
    void saveGraphToFile();
    void loadGeometry();

    void showHelp() const;

    /// @brief Determines if \p node can be added to the graph considering the filtering context
    /// @param node           Name of a node we might want to add to the graph
    /// @param filterString   User-entered search string which must appear EXACTLY in the node name.
    /// @param originPin      The pin from which this new node is being created. \p originPin 
    ///                       will be a valid pointer if a user is trying to add a node by drawing
    ///                       a link off of an existing node's pin (the "originating" pin).
    /// @return               True iff node can be added to the graph considering filtering state.
    bool isValidNodeToAdd(const std::string& node, const std::string& filterString, UiPinPtr originPin = nullptr);
    
    // Track the inputs and outputs for a node named `nodeName` in the graph's `_nodeIOMap`. This operation
    // is performed once for every possible node that may be added to the graph upon the graph's construction.
    void recordIOForNode(const std::string& name, const std::string& type, const ed::PinKind ioKind, const std::string& nodeName);

  private:
    mx::StringVec _geomFilter;
    mx::StringVec _mtlxFilter;
    mx::StringVec _imageFilter;

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
    std::vector<MenuItem> _nodesToAdd;

    // Maps node names to a struct containing their corresponding input and output pins by type 
    // for every node that can be added to our graph.
    std::unordered_map<std::string, PinOrganizer> _nodeIOMap;

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
    std::string _fileDialogImageInputName;

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

    // DPI scaling for fonts
    float _fontScale;

    // Options
    bool _saveNodePositions;

    // Variables for filtering connections based on the current state of the node graph.
    
    // if _smartFilter is true, filter the addNodePopup UI to only contain
    // nodes that can be added based on the currently selected pin.
    bool _smartFilter;

    // The pin from which a new link is currently being drawn.
    UiPinPtr _originPin;
    
    // used for filtering pins when connecting links
    // _pinFilterType: The "type" of the currently selected pin (ex. bool, material, bsdf)
    // _pinFilterKind: The "kind" of the currently selected pin (ex. Input or Output).
    std::string _pinFilterType;
};

#endif
