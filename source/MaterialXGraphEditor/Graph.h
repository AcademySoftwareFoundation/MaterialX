//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GRAPH_H
#define MATERIALX_GRAPH_H

#include <MaterialXGraphEditor/FileDialog.h>
#include <MaterialXGraphEditor/Layout.h>
#include <MaterialXGraphEditor/RenderView.h>
#include <MaterialXGraphEditor/UiNode.h>

#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;
namespace mx = MaterialX;

class MenuItem
{
  public:
    MenuItem(const std::string& name, const std::string& type, const std::string& category, const std::string& group, const std::set<std::string>& inputTypes, const std::set<std::string>& outputTypes) :
        name(name), type(type), category(category), group(group), inputTypes(inputTypes), outputTypes(outputTypes) { }

    // getters
    std::string getName() const { return name; }
    std::string getType() const { return type; }
    std::string getCategory() const { return category; }
    std::string getGroup() const { return group; }
    const std::set<std::string>& getInputTypes() const { return inputTypes; }
    const std::set<std::string>& getOutputTypes() const { return outputTypes; }

    // setters
    void setName(const std::string& newName) { this->name = newName; }
    void setType(const std::string& newType) { this->type = newType; }
    void setCategory(const std::string& newCategory) { this->category = newCategory; }
    void setGroup(const std::string& newGroup) { this->group = newGroup; }
    void setInputTypes(const std::set<std::string>& newInputTypes) { this->inputTypes = newInputTypes; }
    void setOutputTypes(const std::set<std::string>& newOutputTypes) { this->outputTypes = newOutputTypes; }

  private:
    std::string name;
    std::string type;
    std::string category;
    std::string group;
    std::set<std::string> inputTypes;
    std::set<std::string> outputTypes;
};

// A link connects two pins and includes a unique id and the ids of the two pins it connects
// Based on the Link struct from ImGui Node Editor blueprints-examples.cpp
struct Link
{
    Link(int id, int startAttr, int endAttr) :
        _id(id),
        _startAttr(startAttr),
        _endAttr(endAttr)
    {
    }

    int _id;
    int _startAttr, _endAttr;
};

// The UI state associated with a graph level (document or nodegraph).
struct GraphState
{
    // Display name for this graph level.
    std::string name;

    // MaterialX graph element for this level.
    mx::GraphElementPtr graphElem;
    bool isCompoundNodeGraph = false;

    // UI nodes and pins within this graph.
    std::vector<UiNodePtr> nodes;
    std::vector<UiPinPtr> pins;

    // Links and edges representing connections within this graph.
    std::vector<Link> links;
    std::vector<UiEdge> edges;

    // Counter for generating unique UI element IDs.
    int nextUiId = 1;
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
    ~Graph() = default;

    mx::DocumentPtr loadDocument(const mx::FilePath& filename);
    void drawGraph(ImVec2 mousePos);

    RenderViewPtr getRenderer()
    {
        return _renderer;
    }

    void setFontScale(float val)
    {
        _fontScale = val;
    }

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

    // Check if link exists in the current link vector
    bool linkExists(const Link& newLink);

    // Check if link can be added. Show a diagnostic message as the label.
    bool checkCanAddLink(ed::PinId startPinId, ed::PinId endPinId);

    // Add link to nodegraph and set up connections between UiNodes and
    // MaterialX Nodes to update shader
    // startPinId - where the link was initiated
    // endPinId - where the link was ended
    void addLink(ed::PinId startPinId, ed::PinId endPinId);

    // Delete link from current link vector and remove any connections in
    // UiNode or MaterialX Nodes to update shader
    void deleteLink(ed::LinkId deletedLinkId);

    void deleteLinkInfo(int startAtrr, int endAttr);

    // Apply the layout engine to position all nodes.
    void applyLayout(const std::vector<int>& outputNodeIndices);

    // Return pin color based on the type of the value of that pin
    void setPinColor();

    // Based on the pin icon function in the ImGui Node Editor blueprints-example.cpp
    void drawPinIcon(const std::string& type, bool connected, int alpha);

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

    // Return node position in current state's nodes from node name and type to
    // account for input/output UiNodes with same names as MaterialX nodes
    int findNode(const std::string& name, const std::string& type);

    // Add node to graphNodes based on nodedef information
    void addNode(const std::string& category, const std::string& name, const std::string& type);

    void deleteNode(UiNodePtr node);

    // Build the initial graph of a loaded document including shader, material and nodegraph node
    void setUiNodeInfo(UiNodePtr node, const std::string& type, const std::string& category);

    // Check if edge exists in edge vector
    bool edgeExists(const UiEdge& edge);

    // Create an edge between two nodes if it doesn't already exist.
    // Returns true if the edge was created, false if invalid or already exists.
    bool createEdge(UiNodePtr upNode, UiNodePtr downNode, mx::InputPtr connectingInput);

    // Remove node edge based on connecting input
    void removeEdge(int downNode, int upNode, UiPinPtr pin);

    void saveDocument(mx::FilePath filePath);

    // Set position attributes for nodes which changed position
    void savePosition();

    // Restore node positions from MaterialX element attributes.
    void restorePositions();

    // Add an input to a node based on its NodeDef input definition.
    mx::InputPtr addNodeInput(UiNodePtr node, mx::InputPtr nodeDefInput);

    // Traversal methods
    void upNodeGraph();
    UiNodePtr traverseConnection(UiNodePtr node, bool traverseDownstream);

    // Show input values in property editor for a given input
    void showPropertyEditorValue(UiNodePtr node, mx::InputPtr input, const mx::UIProperties& uiProperties);
    // Show input connections in property editor for a given node
    void showPropertyEditorOutputConnections(UiNodePtr node);
    // Show output connections in property editor for a given output pin
    void showPropertyEditorInputConnection(UiPinPtr pin);
    // Show property editor for a given node
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

    void addNodePopup(bool cursor);
    void searchNodePopup(bool cursor);
    bool isPinHovered();
    void addPinPopup();
    bool readOnly();
    void readOnlyPopup();

    // Compiling shaders message
    void shaderPopup();

    void updateMaterials(mx::InputPtr input = nullptr, mx::ValuePtr value = nullptr);

    // Allow for camera manipulation of render view window
    void handleRenderViewInputs();

    // Set the node to display in render view based on selected node or nodegraph
    void setRenderMaterial(UiNodePtr node);

    void clearGraph();
    void loadGraphFromFile(bool prompt);
    void saveGraphToFile();
    void loadGeometry();

    // Initialize the graph state from the current document.
    void initializeGraph();

    void showHelp() const;

  private:
    mx::StringVec _geomFilter;
    mx::StringVec _mtlxFilter;
    mx::StringVec _imageFilter;

    RenderViewPtr _renderer;

    // document and initializing information
    mx::FilePath _materialFilename;
    mx::DocumentPtr _graphDoc;
    mx::StringSet _xincludeFiles;

    mx::FileSearchPath _searchPath;
    mx::FilePathVec _libraryFolders;
    mx::DocumentPtr _stdLib;

    // image information
    mx::ImagePtr _image;
    mx::ImageHandlerPtr _imageHandler;

    // Auxiliary node information.
    std::unordered_map<UiNodePtr, std::vector<UiPinPtr>> _downstreamInputs;
    std::unordered_map<std::string, ImColor> _pinColor;

    // Current graph state, including nodes, pins, and navigation context.
    GraphState _state;

    // current nodes and nodegraphs
    UiNodePtr _currUiNode;
    UiNodePtr _prevUiNode;
    UiNodePtr _currRenderNode;

    // for adding new nodes
    std::vector<MenuItem> _nodesToAdd;

    // Saved states of parent graphs for navigating the graph hierarchy.
    std::vector<GraphState> _parentStates;

    // map for copied nodes
    std::map<UiNodePtr, UiNodePtr> _copiedNodes;

    bool _needsLayout;
    bool _layoutPending;
    bool _needsNavigation;
    bool _delete;

    // file dialog information
    FileDialog _fileDialog;
    FileDialog _fileDialogSave;
    FileDialog _fileDialogImage;
    FileDialog _fileDialogGeom;
    std::string _fileDialogImageInputName;

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
    // used for filtering pins when adding a node from a link
    std::string _menuFilterType;
    // used for auto connecting pins if a node is added by drawing a link from a pin
    ed::PinId _pinIdToLinkFrom;
    ed::PinId _pinIdToLinkTo;

    // DPI scaling for fonts
    float _fontScale;

    // Layout engine
    Layout _layout;

    // Options
    bool _saveNodePositions;
};

#endif
