#include <MaterialXGraphEditor/Graph.h>

#include <MaterialXRenderGlsl/External/Glad/glad.h>

#include <GLFW/glfw3.h>

// the default node size is based off the of the size of the dot_color3 node using ed::getNodeSize() on that node
const ImVec2 DEFAULT_NODE_SIZE = ImVec2(138, 116);
const int DEFAULT_ALPHA = 255;
const int FILTER_ALPHA = 50;
static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

Graph::Graph(const std::string& materialFilename, const mx::FileSearchPath& searchPath, const mx::FilePathVec& libraryFolders) :
    _materialFilename(materialFilename),
    _searchPath(searchPath),
    _libraryFolders(libraryFolders),
    _initial(false),
    _delete(false),
    _fileDialogSave(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir),
    _isNodeGraph(false),
    _graphTotalSize(0),
    _popup(false),
    _searchNodeId(-1),
    _addNewNode(false),
    _ctrlClick(false),
    _isCut(false),
    _autoLayout(false),
    _pinFilterType("")

{
}

void Graph::loadStandardLibraries()
{
    // Initialize the standard library.
    try
    {
        _stdLib = mx::createDocument();
        _xincludeFiles = mx::loadLibraries(_libraryFolders, _searchPath, _stdLib);
        if (_xincludeFiles.empty())
        {
            std::cerr << "Could not find standard data libraries on the given search path: " << _searchPath.asString() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to load standard data libraries: " << e.what() << std::endl;
        return;
    }
}
void Graph::initialize()
{
    loadStandardLibraries();

    // load nodes def to create add node ui
    std::vector<mx::NodeDefPtr> nodeDefs = _stdLib->getNodeDefs();
    for (size_t i = 0; i < nodeDefs.size(); i++)
    {
        // nodeDef is the key for the map
        std::string group = nodeDefs[i]->getNodeGroup();
        if (group == "")
        {
            group = "extra";
        }
        std::unordered_map<std::string, std::vector<mx::NodeDefPtr>>::iterator it = _nodesToAdd.find(group);
        if (it == _nodesToAdd.end())
        {
            std::vector<mx::NodeDefPtr> nodes;
            _nodesToAdd[group] = nodes;
        }
        _nodesToAdd[group].push_back(nodeDefs[i]);
    }

    mx::FilePath captureFilename = "resources/Materials/Examples/example.png";
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    std::string meshFilename = "resources/Geometry/shaderball.glb";
    std::string envRadianceFilename = "resources/Lights/san_giuseppe_bridge_split.hdr";
    mx::Color3 screenColor(1.0f, 0.3f, 0.32f);

    _renderer = std::make_shared<RenderView>(materialFilename, meshFilename, envRadianceFilename, _searchPath, _libraryFolders, 1280, 960);
    _renderer->initialize();
}

mx::DocumentPtr Graph::loadDocument(std::string filename)
{
    std::string materialFilename = filename;
    mx::FilePathVec libraryFolders = { "libraries" };
    _libraryFolders = libraryFolders;
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const mx::FilePath& filename,
                                          const mx::FileSearchPath& searchPath, const mx::XmlReadOptions* options)
    {
        mx::FilePath resolvedFilename = searchPath.find(filename);
        if (resolvedFilename.exists())
        {
            readFromXmlFile(doc, resolvedFilename, searchPath, options);
        }
        else
        {
            std::cerr << "Include file not found: " << filename.asString() << std::endl;
        }
    };

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, materialFilename, _searchPath, &readOptions);
    _graphStack = std::stack<std::vector<UiNodePtr>>();
    return doc;
}

// populate nodes to add with input output group and nodegraph nodes which are not found in the stdlib
void Graph::addExtraNodes()
{
    std::vector<std::string> groups{ "Input Nodes", "Output Nodes", "Group Nodes", "Node Graph" };
    std::vector<std::string> types{
        "float", "integer", "vector2", "vector3", "vector4", "color3", "color4", "string", "filename", "bool"
    };
    // need to clear vectors if has previously used tab without there being a document, need to use the current graph doc
    for (std::string group : groups)
    {
        if (_extraNodes[group].size() > 0)
        {
            _extraNodes[group].clear();
        }
    }
    for (std::string type : types)
    {

        std::string nodeName = "ND_input";
        nodeName += type;
        std::vector<std::string> input{ nodeName, type, "input" };
        _extraNodes["Input Nodes"].push_back(input);
        nodeName = "ND_output";
        nodeName += type;
        std::vector<std::string> output{ nodeName, type, "output" };
        _extraNodes["Output Nodes"].push_back(output);
    }
    // group node
    std::vector<std::string> groupNode{ "ND_Group", "", "group" };
    _extraNodes["Group Nodes"].push_back(groupNode);
    // node graph nodes
    std::vector<std::string> nodeGraph{ "ND_Node Graph", "", "nodegraph" };
    _extraNodes["Node Graph"].push_back(nodeGraph);
}
// return output pin needed to link the inputs and outputs
ed::PinId Graph::getOutputPin(UiNodePtr node, UiNodePtr upNode, Pin input)
{
    if (upNode->getNodeGraph() != nullptr)
    {
        // for nodegraph need to get the correct ouput pin accorinding to the names of the output nodes
        mx::OutputPtr output = input._pinNode->getNode()->getConnectedOutput(input._name);
        if (output)
        {
            std::string outName = input._pinNode->getNode()->getConnectedOutput(input._name)->getName();
            for (Pin outputs : upNode->outputPins)
            {
                if (outputs._name == outName)
                {
                    return outputs._pinId;
                }
            }
        }
        return ed::PinId();
    }
    else
    {
        // every other node can just get the first output pin since there is only one
        return (upNode->outputPins[0]._pinId);
    }
}

// connect links via connected nodes in UiNodePtr
void Graph::linkGraph()
{
    _currLinks.clear();
    // start with bottom of graph
    for (UiNodePtr node : _graphNodes)
    {
        std::vector<Pin> inputs = node->inputPins;
        if (node->getInput() == nullptr)
        {
            for (size_t i = 0; i < inputs.size(); i++)
            {
                // get upstream node for all inputs
                std::string inputName = inputs[i]._name;

                UiNodePtr inputNode = node->getConnectedNode(inputName);
                if (inputNode != nullptr)
                {
                    // set current node position

                    Link link;
                    // getting the input connections for the current uiNode
                    ax::NodeEditor::PinId id = inputs[i]._pinId;
                    inputs[i].setConnected(true);
                    int end = int(id.Get());
                    link._endAttr = end;
                    // get id number of output of node

                    int start = int(getOutputPin(node, inputNode, inputs[i]).Get());
                    if (start >= 0)
                    {
                        inputNode->outputPins[0].setConnected(true);
                        link._startAttr = start;

                        if (!linkExists(link))
                        {
                            _currLinks.push_back(link);
                        }
                    }
                }
                else if (inputs[i]._input)
                {
                    if (inputs[i]._input->getInterfaceInput())
                    {

                        inputs[i].setConnected(true);
                    }
                }
                else
                {
                    inputs[i].setConnected(false);
                }
            }
        }
    }
}

// connect all the links via the graph editor library
void Graph::connectLinks()
{

    for (Link const& link : _currLinks)
    {

        ed::Link(link.id, link._startAttr, link._endAttr);
    }
}

// find link position in currLinks vector from link id
int Graph::findLinkPosition(int id)
{

    int count = 0;
    for (size_t i = 0; i < _currLinks.size(); i++)
    {
        if (_currLinks[i].id == id)
        {
            return count;
        }
        count++;
    }
    return -1;
}
// check if a node has already been assigned a position
bool Graph::checkPosition(UiNodePtr node)
{
    if (node->getMxElement() != nullptr)
    {
        if (node->getMxElement()->getAttribute("xpos") != "")
        {
            return true;
        }
    }
    return false;
}
// calculate the total vertical space the node level takes up
float Graph::totalHeight(int level)
{
    float total = 0.f;
    for (UiNodePtr node : _levelMap[level])
    {
        total += ed::GetNodeSize(node->getId()).y;
    }
    return total;
}
// set the y position of node based of the starting position and the nodes above it
void Graph::setYSpacing(int level, float startingPos)
{
    // set the y spacing for each node
    float currPos = startingPos;
    for (UiNodePtr node : _levelMap[level])
    {
        ImVec2 oldPos = ed::GetNodePosition(node->getId());
        ed::SetNodePosition(node->getId(), ImVec2(oldPos.x, currPos));
        currPos += ed::GetNodeSize(node->getId()).y + 40;
    }
}

// calculate the average y position for a specific node level
float Graph::findAvgY(const std::vector<UiNodePtr>& nodes)
{
    // find the mid point of node level grou[
    float total = 0.f;
    int count = 0;
    for (UiNodePtr node : nodes)
    {
        ImVec2 pos = ed::GetNodePosition(node->getId());
        ImVec2 size = ed::GetNodeSize(node->getId());

        total += ((size.y + pos.y) + pos.y) / 2;
        count++;
    }
    return (total / count);
}

void Graph::findYSpacing(float startY)

{
    // assume level 0 is set
    // for each level find the average y position of the previous level to use as a spacing guide
    int i = 0;
    for (std::pair<int, std::vector<UiNodePtr>> levelChunk : _levelMap)
    {
        if (_levelMap[i].size() > 0)
        {
            if (_levelMap[i][0]->_level > 0)
            {

                int prevLevel = _levelMap[i].front()->_level - 1;
                float avgY = findAvgY(_levelMap[prevLevel]);
                float height = totalHeight(_levelMap[i].front()->_level);
                // caculate the starting position to be above the previous level's center so that it is evenly spaced on either side of the center
                float startingPos = avgY - ((height + (_levelMap[i].size() * 20)) / 2) + startY;
                setYSpacing(_levelMap[i].front()->_level, startingPos);
            }
            else
            {
                setYSpacing(_levelMap[i].front()->_level, startY);
            }
        }
        ++i;
    }
}

// layout the x position by assigning the node levels based off its distance from the first node
ImVec2 Graph::layoutPosition(UiNodePtr layoutNode, ImVec2 startingPos, bool initialLayout, int level)
{

    if (checkPosition(layoutNode) && !_autoLayout)
    {
        for (UiNodePtr node : _graphNodes)
        {
            // since nodegrpah nodes do not have any materialX info they are placed based off their conneced node
            if (node->getNodeGraph() != nullptr)
            {
                std::vector<UiNodePtr> outputCon = node->getOutputConnections();
                if (outputCon.size() > 0)
                {
                    ImVec2 outputPos = ed::GetNodePosition(outputCon[0]->getId());
                    ed::SetNodePosition(node->getId(), ImVec2(outputPos.x - 400, outputPos.y));
                    node->setPos(ImVec2(outputPos.x - 400, outputPos.y));
                }
            }
            else
            {
                float x = std::stof(node->getMxElement()->getAttribute("xpos"));
                float y = std::stof(node->getMxElement()->getAttribute("ypos"));
                x *= DEFAULT_NODE_SIZE.x;
                y *= DEFAULT_NODE_SIZE.y;
                ed::SetNodePosition(node->getId(), ImVec2(x, y));
                node->setPos(ImVec2(x, y));
            }
        }
        return ImVec2(0.f, 0.f);
    }
    else
    {
        ImVec2 currPos = startingPos;
        ImVec2 newPos = currPos;
        if (layoutNode->_level != -1)
        {
            if (layoutNode->_level < level)
            {
                // remove the old instance of the node from the map
                int levelNum = 0;
                int removeNum = -1;
                for (UiNodePtr levelNode : _levelMap[layoutNode->_level])
                {
                    if (levelNode->getName() == layoutNode->getName())
                    {
                        removeNum = levelNum;
                    }
                    levelNum++;
                }
                if (removeNum > -1)
                {
                    _levelMap[layoutNode->_level].erase(_levelMap[layoutNode->_level].begin() + removeNum);
                }

                layoutNode->_level = level;
            }
        }
        else
        {
            layoutNode->_level = level;
        }

        if (_levelMap.find(layoutNode->_level) != _levelMap.end())
        {
            // key already exists add to it
            if ((!std::count(_levelMap[layoutNode->_level].begin(), _levelMap[layoutNode->_level].end(), layoutNode)))
            {
                _levelMap[layoutNode->_level].push_back(layoutNode);
            }
        }
        else
        {
            // insert new vector into key
            std::vector<UiNodePtr> newValue = { layoutNode };
            _levelMap.insert({ layoutNode->_level, newValue });
        }
        std::vector<Pin> pins = layoutNode->inputPins;
        if (initialLayout)
        {
            // check number of inputs that are connected to node
            if (layoutNode->getInputConnect() > 0)
            {
                // not top of node graph stop recursion
                if (pins.size() != 0 && layoutNode->getInput() == nullptr)
                {
                    int numNode = 0;
                    for (size_t i = 0; i < pins.size(); i++)
                    {
                        // get upstream node for all inputs
                        newPos = startingPos;
                        UiNodePtr nextNode = layoutNode->getConnectedNode(pins[i]._name);
                        if (nextNode)
                        {
                            startingPos.x = 1200.f - ((layoutNode->_level) * 350);
                            // pos.y = 0;
                            ed::SetNodePosition(layoutNode->getId(), startingPos);
                            layoutNode->setPos(ImVec2(startingPos));

                            newPos.x = 1200.f - ((layoutNode->_level + 1) * 75);
                            ++numNode;
                            // call layout position on upstream node with newPos as -140 to the left of current node
                            layoutPosition(nextNode, ImVec2(newPos.x, startingPos.y), initialLayout, layoutNode->_level + 1);
                        }
                    }
                }
            }
            else
            {
                startingPos.x = 1200.f - ((layoutNode->_level) * 350);
                layoutNode->setPos(ImVec2(startingPos));
                ////set current node position
                ed::SetNodePosition(layoutNode->getId(), ImVec2(startingPos));
            }
        }
        return ImVec2(0.f, 0.f);
    }
}

// extra layout pass for inputs and nodes that do not attach to an output node
void Graph::layoutInputs()
{
    // layout inputs after other nodes so that they can be all in a line on far left side of node graph
    if (_levelMap.begin() != _levelMap.end())
    {
        int levelCount = -1;
        for (std::pair<int, std::vector<UiNodePtr>> nodes : _levelMap)
        {
            ++levelCount;
        }
        ImVec2 startingPos = ed::GetNodePosition(_levelMap[levelCount].back()->getId());
        startingPos.y += ed::GetNodeSize(_levelMap[levelCount].back()->getId()).y + 20;

        for (UiNodePtr uiNode : _graphNodes)
        {

            if (uiNode->getOutputConnections().size() == 0 && (uiNode->getInput() != nullptr))
            {
                ed::SetNodePosition(uiNode->getId(), ImVec2(startingPos));
                startingPos.y += ed::GetNodeSize(uiNode->getId()).y;
                startingPos.y += 23;
            }
            // accoutning for extra nodes like in gltf
            else if (uiNode->getOutputConnections().size() == 0 && (uiNode->getNode() != nullptr))
            {
                if (uiNode->getNode()->getCategory() != mx::SURFACE_MATERIAL_NODE_STRING)
                {
                    layoutPosition(uiNode, ImVec2(1200, 750), _initial, 0);
                }
            }
        }
    }
}

// reutrn pin color based on the type of the value of that pin
ImColor Graph::getPinColor(std::string type)
{

    if (type == "integer")
    {
        return ImColor(255, 255, 28, 255);
    }
    else if (type == "bool")
    {
        return ImColor(255, 0, 255, 255);
    }
    else if (type == "float")
    {
        return ImColor(50, 100, 255, 255);
    }
    else if (type == "color3")
    {
        return ImColor(178, 34, 34, 255);
    }
    else if (type == "color4")
    {
        return ImColor(50, 10, 255, 255);
    }
    else if (type == "vector2")
    {
        return ImColor(100, 255, 100, 255);
    }
    else if (type == "vector3")
    {
        return ImColor(0, 255, 0, 255);
    }
    else if (type == "vector4")
    {
        return ImColor(100, 0, 100, 255);
    }
    else if (type == "matrix33")
    {
        return ImColor(0, 100, 100, 255);
    }
    else if (type == "matrix44")
    {
        return ImColor(50, 255, 100, 255);
    }
    else if (type == "filename")
    {
        return ImColor(255, 184, 28, 255);
    }
    else if (type == "intVec")
    {
        return ImColor(100, 100, 50, 255);
    }
    else if (type == "BoolVec")
    {
        return ImColor(121, 60, 180, 255);
    }
    else if (type == "FloatVec")
    {
        return ImColor(10, 181, 150, 255);
    }
    else if (type == "StringVec")
    {
        return ImColor(255, 50, 100, 255);
    }
    else if (type == "long")
    {
        return ImColor(0, 100, 151, 255);
    }
    else if (type == "double")
    {
        return ImColor(150, 255, 255, 255);
    }
    else if (type == "material")
    {
        return ImColor(255, 255, 255, 255);
    }
    else if (type == "string")
    {
        return ImColor(155, 50, 100, 255);
    }
    else if (type == "surfaceshader")
    {
        return ImColor(155, 150, 100, 255);
    }
    else if (type == "")
    {
        return ImColor(70, 70, 70);
    }
    else
    {
        return ImColor(0, 0, 0, 0);
    }
}

auto showLabel = [](const char* label, ImColor color)
{
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
    auto size = ImGui::CalcTextSize(label);

    auto padding = ImGui::GetStyle().FramePadding;
    auto spacing = ImGui::GetStyle().ItemSpacing;

    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

    auto rectMin = ImGui::GetCursorScreenPos() - padding;
    auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
    ImGui::TextUnformatted(label);
};

// set the node to display in render veiw based off the selected node or nodegraph
void Graph::setRenderMaterial(UiNodePtr node)
{
    // set render node right away is node is a material
    if (node->getNode() && node->getNode()->getType() == "material")
    {
        _currRenderNode = node;
        updateMaterials(nullptr, nullptr, true);
    }
    else
    {
        // continue downstream using output connections until a material node is found
        std::vector<UiNodePtr> outNodes = node->getOutputConnections();
        if (outNodes.size() > 0)
        {
            if (outNodes[0]->getNode())
            {
                if (outNodes[0]->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                {
                    std::vector<UiNodePtr> shaderOut = outNodes[0]->getOutputConnections();
                    if (shaderOut.size() > 0)
                    {
                        if (shaderOut[0])
                        {
                            std::string type = shaderOut[0]->getNode()->getType();
                            if (shaderOut[0]->getNode()->getType() == "material")
                            {
                                _currRenderNode = shaderOut[0];
                                updateMaterials(nullptr, nullptr, true);
                            }
                        }
                    }
                    else
                    {
                        _currRenderNode = nullptr;
                    }
                }
                else if (outNodes[0]->getNode()->getType() == mx::MATERIAL_TYPE_STRING)
                {
                    _currRenderNode = outNodes[0];
                    updateMaterials(nullptr, nullptr, true);
                }
            }
            else
            {
                _currRenderNode = nullptr;
            }
        }
        else
        {
            _currRenderNode = nullptr;
        }
    }
}

void Graph::updateMaterials(mx::InputPtr input, mx::ValuePtr value, bool genShader)
{

    std::string renderablePaths;
    std::vector<mx::TypedElementPtr> elems;
    std::vector<mx::NodePtr> materialNodes;
    mx::findRenderableElements(_graphDoc, elems);
    if (elems.size() > 0 && _renderer->getMaterials().size() == 0)
    {
        _renderer->updateMaterials(_graphDoc);
    }

    size_t num = 0;
    for (mx::TypedElementPtr elem : elems)
    {
        mx::TypedElementPtr renderableElem = elem;
        mx::NodePtr node = elem->asA<mx::Node>();
        if (node)
        {
            if (_currRenderNode)
            {
                if (node->getName() == _currRenderNode->getName())
                {
                    std::string name = node->getName();
                    materialNodes.push_back(node->getType() == mx::MATERIAL_TYPE_STRING ? node : nullptr);
                    renderablePaths = renderableElem->getNamePath();
                    break;
                }
            }
            else
            {
                std::string name = node->getName();
                materialNodes.push_back(node->getType() == mx::MATERIAL_TYPE_STRING ? node : nullptr);
                renderablePaths = renderableElem->getNamePath();
            }
        }
    }
    if (genShader)
    {
        if (renderablePaths != "")
        {
            mx::ElementPtr elem = _graphDoc->getDescendant(renderablePaths);
            mx::TypedElementPtr typedElem = elem ? elem->asA<mx::TypedElement>() : nullptr;
            _renderer->getMaterials()[num]->setElement(typedElem);
            _renderer->getMaterials()[num]->setMaterialNode(materialNodes[0]);
            _renderer->getMaterials()[num]->generateShader(_renderer->getGenContext());
        }
    }
    else
    {
        std::string name = input->getNamePath();
        // need to use exact interface name in order for input
        mx::InputPtr interfaceInput = findInput(input, input->getName());
        if (interfaceInput)
        {
            name = interfaceInput->getNamePath();
        }
        _renderer->getMaterials()[num]->modifyUniform(name, value);
    }
}
// set the value of the selected node constants in the node property editor
void Graph::setConstant(UiNodePtr node, mx::InputPtr& input)
{
    std::string type = input->getType();
    std::string inName = input->getName();
    float labelWidth = ImGui::CalcTextSize(inName.c_str()).x;
    // if input is a float set the float slider Ui to the value
    if (input->getType() == "float")
    {
        mx::ValuePtr val = input->getValue();

        if (val && val->isA<float>())
        {
            // updates the value to the default for new nodes
            float prev = val->asA<float>(), temp = val->asA<float>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 20);
            ImGui::DragFloat("##hidelabel", &temp, 0.01f, 0.f, 100.f);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "integer")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<int>())
        {
            int prev = val->asA<int>(), temp = val->asA<int>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 20);
            ImGui::DragInt("##hidelabel", &temp, 1, 0, 100);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "color3")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<mx::Color3>())
        {
            mx::Color3 prev = val->asA<mx::Color3>(), temp = val->asA<mx::Color3>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 100);
            ImGui::DragFloat3("##hidelabel", &temp[0], 0.01f, 0.f, 100.f);
            ImGui::SameLine();
            ImGui::ColorEdit3("##color", &temp[0], ImGuiColorEditFlags_NoInputs);
            ImGui::PopItemWidth();

            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "color4")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<mx::Color4>())
        {
            mx::Color4 prev = val->asA<mx::Color4>(), temp = val->asA<mx::Color4>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 100);
            ImGui::DragFloat4("##hidelabel", &temp[0], 0.01f, 0.f, 100.f);
            ImGui::SameLine();
            // color edit for the color picker to the right of the color floats
            ImGui::ColorEdit4("##color", &temp[0], ImGuiColorEditFlags_NoInputs);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (temp != prev)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "vector2")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<mx::Vector2>())
        {
            mx::Vector2 prev = val->asA<mx::Vector2>(), temp = val->asA<mx::Vector2>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 100);
            ImGui::DragFloat2("##hidelabel", &temp[0], 0.01f, 0.f, 100.f);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "vector3")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<mx::Vector3>())
        {
            mx::Vector3 prev = val->asA<mx::Vector3>(), temp = val->asA<mx::Vector3>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 100);
            ImGui::DragFloat3("##hidelabel", &temp[0], 0.01f, 0.f, 100.f);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "vector4")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<mx::Vector4>())
        {
            mx::Vector4 prev = val->asA<mx::Vector4>(), temp = val->asA<mx::Vector4>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth + 90);
            ImGui::DragFloat4("##hidelabel", &temp[0], 0.01f, 0.f, 100.f);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "string")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<std::string>())
        {
            std::string prev = val->asA<std::string>(), temp = val->asA<std::string>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth);
            ImGui::InputText("##constant", &temp);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
    else if (input->getType() == "filename")
    {
        mx::ValuePtr val = input->getValue();

        if (val && val->isA<std::string>())
        {
            std::string temp = val->asA<std::string>(), prev = val->asA<std::string>();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.15f, .15f, .15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.2f, .4f, .6f, 1.0f));
            // browser button to select new file
            if (ImGui::Button("Browse"))
            {
                _fileDialogConstant.SetTitle("Node Input Dialog");
                _fileDialogConstant.Open();
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth);
            ImGui::Text("%s", mx::FilePath(temp).getBaseName().c_str());
            ImGui::PopItemWidth();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();

            // create and load document from selected file
            if (_fileDialogConstant.HasSelected())
            {
                // set the new filename to the complete file path
                mx::FilePath fileName = mx::FilePath(_fileDialogConstant.GetSelected().string());
                temp = fileName;
                // need to set the file prefix for the input to "" so that it can find the new file
                input->setAttribute(input->FILE_PREFIX_ATTRIBUTE, "");
                _fileDialogConstant.ClearSelected();
            }

            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValueString(temp);
                std::string test = input->getValueString();
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), true);
            }
        }
    }
    else if (input->getType() == "boolean")
    {
        mx::ValuePtr val = input->getValue();
        if (val && val->isA<bool>())
        {
            bool prev = val->asA<bool>(), temp = val->asA<bool>();
            ImGui::SameLine();
            ImGui::PushItemWidth(labelWidth);
            ImGui::Checkbox("", &temp);
            ImGui::PopItemWidth();
            // set input value  and update materials if different from previous value
            if (prev != temp)
            {
                input->setValue(temp, input->getType());
                updateMaterials(input, input->getValue(), false);
            }
        }
    }
}
// build the initial graph of a loaded mtlx document including shader, material and nodegraph node
void Graph::setUiNodeInfo(UiNodePtr node, std::string type, std::string category)
{
    node->setType(type);
    node->setCategory(category);
    _graphNodes.push_back(std::move(node));
    ++_graphTotalSize;
}
void Graph::buildUiBaseGraph(const std::vector<mx::NodeGraphPtr>& nodeGraphs, const std::vector<mx::NodePtr>& docNodes, const std::vector<mx::InputPtr>& inputNodes)
{
    _graphNodes.clear();
    _currLinks.clear();
    _currEdge.clear();
    _newLinks.clear();
    _graphTotalSize = 0;
    int id = 1;
    // creating uiNodes for nodes that belong to the document so they are not in a nodegraph
    for (mx::NodePtr node : docNodes)
    {
        std::string name = node->getName();
        auto currNode = std::make_shared<UiNode>(name, id);
        currNode->setNode(node);
        ++id;
        setUiNodeInfo(currNode, node->getType(), node->getCategory());
    }
    // creating uiNodes for the nodegraph
    for (mx::NodeGraphPtr nodeGraph : nodeGraphs)
    {
        std::string name = nodeGraph->getName();
        auto currNode = std::make_shared<UiNode>(name, id);
        currNode->setNodeGraph(nodeGraph);
        ++id;
        setUiNodeInfo(currNode, "", "nodegraph");
    }
    for (mx::InputPtr input : inputNodes)
    {
        auto currNode = std::make_shared<UiNode>(input->getName(), id);
        currNode->setInput(input);
        ++id;
        setUiNodeInfo(currNode, input->getType(), input->getCategory());
    }
    // creating edges for nodegraphs
    for (mx::NodeGraphPtr graph : nodeGraphs)
    {
        for (mx::InputPtr input : graph->getActiveInputs())
        {
            int upNum = -1;
            int downNum = -1;
            mx::NodePtr connectedNode = input->getConnectedNode();
            if (connectedNode)
            {
                upNum = findNode(connectedNode->getName(), "node");
                downNum = findNode(graph->getName(), "nodegraph");
                if (upNum > -1)
                {
                    Edge newEdge = Edge(_graphNodes[upNum], _graphNodes[downNum], input);
                    _graphNodes[downNum]->edges.push_back(newEdge);
                    _graphNodes[downNum]->setInputNodeNum(1);
                    _graphNodes[upNum]->setOutputConnection(_graphNodes[downNum]);
                    _currEdge.push_back(newEdge);
                }
            }
        }
    }
    // creating edges for surface and material nodes
    for (mx::NodePtr node : docNodes)
    {
        for (mx::InputPtr input : node->getActiveInputs())
        {

            mx::string nodeGraphName = input->getNodeGraphString();
            mx::NodePtr connectedNode = input->getConnectedNode();
            mx::OutputPtr connectedOutput = input->getConnectedOutput();
            int upNum = -1;
            int downNum = -1;
            if (nodeGraphName != "")
            {

                upNum = findNode(nodeGraphName, "nodegraph");
                downNum = findNode(node->getName(), "node");
            }
            else if (connectedNode)
            {
                upNum = findNode(connectedNode->getName(), "node");
                downNum = findNode(node->getName(), "node");
            }
            else if (connectedOutput)
            {
                upNum = findNode(connectedOutput->getName(), "output");
                downNum = findNode(node->getName(), "node");
            }
            else if (input->getInterfaceName() != "")
            {
                upNum = findNode(input->getInterfaceName(), "input");
                downNum = findNode(node->getName(), "node");
            }
            if (upNum != -1)
            {

                Edge newEdge = Edge(_graphNodes[upNum], _graphNodes[downNum], input);
                if (!edgeExists(newEdge))
                {
                    _graphNodes[downNum]->edges.push_back(newEdge);
                    _graphNodes[downNum]->setInputNodeNum(1);
                    _graphNodes[upNum]->setOutputConnection(_graphNodes[downNum]);
                    _currEdge.push_back(newEdge);
                }
            }
        }
    }
}
// build the UiNode node graph based off of diving into a node graph node
void Graph::buildUiNodeGraph(const mx::NodeGraphPtr& nodeGraphs)
{

    // clear all values so that ids can start with 0 or 1
    _graphNodes.clear();
    _currLinks.clear();
    _currEdge.clear();
    _newLinks.clear();

    int id = 1;

    if (nodeGraphs)
    {
        mx::NodeGraphPtr nodeGraph = nodeGraphs;
        std::vector<mx::ElementPtr> children = nodeGraph->topologicalSort();
        // Write out all nodes.

        mx::NodeDefPtr nodeDef = nodeGraph->getNodeDef();
        mx::NodeDefPtr currNodeDef;

        // create input nodes for shader translation graphs
        if (nodeDef)
        {
            std::vector<mx::InputPtr> inputs = nodeDef->getActiveInputs();

            for (mx::InputPtr input : inputs)
            {
                auto currNode = std::make_shared<UiNode>(input->getName(), id);
                currNode->setInput(input);
                ++id;
                setUiNodeInfo(currNode, input->getType(), input->getCategory());
            }
        }

        // search node graph children to create uiNodes
        for (mx::ElementPtr elem : children)
        {
            mx::NodePtr node = elem->asA<mx::Node>();
            mx::InputPtr input = elem->asA<mx::Input>();
            mx::OutputPtr output = elem->asA<mx::Output>();
            std::string name = elem->getName();
            auto currNode = std::make_shared<UiNode>(name, id);
            if (node)
            {
                currNode->setNode(node);
                ++id;
                setUiNodeInfo(currNode, node->getType(), node->getCategory());
            }
            else if (input)
            {
                currNode->setInput(input);
                ++id;
                setUiNodeInfo(currNode, input->getType(), input->getCategory());
            }
            else if (output)
            {
                ++id;
                currNode->setOutput(output);
                setUiNodeInfo(currNode, output->getType(), output->getCategory());
            }
        }

        // Write out all connections.
        std::set<mx::Edge> processedEdges;
        mx::StringSet processedInterfaces;
        for (mx::OutputPtr output : nodeGraph->getOutputs())
        {
            for (mx::Edge edge : output->traverseGraph())
            {
                if (!processedEdges.count(edge))
                {
                    mx::ElementPtr upstreamElem = edge.getUpstreamElement();
                    mx::ElementPtr downstreamElem = edge.getDownstreamElement();
                    mx::ElementPtr connectingElem = edge.getConnectingElement();

                    mx::NodePtr upstreamNode = upstreamElem->asA<mx::Node>();
                    mx::NodePtr downstreamNode = downstreamElem->asA<mx::Node>();
                    mx::InputPtr upstreamInput = upstreamElem->asA<mx::Input>();
                    mx::InputPtr downstreamInput = downstreamElem->asA<mx::Input>();
                    mx::OutputPtr upstreamOutput = upstreamElem->asA<mx::Output>();
                    mx::OutputPtr downstreamOutput = downstreamElem->asA<mx::Output>();
                    std::string downName = downstreamElem->getName();
                    std::string upName = upstreamElem->getName();
                    std::string upstreamType;
                    std::string downstreamType;
                    if (upstreamNode)
                    {
                        upstreamType = "node";
                    }
                    else if (upstreamInput)
                    {
                        upstreamType = "input";
                    }
                    else if (upstreamOutput)
                    {
                        upstreamType = "output";
                    }
                    if (downstreamNode)
                    {
                        downstreamType = "node";
                    }
                    else if (downstreamInput)
                    {
                        downstreamType = "input";
                    }
                    else if (downstreamOutput)
                    {
                        downstreamType = "output";
                    }
                    int upNode = findNode(upName, upstreamType);
                    int downNode = findNode(downName, downstreamType);
                    if (_graphNodes[downNode]->getOutput() != nullptr)
                    {
                        // creating edges for the output nodes
                        Edge newEdge = Edge(_graphNodes[upNode], _graphNodes[downNode], nullptr);
                        _graphNodes[downNode]->edges.push_back(newEdge);
                        _graphNodes[downNode]->setInputNodeNum(1);
                        _graphNodes[upNode]->setOutputConnection(_graphNodes[downNode]);
                        _currEdge.push_back(newEdge);
                    }
                    else if (connectingElem)
                    {

                        mx::InputPtr connectingInput = connectingElem->asA<mx::Input>();

                        if (connectingInput)
                        {
                            if ((upNode >= 0) && (downNode >= 0))
                            {
                                Edge newEdge = Edge(_graphNodes[upNode], _graphNodes[downNode], connectingInput);
                                _graphNodes[downNode]->edges.push_back(newEdge);
                                _graphNodes[downNode]->setInputNodeNum(1);
                                _graphNodes[upNode]->setOutputConnection(_graphNodes[downNode]);
                                _currEdge.push_back(newEdge);
                            }
                        }
                    }
                    if (upstreamNode)
                    {
                        std::vector<mx::InputPtr> ins = upstreamNode->getActiveInputs();
                        for (mx::InputPtr input : ins)
                        {
                            // connecting input nodes
                            if (input->hasInterfaceName())
                            {
                                std::string interfaceName = input->getInterfaceName();
                                int newUp = findNode(interfaceName, "input");
                                if (newUp >= 0)
                                {
                                    mx::InputPtr inputP = std::make_shared<mx::Input>(downstreamElem, input->getName());
                                    Edge newEdge = Edge(_graphNodes[newUp], _graphNodes[upNode], inputP);

                                    _graphNodes[upNode]->edges.push_back(newEdge);
                                    _graphNodes[upNode]->setInputNodeNum(1);
                                    _graphNodes[newUp]->setOutputConnection(_graphNodes[upNode]);
                                    _currEdge.push_back(newEdge);
                                }
                            }
                        }
                    }

                    processedEdges.insert(edge);
                }
            }
        }

        // second pass to catch all of the connections that arent part of an output
        for (mx::ElementPtr elem : children)
        {
            mx::NodePtr node = elem->asA<mx::Node>();
            mx::InputPtr inputElem = elem->asA<mx::Input>();
            mx::OutputPtr output = elem->asA<mx::Output>();
            std::string name = elem->getName();
            if (node)
            {
                std::vector<mx::InputPtr> inputs = node->getActiveInputs();
                for (mx::InputPtr input : inputs)
                {
                    mx::NodePtr upNode = input->getConnectedNode();
                    if (upNode)
                    {
                        int upNum = findNode(upNode->getName(), "node");
                        int downNode = findNode(node->getName(), "node");
                        if ((upNum >= 0) && (downNode >= 0))
                        {

                            Edge newEdge = Edge(_graphNodes[upNum], _graphNodes[downNode], input);
                            if (!edgeExists(newEdge))
                            {
                                _graphNodes[downNode]->edges.push_back(newEdge);
                                _graphNodes[downNode]->setInputNodeNum(1);
                                _graphNodes[upNum]->setOutputConnection(_graphNodes[downNode]);
                                _currEdge.push_back(newEdge);
                            }
                        }
                    }
                    else if (input->getInterfaceInput())
                    {
                        int upNum = findNode(input->getInterfaceInput()->getName(), "input");
                        int downNode = findNode(node->getName(), "node");
                        if ((upNum >= 0) && (downNode >= 0))
                        {

                            Edge newEdge = Edge(_graphNodes[upNum], _graphNodes[downNode], input);
                            if (!edgeExists(newEdge))
                            {
                                _graphNodes[downNode]->edges.push_back(newEdge);
                                _graphNodes[downNode]->setInputNodeNum(1);
                                _graphNodes[upNum]->setOutputConnection(_graphNodes[downNode]);
                                _currEdge.push_back(newEdge);
                            }
                        }
                    }
                }
            }
            else if (output)
            {
                mx::NodePtr upNode = output->getConnectedNode();
                if (upNode)
                {
                    int upNum = findNode(upNode->getName(), "node");
                    int downNode = findNode(output->getName(), "output");
                    Edge newEdge = Edge(_graphNodes[upNum], _graphNodes[downNode], nullptr);
                    if (!edgeExists(newEdge))
                    {
                        _graphNodes[downNode]->edges.push_back(newEdge);
                        _graphNodes[downNode]->setInputNodeNum(1);
                        _graphNodes[upNum]->setOutputConnection(_graphNodes[downNode]);
                        _currEdge.push_back(newEdge);
                    }
                }
            }
        }
    }
}

// return node position in _graphNodes based off node name and type to account for input/output UiNodes with same names as mx Nodes
int Graph::findNode(std::string name, std::string type)
{
    int count = 0;
    for (size_t i = 0; i < _graphNodes.size(); i++)
    {
        if (_graphNodes[i]->getName() == name)
        {
            if (type == "node" && _graphNodes[i]->getNode() != nullptr)
            {
                return count;
            }
            else if (type == "input" && _graphNodes[i]->getInput() != nullptr)
            {
                return count;
            }
            else if (type == "output" && _graphNodes[i]->getOutput() != nullptr)
            {
                return count;
            }
            else if (type == "nodegraph" && _graphNodes[i]->getNodeGraph() != nullptr)
            {
                return count;
            }
        }
        count++;
    }
    return -1;
}

void Graph::positionPasteBin(ImVec2 pos)
{
    ImVec2 totalPos = ImVec2(0, 0);
    ImVec2 avgPos = ImVec2(0, 0);

    // get average position of original nodes
    for (auto pasteNode : _copiedNodes)
    {
        ImVec2 origPos = ed::GetNodePosition(pasteNode.first->getId());
        totalPos.x += origPos.x;
        totalPos.y += origPos.y;
    }
    avgPos.x = totalPos.x / (int) _copiedNodes.size();
    avgPos.y = totalPos.y / (int) _copiedNodes.size();

    // get offset from clciked position
    ImVec2 offset = ImVec2(0, 0);
    offset.x = pos.x - avgPos.x;
    offset.y = pos.y - avgPos.y;
    for (auto pasteNode : _copiedNodes)
    {
        ImVec2 newPos = ImVec2(0, 0);
        newPos.x = ed::GetNodePosition(pasteNode.first->getId()).x + offset.x;
        newPos.y = ed::GetNodePosition(pasteNode.first->getId()).y + offset.y;
        ed::SetNodePosition(pasteNode.second->getId(), newPos);
    }
}
void Graph::createEdge(UiNodePtr upNode, UiNodePtr downNode, mx::InputPtr connectingInput)
{
    if (downNode->getOutput())
    {
        // creating edges for the output nodes
        Edge newEdge = Edge(upNode, downNode, nullptr);
        downNode->edges.push_back(newEdge);
        downNode->setInputNodeNum(1);
        upNode->setOutputConnection(downNode);
        _currEdge.push_back(newEdge);
    }
    else if (connectingInput)
    {

        Edge newEdge = Edge(upNode, downNode, connectingInput);
        downNode->edges.push_back(newEdge);
        downNode->setInputNodeNum(1);
        upNode->setOutputConnection(downNode);
        _currEdge.push_back(newEdge);
    }
}

void Graph::copyUiNode(UiNodePtr node)
{
    UiNodePtr copyNode = std::make_shared<UiNode>(int(_graphTotalSize + 1));
    ++_graphTotalSize;
    if (node->getMxElement())
    {
        std::string newName = node->getMxElement()->getParent()->createValidChildName(node->getName());
        if (node->getNode())
        {
            mx::NodePtr mxNode;
            if (_isNodeGraph)
            {
                mxNode = _currNodeGraph->addNodeInstance(node->getNode()->getNodeDef());
            }
            else
            {
                mxNode = _graphDoc->addNodeInstance(node->getNode()->getNodeDef());
            }
            mxNode->copyContentFrom(node->getNode());
            mxNode->setName(newName);
            copyNode->setNode(mxNode);
        }
        else if (node->getInput())
        {
            mx::InputPtr mxInput;
            if (_isNodeGraph)
            {
                mxInput = _currNodeGraph->addInput(newName);
            }
            else
            {
                mxInput = _graphDoc->addInput(newName);
            }
            mxInput->copyContentFrom(node->getInput());
            copyNode->setInput(mxInput);
        }
        else if (node->getOutput())
        {
            mx::OutputPtr mxOutput;
            if (_isNodeGraph)
            {
                mxOutput = _currNodeGraph->addOutput(newName);
            }
            else
            {
                mxOutput = _graphDoc->addOutput(newName);
            }
            mxOutput->copyContentFrom(node->getOutput());
            mxOutput->setName(newName);
            copyNode->setOutput(mxOutput);
        }
        copyNode->getMxElement()->setName(newName);
        copyNode->setName(newName);
    }
    else if (node->getNodeGraph())
    {
        _graphDoc->addNodeGraph();
        std::string nodeGraphName = _graphDoc->getNodeGraphs().back()->getName();
        copyNode->setNodeGraph(_graphDoc->getNodeGraphs().back());
        _currNodeGraph = _graphDoc->getNodeGraphs().back();
        copyNode->setName(nodeGraphName);
        copyNodeGraph(node, copyNode);
    }
    copyNode->setCategory(node->getCategory());
    copyNode->setType(node->getType());
    _copiedNodes[node] = copyNode;
    _graphNodes.push_back(copyNode);
}
void Graph::copyNodeGraph(UiNodePtr origGraph, UiNodePtr copyGraph)
{
    copyGraph->getNodeGraph()->copyContentFrom(origGraph->getNodeGraph());
    std::vector<mx::InputPtr> inputs = copyGraph->getNodeGraph()->getActiveInputs();
    for (mx::InputPtr input : inputs)
    {
        std::string newName = _graphDoc->createValidChildName(input->getName());
        input->setName(newName);
    }
}
void Graph::copyInputs()
{
    for (std::map<UiNodePtr, UiNodePtr>::iterator iter = _copiedNodes.begin(); iter != _copiedNodes.end(); ++iter)
    {
        int count = 0;
        UiNodePtr origNode = iter->first;
        UiNodePtr copyNode = iter->second;
        for (Pin pin : origNode->inputPins)
        {

            if (origNode->getConnectedNode(pin._name) && !_ctrlClick)
            {
                // if original node is connected check if connect node is in copied nodes
                if (_copiedNodes.find(origNode->getConnectedNode(pin._name)) != _copiedNodes.end())
                {
                    // set copy node connected to the value at this key
                    // create an edge
                    createEdge(_copiedNodes[origNode->getConnectedNode(pin._name)], copyNode, copyNode->inputPins[count]._input);
                    UiNodePtr upNode = _copiedNodes[origNode->getConnectedNode(pin._name)];
                    if (copyNode->getNode() || copyNode->getNodeGraph())
                    {

                        mx::InputPtr connectingInput = nullptr;
                        copyNode->inputPins[count]._input->copyContentFrom(pin._input);
                        // update value to be empty
                        if (copyNode->getNode() && copyNode->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                        {
                            if (upNode->getOutput())
                            {
                                copyNode->inputPins[count]._input->setConnectedOutput(upNode->getOutput());
                            }
                            else if (upNode->getInput())
                            {

                                copyNode->inputPins[count]._input->setInterfaceName(upNode->getName());
                            }
                            else
                            {
                                // node graph
                                if (upNode->getNodeGraph())
                                {
                                    ed::PinId outputId = getOutputPin(copyNode, upNode, copyNode->inputPins[count]);
                                    for (Pin outPin : upNode->outputPins)
                                    {
                                        if (outPin._pinId == outputId)
                                        {
                                            mx::OutputPtr outputs = upNode->getNodeGraph()->getOutput(outPin._name);
                                            copyNode->inputPins[count]._input->setConnectedOutput(outputs);
                                        }
                                    }
                                }
                                else
                                {
                                    copyNode->inputPins[count]._input->setConnectedNode(upNode->getNode());
                                }
                            }
                        }
                        else
                        {
                            if (upNode->getInput())
                            {

                                copyNode->inputPins[count]._input->setInterfaceName(upNode->getName());
                            }
                            else
                            {
                                copyNode->inputPins[count]._input->setConnectedNode(upNode->getNode());
                            }
                        }

                        copyNode->inputPins[count].setConnected(true);
                        copyNode->inputPins[count]._input->removeAttribute(mx::ValueElement::VALUE_ATTRIBUTE);
                    }
                    else if (copyNode->getOutput() != nullptr)
                    {
                        mx::InputPtr connectingInput = nullptr;
                        copyNode->getOutput()->setConnectedNode(upNode->getNode());
                    }

                    // update input node num and output connections
                    copyNode->setInputNodeNum(1);
                    upNode->setOutputConnection(copyNode);
                }
                else if (pin._input)
                {
                    if (pin._input->getInterfaceInput())
                    {
                        copyNode->inputPins[count]._input->removeAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE);
                    }
                    copyNode->inputPins[count].setConnected(false);
                    setDefaults(copyNode->inputPins[count]._input);
                    copyNode->inputPins[count]._input->setConnectedNode(nullptr);
                    copyNode->inputPins[count]._input->setConnectedOutput(nullptr);
                }
            }
            count++;
        }
    }
}
void Graph::cutNodes()
{
    for (std::map<UiNodePtr, UiNodePtr>::iterator iter = _copiedNodes.begin(); iter != _copiedNodes.end(); ++iter)
    {
        // for each node check output nodes and upstream nodes
        for (Pin pin : iter->first->inputPins)
        {
            if (pin.getConnected())
            {
                if (iter->first->getConnectedNode(pin._name))
                {
                    ed::PinId outId = getOutputPin(iter->first, iter->first->getConnectedNode(pin._name), pin);
                    deleteLinkInfo(int(outId.Get()), (int) pin._pinId.Get());
                }
            }
        }
        for (UiNodePtr downNode : iter->first->getOutputConnections())
        {
            for (Pin pin : downNode->inputPins)
            {
                if (pin.getConnected())
                {
                    if (downNode->getConnectedNode(pin._name))
                    {
                        if (downNode->getConnectedNode(pin._name)->getName() == iter->first->getName())
                        {
                            ed::PinId outId = getOutputPin(downNode->getConnectedNode(pin._name), iter->first, pin);
                            deleteLinkInfo(int(outId.Get()), (int) pin._pinId.Get());
                        }
                    }
                }
            }
        }
    }
}
// add node to graphNodes based off of node def information
void Graph::addNode(std::string category, std::string name, std::string type)
{
    mx::NodePtr node = nullptr;
    std::vector<mx::NodeDefPtr> matchingNodeDefs;
    // create document or node graph is there is not already one
    if (category == "output")
    {
        std::string outName = "";

        mx::OutputPtr newOut;
        // add output as child of correct parent and create valid name
        if (_isNodeGraph)
        {
            outName = _currNodeGraph->createValidChildName(name);
            newOut = _currNodeGraph->addOutput(outName, type);
        }
        else
        {
            outName = _graphDoc->createValidChildName(name);
            newOut = _graphDoc->addOutput(outName, type);
        }
        auto outputNode = std::make_shared<UiNode>(outName, int(_graphTotalSize + 1));
        outputNode->setOutput(newOut);
        setUiNodeInfo(outputNode, type, category);
        return;
    }
    if (category == "input")
    {
        std::string inName = "";
        mx::InputPtr newIn = nullptr;
        // add input as child of correct parent and create valid name
        if (_isNodeGraph)
        {
            inName = _currNodeGraph->createValidChildName(name);
            newIn = _currNodeGraph->addInput(inName, type);
        }
        else
        {
            inName = _graphDoc->createValidChildName(name);
            newIn = _graphDoc->addInput(inName, type);
        }
        auto inputNode = std::make_shared<UiNode>(inName, int(_graphTotalSize + 1));
        setDefaults(newIn);
        inputNode->setInput(newIn);
        setUiNodeInfo(inputNode, type, category);
        return;
    }
    else if (category == "group")
    {
        auto groupNode = std::make_shared<UiNode>(name, int(_graphTotalSize + 1));
        // set message of group uinode in order to identify it as such
        groupNode->setMessage("Comment");
        setUiNodeInfo(groupNode, type, "group");
        // create ui portions of group node
        buildGroupNode(_graphNodes.back());
        return;
    }
    else if (category == "nodegraph")
    {
        // create new mx::NodeGraph and set as current node graph
        _graphDoc->addNodeGraph();
        std::string nodeGraphName = _graphDoc->getNodeGraphs().back()->getName();
        auto nodeGraphNode = std::make_shared<UiNode>(nodeGraphName, int(_graphTotalSize + 1));
        // set mx::Nodegraph as node graph for uiNode
        nodeGraphNode->setNodeGraph(_graphDoc->getNodeGraphs().back());
        _currNodeGraph = _graphDoc->getNodeGraphs().back();
        setUiNodeInfo(nodeGraphNode, type, "nodegraph");
        return;
    }
    // if shader or material we want to add to the document instead of the node graph
    else if (type == mx::SURFACE_SHADER_TYPE_STRING)
    {
        matchingNodeDefs = _graphDoc->getMatchingNodeDefs(category);
        for (mx::NodeDefPtr nodedef : matchingNodeDefs)
        {
            std::string nodedefName = nodedef->getName();
            std::string sub = nodedefName.substr(3, nodedefName.length());
            if (sub == name)
            {
                node = _graphDoc->addNodeInstance(nodedef);
                node->setName(_graphDoc->createValidChildName(name));
                break;
            }
        }
    }
    else if (type == mx::MATERIAL_TYPE_STRING)
    {
        matchingNodeDefs = _graphDoc->getMatchingNodeDefs(category);
        for (mx::NodeDefPtr nodedef : matchingNodeDefs)
        {
            std::string nodedefName = nodedef->getName();
            std::string sub = nodedefName.substr(3, nodedefName.length());
            if (sub == name)
            {
                node = _graphDoc->addNodeInstance(nodedef);
                node->setName(_graphDoc->createValidChildName(name));
                break;
            }
        }
    }
    else
    {
        matchingNodeDefs = _graphDoc->getMatchingNodeDefs(category);
        for (mx::NodeDefPtr nodedef : matchingNodeDefs)
        {
            // use substring of name in order to remove ND_
            std::string nodedefName = nodedef->getName();
            std::string sub = nodedefName.substr(3, nodedefName.length());
            if (sub == name)
            {
                if (_isNodeGraph)
                {
                    node = _currNodeGraph->addNodeInstance(nodedef);
                    node->setName(_currNodeGraph->createValidChildName(name));
                }
                else
                {
                    node = _graphDoc->addNodeInstance(nodedef);
                    node->setName(_graphDoc->createValidChildName(name));
                }
            }
        }
    }
    if (node)
    {
        int num = 0;
        int countDef = 0;
        for (size_t i = 0; i < matchingNodeDefs.size(); i++)
        {
            // use substring of name in order to remove ND_
            std::string nodedefName = matchingNodeDefs[i]->getName();
            std::string sub = nodedefName.substr(3, nodedefName.length());
            if (sub == name)
            {
                num = countDef;
            }
            countDef++;
        }
        std::vector<mx::InputPtr> defInputs = matchingNodeDefs[num]->getActiveInputs();
        // adding inputs to node first so that we can later set their values
        for (mx::InputPtr input : defInputs)
        {
            mx::ValuePtr newVal = input->getValue();
            node->addInput(input->getName(), input->getType());
        }
        auto newNode = std::make_shared<UiNode>(node->getName(), int(_graphTotalSize + 1));
        newNode->setCategory(category);
        newNode->setType(type);
        node->setType(type);
        ++_graphTotalSize;

        newNode->setNode(node);
        std::vector<mx::InputPtr> nodeInputs = newNode->getNode()->getActiveInputs();

        // set value to node inputs
        for (mx::InputPtr inputNew : nodeInputs)
        {
            mx::ValuePtr val = matchingNodeDefs[num]->getActiveInput(inputNew->getName())->getValue();
            bool geomProp = matchingNodeDefs[num]->getActiveInput(inputNew->getName())->hasDefaultGeomPropString();
            inputNew->setConnectedNode(nullptr);
            if (val)
            {
                inputNew->setValue(val->getValueString().c_str(), inputNew->getType());
                inputNew->setValueString(val->getValueString());
            }
            else if (!geomProp)
            {
                // only set the defaults if not a geom prop as that should not be changed in the property editor
                setDefaults(inputNew);
            }
        }

        _graphNodes.push_back(std::move(newNode));
        std::vector<mx::NodePtr> nodes = _graphDoc->getNodes();
        updateMaterials(nullptr, nullptr, true);
    }
}
// return node pos
int Graph::getNodeId(ed::PinId pinId)
{
    for (Pin pin : _currPins)
    {
        if (pin._pinId == pinId)
        {
            return findNode(pin._pinNode->getId());
        }
    }
    return -1;
}

// return pin based off of Pin id
Pin Graph::getPin(ed::PinId pinId)
{
    for (Pin pin : _currPins)
    {
        if (pin._pinId == pinId)
        {
            return pin;
        }
    }
    Pin nullPin = Pin(-10000, "nullPin", "null", nullptr, ax::NodeEditor::PinKind::Output, nullptr, nullptr);
    return nullPin;
}

void Graph::DrawPinIcon(std::string type, bool connected, int alpha)
{
    ax::Drawing::IconType iconType = ax::Drawing::IconType::Circle;
    ImColor color = getPinColor(type);
    color.Value.w = alpha / 255.0f;

    ax::Widgets::Icon(ImVec2(24, 24), iconType, connected, color, ImColor(32, 32, 32, alpha));
}

void Graph::buildGroupNode(UiNodePtr node)
{
    const float commentAlpha = 0.75f;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
    ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
    ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));

    ed::BeginNode(node->getId());
    ImGui::PushID(node->getId());

    std::string original = node->getMessage();
    std::string temp = original;
    ImVec2 messageSize = ImGui::CalcTextSize(temp.c_str());
    ImGui::PushItemWidth(messageSize.x + 15);
    ImGui::InputText("##edit", &temp);
    node->setMessage(temp);
    ImGui::PopItemWidth();
    ed::Group(ImVec2(300, 200));
    ImGui::PopID();
    ed::EndNode();
    ed::PopStyleColor(2);
    ImGui::PopStyleVar();
    if (ed::BeginGroupHint(node->getId()))
    {
        // auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
        auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);
        auto min = ed::GetGroupMin();

        ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
        ImGui::BeginGroup();
        ImGui::PushID(node->getId() + 1000);
        std::string tempName = node->getName();
        ImVec2 nameSize = ImGui::CalcTextSize(temp.c_str());
        ImGui::PushItemWidth(nameSize.x);
        ImGui::InputText("##edit", &tempName);
        node->setName(tempName);
        ImGui::PopID();
        ImGui::EndGroup();

        auto drawList = ed::GetHintBackgroundDrawList();

        auto hintBounds = ImGui_GetItemRect();
        auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

        drawList->AddRectFilled(
            hintFrameBounds.GetTL(),
            hintFrameBounds.GetBR(),
            IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

        drawList->AddRect(
            hintFrameBounds.GetTL(),
            hintFrameBounds.GetBR(),
            IM_COL32(0, 255, 255, 128 * bgAlpha / 255), 4.0f);
    }
    ed::EndGroupHint();
}

mx::InputPtr Graph::findInput(mx::InputPtr nodeInput, std::string name)
{
    if (_isNodeGraph)
    {
        for (UiNodePtr node : _graphNodes)
        {
            if (node->getNode())
            {
                for (mx::InputPtr input : node->getNode()->getActiveInputs())
                {
                    if (input->getInterfaceInput())
                    {

                        if (input->getInterfaceInput() == nodeInput)
                        {
                            return input;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (_currUiNode->getNodeGraph())
        {
            for (mx::NodePtr node : _currUiNode->getNodeGraph()->getNodes())
            {
                for (mx::InputPtr input : node->getActiveInputs())
                {
                    if (input->getInterfaceInput())
                    {

                        if (input->getInterfaceName() == name)
                        {
                            return input;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}
//  from node library blueprints-example.cpp
static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

void Graph::outputPin(UiNodePtr node, int attrId, std::string outputPin, std::string name)
{
    // create output pin
    float nodeWidth = 20 + ImGui::CalcTextSize(node->getName().c_str()).x;
    if (nodeWidth < 75)
    {
        nodeWidth = 75;
    }
    const float labelWidth = ImGui::CalcTextSize("output").x;
    ImGui::Indent(nodeWidth - labelWidth);
    // create node editor pin
    Pin outPin = Pin(attrId, &*name.begin(), outputPin, node, ax::NodeEditor::PinKind::Output, nullptr, nullptr);
    ed::BeginPin(attrId, ed::PinKind::Output);
    ImGui::Text("%s", name.c_str());
    ImGui::SameLine();
    if (!_pinFilterType.empty())
    {
        if (_pinFilterType == outputPin)
        {
            DrawPinIcon(outputPin, true, DEFAULT_ALPHA);
        }
        else
        {
            DrawPinIcon(outputPin, true, FILTER_ALPHA);
        }
    }
    else
    {
        DrawPinIcon(outputPin, true, DEFAULT_ALPHA);
    }

    ed::EndPin();
    node->outputPins.push_back(outPin);
    _currPins.push_back(outPin);
    ImGui::Unindent(nodeWidth - labelWidth);
    ++attrId;
}

void Graph::CreateInputPin(int attrId, mx::InputPtr input)
{
    ed::BeginPin(attrId, ed::PinKind::Input);
    ImGui::PushID(int(attrId));
    if (!_pinFilterType.empty())
    {
        if (_pinFilterType == input->getType())
        {
            DrawPinIcon(input->getType(), true, DEFAULT_ALPHA);
        }
        else
        {
            DrawPinIcon(input->getType(), true, FILTER_ALPHA);
        }
    }
    else
    {
        DrawPinIcon(input->getType(), true, DEFAULT_ALPHA);
    }

    ImGui::SameLine();
    ImGui::TextUnformatted(input->getName().c_str());
    ed::EndPin();
    ImGui::PopID();
}

std::vector<int> Graph::CreateNodes(bool nodegraph)
{
    _currPins.clear();
    std::vector<int> outputNum;

    int attrId = _graphTotalSize + 1;
    for (UiNodePtr node : _graphNodes)
    {
        if (node->getCategory() == "group")
        {
            buildGroupNode(node);
        }
        else
        {
            // color for output pin
            std::string outputType;
            node->inputPins.clear();
            node->outputPins.clear();

            if (node->getNode() != nullptr)
            {

                ed::BeginNode(node->getId());
                ImGui::PushID(node->getId());
                ImGui::SetWindowFontScale(1.2f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, -8.0),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(55, 55, 55, 255)), 12.f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, 3),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(55, 55, 55, 255)), 0.f);
                ImGui::Text("%s", node->getName().c_str());
                ImGui::SetWindowFontScale(1);

                std::vector<mx::InputPtr> inputs = node->getNode()->getActiveInputs();

                outputType = node->getNode()->getType();
                outputPin(node, int(attrId), outputType, "output");
                ++attrId;
                for (size_t i = 0; i < inputs.size(); i++)
                {
                    std::string inName = inputs[i]->getName();
                    mx::InputPtr newIn = inputs[i]->getInterfaceInput();
                    Pin inPin = Pin(int(attrId), &*inName.begin(), inputs[i]->getType(), node, ax::NodeEditor::PinKind::Input, inputs[i], nullptr);
                    if (node->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                    {

                        mx::OutputPtr out = inputs[i]->getConnectedOutput();
                        if (out)
                        {
                            inPin.setConnected(true);
                        }
                    }
                    else if (inputs[i]->getConnectedNode() || inputs[i]->getInterfaceInput())
                    {
                        inPin.setConnected(true);
                    }
                    if (node->_showAllInputs)
                    {
                        CreateInputPin(attrId, inputs[i]);
                    }
                    else if (inPin.getConnected())
                    {
                        CreateInputPin(attrId, inputs[i]);
                    }

                    node->inputPins.push_back(inPin);
                    _currPins.push_back(inPin);

                    ++attrId;
                }
                // set color of output pin

                if (node->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                {
                    if (node->getOutputConnections().size() > 0)
                    {
                        for (UiNodePtr outputCon : node->getOutputConnections())
                        {
                            outputNum.push_back(findNode(outputCon->getId()));
                        }
                    }
                }
            }
            else if (node->getInput() != nullptr)
            {
                ed::BeginNode(node->getId());
                ImGui::PushID(node->getId());
                ImGui::SetWindowFontScale(1.2f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0f, -8.0f),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(85, 85, 85, 255)), 12.f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0f, 3.f),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(85, 85, 85, 255)), 0.f);
                ImGui::Text("%s", node->getName().c_str());
                ImGui::SetWindowFontScale(1);

                outputType = node->getInput()->getType();
                outputPin(node, int(attrId), outputType, "output");
                ++attrId;
                Pin inPin = Pin(int(attrId), &*("Value"), node->getInput()->getType(), node, ax::NodeEditor::PinKind::Input, node->getInput(), nullptr);
                if (node->getInput()->getConnectedNode())
                {
                    inPin.setConnected(true);
                }

                ed::BeginPin(attrId, ed::PinKind::Input);
                if (!_pinFilterType.empty())
                {
                    if (_pinFilterType == node->getInput()->getType())
                    {
                        DrawPinIcon(node->getInput()->getType(), true, DEFAULT_ALPHA);
                    }
                    else
                    {
                        DrawPinIcon(node->getInput()->getType(), true, FILTER_ALPHA);
                    }
                }
                else
                {
                    DrawPinIcon(node->getInput()->getType(), true, DEFAULT_ALPHA);
                }

                ImGui::SameLine();
                ImGui::TextUnformatted("value");
                ed::EndPin();

                node->inputPins.push_back(inPin);
                _currPins.push_back(inPin);

                ++attrId;
            }
            else if (node->getOutput() != nullptr)
            {

                ed::BeginNode(node->getId());
                ImGui::PushID(node->getId());
                ImGui::SetWindowFontScale(1.2f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, -8.0),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(35, 35, 35, 255)), 12.f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, 3),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(35, 35, 35, 255)), 0);
                ImGui::Text("%s", node->getName().c_str());
                ImGui::SetWindowFontScale(1.0);

                outputType = node->getOutput()->getType();
                outputPin(node, int(attrId), outputType, "output");
                ++attrId;

                Pin inPin = Pin(int(attrId), &*("input"), node->getOutput()->getType(), node, ax::NodeEditor::PinKind::Input, nullptr, node->getOutput());
                ed::BeginPin(attrId, ed::PinKind::Input);
                if (!_pinFilterType.empty())
                {
                    if (_pinFilterType == node->getOutput()->getType())
                    {
                        DrawPinIcon(node->getOutput()->getType(), true, DEFAULT_ALPHA);
                    }
                    else
                    {
                        DrawPinIcon(node->getOutput()->getType(), true, FILTER_ALPHA);
                    }
                }
                else
                {
                    DrawPinIcon(node->getOutput()->getType(), true, DEFAULT_ALPHA);
                }
                ImGui::SameLine();
                ImGui::TextUnformatted("input");

                ed::EndPin();

                node->inputPins.push_back(inPin);
                _currPins.push_back(inPin);

                ++attrId;

                if (nodegraph)
                {
                    outputNum.push_back(findNode(node->getId()));
                }
            }
            else if (node->getNodeGraph() != nullptr)
            {
                ed::BeginNode(node->getId());
                ImGui::PushID(node->getId());
                ImGui::SetWindowFontScale(1.2f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, -8.0),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(35, 35, 35, 255)), 12.f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetCursorScreenPos() + ImVec2(-7.0, 3),
                    ImGui::GetCursorScreenPos() + ImVec2(ed::GetNodeSize(node->getId()).x - 9.f, ImGui::GetTextLineHeight() + 2.f),
                    ImColor(ImColor(35, 35, 35, 255)), 0);
                ImGui::Text("%s", node->getName().c_str());
                ImGui::SetWindowFontScale(1.0);
                std::vector<mx::InputPtr> inputs = node->getNodeGraph()->getActiveInputs();
                for (mx::InputPtr input : node->getNodeGraph()->getActiveInputs())
                {

                    std::string inName = input->getName();
                    Pin inPin = Pin(int(attrId), &*inName.begin(), input->getType(), node, ax::NodeEditor::PinKind::Input, input, nullptr);

                    if (input->getConnectedNode() != nullptr)
                    {
                        inPin.setConnected(true);
                    }
                    if (node->_showAllInputs)
                    {
                        CreateInputPin(attrId, input);
                    }
                    else if (inPin.getConnected())
                    {
                        CreateInputPin(attrId, input);
                    }
                    node->inputPins.push_back(inPin);
                    _currPins.push_back(inPin);
                    ++attrId;
                }
                for (mx::OutputPtr output : node->getNodeGraph()->getOutputs())
                {
                    outputType = output->getType();
                    std::string name = output->getName();
                    outputPin(node, int(attrId), outputType, name);
                    ++attrId;
                }
            }
            ImGui::PopID();
            ed::EndNode();
        }
    }
    ImGui::SetWindowFontScale(1.0);
    return outputNum;
}
void Graph::setDefaults(mx::InputPtr input)
{
    if (input->getType() == "float")
    {

        input->setValue(0.f, "float");
    }
    else if (input->getType() == "integer")
    {

        input->setValue(0, "integer");
    }
    else if (input->getType() == "color3")
    {

        input->setValue(mx::Color3(0.f, 0.f, 0.f), "color3");
    }
    else if (input->getType() == "color4")
    {
        input->setValue(mx::Color4(0.f, 0.f, 0.f, 1.f), "color4");
    }
    else if (input->getType() == "vector2")
    {
        input->setValue(mx::Vector2(0.f, 0.f), "vector2");
    }
    else if (input->getType() == "vector3")
    {
        input->setValue(mx::Vector3(0.f, 0.f, 0.f), "vector3");
    }
    else if (input->getType() == "vector4")
    {

        input->setValue(mx::Vector4(0.f, 0.f, 0.f, 0.f), "vector4");
    }
    else if (input->getType() == "string")
    {
        input->setValue("", "string");
    }
    else if (input->getType() == "filename")
    {

        input->setValue("", "filename");
    }
    else if (input->getType() == "boolean")
    {

        input->setValue(false, "boolean");
    }
}
// add link to nodegraph and set up connections between UiNodes and MaterialX Nodes to update shader
void Graph::AddLink(ed::PinId inputPinId, ed::PinId outputPinId)
{
    int end_attr = int(outputPinId.Get());
    int start_attr = int(inputPinId.Get());
    Pin inputPin = getPin(outputPinId);
    Pin outputPin = getPin(inputPinId);
    if (inputPinId && outputPinId && (outputPin._type == inputPin._type)) // both are valid, let's accept link
    {
        if (inputPin._connected == false)
        {
            int upNode = getNodeId(inputPinId);
            int downNode = getNodeId(outputPinId);

            // make sure there is an implementation for node
            const mx::ShaderGenerator& shadergen = _renderer->getGenContext().getShaderGenerator();

            // Find the implementation for this nodedef if not an input or output uinode
            if (_graphNodes[downNode]->getInput() && _isNodeGraph)
            {
                ed::RejectNewItem();
                showLabel("Cannot connect to inputs inside of graph", ImColor(50, 50, 50, 255));
                return;
            }
            else if (_graphNodes[upNode]->getNode())
            {
                mx::ShaderNodeImplPtr impl = shadergen.getImplementation(*_graphNodes[upNode]->getNode()->getNodeDef(), _renderer->getGenContext());
                if (!impl)
                {
                    ed::RejectNewItem();
                    showLabel("Invalid Connection: Node does not have an implementation", ImColor(50, 50, 50, 255));
                    return;
                }
            }

            if (ed::AcceptNewItem())
            {
                // Since we accepted new link, lets add one to our list of links.
                Link link;
                link._startAttr = start_attr;
                link._endAttr = end_attr;
                _currLinks.push_back(link);
                if (_graphNodes[downNode]->getNode() || _graphNodes[downNode]->getNodeGraph())
                {

                    mx::InputPtr connectingInput = nullptr;
                    for (Pin pin : _graphNodes[downNode]->inputPins)
                    {
                        if (pin._pinId == outputPinId)
                        {
                            // update value to be empty
                            if (_graphNodes[downNode]->getNode() && _graphNodes[downNode]->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                            {
                                if (_graphNodes[upNode]->getOutput() != nullptr)
                                {
                                    pin._input->setConnectedOutput(_graphNodes[upNode]->getOutput());
                                }
                                else
                                {
                                    // node graph
                                    if (_graphNodes[upNode]->getNodeGraph() != nullptr)
                                    {
                                        for (Pin outPin : _graphNodes[upNode]->outputPins)
                                        {
                                            // set pin connection to correct output
                                            if (outPin._pinId == inputPinId)
                                            {
                                                mx::OutputPtr outputs = _graphNodes[upNode]->getNodeGraph()->getOutput(outPin._name);
                                                pin._input->setConnectedOutput(outputs);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        pin._input->setConnectedNode(_graphNodes[upNode]->getNode());
                                    }
                                }
                            }
                            else
                            {
                                if (_graphNodes[upNode]->getInput())
                                {

                                    pin._input->setInterfaceName(_graphNodes[upNode]->getName());
                                }
                                else
                                {
                                    pin._input->setConnectedNode(_graphNodes[upNode]->getNode());
                                }
                            }

                            pin.setConnected(true);
                            pin._input->removeAttribute(mx::ValueElement::VALUE_ATTRIBUTE);
                            connectingInput = pin._input;
                            break;
                        }
                    }
                    ////create new edge and set edge information
                    createEdge(_graphNodes[upNode], _graphNodes[downNode], connectingInput);
                    updateMaterials(connectingInput, nullptr, true);
                }
                else if (_graphNodes[downNode]->getOutput() != nullptr)
                {
                    mx::InputPtr connectingInput = nullptr;
                    _graphNodes[downNode]->getOutput()->setConnectedNode(_graphNodes[upNode]->getNode());

                    ////create new edge and set edge information
                    createEdge(_graphNodes[upNode], _graphNodes[downNode], connectingInput);
                    updateMaterials(connectingInput, nullptr, true);
                }
                else
                {
                    // create new edge and set edge info
                    Edge newEdge = Edge(_graphNodes[upNode], _graphNodes[downNode], nullptr);
                    _graphNodes[downNode]->edges.push_back(newEdge);
                    _currEdge.push_back(newEdge);
                    // update input node num and output connections
                    _graphNodes[downNode]->setInputNodeNum(1);
                    _graphNodes[upNode]->setOutputConnection(_graphNodes[downNode]);
                }
            }
        }
        else
        {
            ed::RejectNewItem();
        }
    }
    else
    {
        ed::RejectNewItem();
        showLabel("Invalid Connection due to Mismatch Types", ImColor(50, 50, 50, 255));
    }
}

void Graph::deleteLinkInfo(int startAttr, int endAttr)
{
    int upNode = getNodeId(startAttr);
    int downNode = getNodeId(endAttr);
    // ok so downNode edge remov
    int num = _graphNodes[downNode]->getEdgeIndex(_graphNodes[upNode]->getId());
    if (num != -1)
    {
        if (_graphNodes[downNode]->edges.size() == 1)
        {
            _graphNodes[downNode]->edges.erase(_graphNodes[downNode]->edges.begin() + 0);
        }
        else if (_graphNodes[downNode]->edges.size() > 1)
        {
            _graphNodes[downNode]->edges.erase(_graphNodes[downNode]->edges.begin() + num);
        }
    }

    // downNode set node num -1
    _graphNodes[downNode]->setInputNodeNum(-1);
    // upNode remove outputconnection
    _graphNodes[upNode]->removeOutputConnection(_graphNodes[downNode]->getName());
    // change input so that is default val
    // change informtion of actual mx::Node
    if (_graphNodes[downNode]->getNode())
    {
        mx::NodeDefPtr nodeDef = _graphNodes[downNode]->getNode()->getNodeDef(_graphNodes[downNode]->getNode()->getName());

        for (Pin pin : _graphNodes[downNode]->inputPins)
        {

            if ((int) pin._pinId.Get() == endAttr)
            {

                mx::ValuePtr val = nodeDef->getActiveInput(pin._input->getName())->getValue();
                if (_graphNodes[downNode]->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING)
                {
                    pin._input->setConnectedOutput(nullptr);
                }
                else
                {
                    pin._input->setConnectedNode(nullptr);
                }
                if (_graphNodes[upNode]->getInput())
                {
                    // remove interface value in order to set the default of the input
                    pin._input->removeAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE);
                    setDefaults(pin._input);
                    setDefaults(_graphNodes[upNode]->getInput());
                }

                pin.setConnected(false);
                // if a value exists update the infput with it
                if (val)
                {
                    std::string valString = val->getValueString();
                    pin._input->setValueString(val->getValueString());
                    updateMaterials(pin._input, pin._input->getValue(), true);
                }
            }
        }
    }
    else if (_graphNodes[downNode]->getNodeGraph())
    {
        // set default values for nodegraph node pins ie nodegraph inputs
        mx::NodeDefPtr nodeDef = _graphNodes[downNode]->getNodeGraph()->getNodeDef();
        for (Pin pin : _graphNodes[downNode]->inputPins)
        {

            if ((int) pin._pinId.Get() == endAttr)
            {

                if (_graphNodes[upNode]->getInput())
                {
                    _graphNodes[downNode]->getNodeGraph()->getInput(pin._name)->removeAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE);
                    setDefaults(_graphNodes[upNode]->getInput());
                }
                pin._input->setConnectedNode(nullptr);
                pin.setConnected(false);
                setDefaults(pin._input);
                updateMaterials(pin._input, pin._input->getValue(), true);
            }
        }
    }
}
// delete link from currLink vector and remove any connections in UiNode or MaterialX Nodes to update shader
void Graph::deleteLink(ed::LinkId deletedLinkId)
{
    // If you agree that link can be deleted, accept deletion.
    if (ed::AcceptDeletedItem())
    {
        int link_id = int(deletedLinkId.Get());
        // Then remove link from your data.
        int pos = findLinkPosition(link_id);

        // link start -1 equals node num
        Link currLink = _currLinks[pos];
        deleteLinkInfo(currLink._startAttr, currLink._endAttr);
        _currLinks.erase(_currLinks.begin() + pos);
    }
}

void Graph::deleteNode(UiNodePtr node)
{

    // delete link
    for (Pin inputPins : node->inputPins)
    {
        UiNodePtr upNode = node->getConnectedNode(inputPins._name);
        if (upNode)
        {
            upNode->removeOutputConnection(node->getName());
            int num = node->getEdgeIndex(upNode->getId());
            // erase edge between node and up node
            if (num != -1)
            {
                if (node->edges.size() == 1)
                {
                    node->edges.erase(node->edges.begin() + 0);
                }
                else if (node->edges.size() > 1)
                {
                    node->edges.erase(node->edges.begin() + num);
                }
            }
        }
    }
    // update downNode info
    std::vector<UiNodePtr> downNodes = node->getOutputConnections();
    for (UiNodePtr downNode : downNodes)
    {
        int num = downNode->getEdgeIndex(node->getId());
        if (num != -1)
        {
            if (downNode->edges.size() == 1)
            {
                downNode->edges.erase(downNode->edges.begin() + 0);
            }
            else if (downNode->edges.size() > 1)
            {
                downNode->edges.erase(downNode->edges.begin() + num);
            }
        }

        downNode->setInputNodeNum(-1);
        // not really necessary since it will be deleted
        node->removeOutputConnection(downNode->getName());
        // change input so that is default val
        // change informtion of actual mx::Node
        if (downNode->getNode() != nullptr)
        {

            mx::NodeDefPtr nodeDef = downNode->getNode()->getNodeDef(downNode->getNode()->getName());

            for (Pin pin : downNode->inputPins)
            {

                if (pin._input->getConnectedNode())
                {
                    if (pin._input->getConnectedNode()->getName() == node->getName())
                    {
                        mx::ValuePtr val = nodeDef->getActiveInput(pin._input->getName())->getValue();
                        std::cout << nodeDef->asString() << std::endl;
                        if (downNode->getNode()->getType() == "surfaceshader")
                        {
                            pin._input->setConnectedOutput(nullptr);
                        }
                        else
                        {
                            pin._input->setConnectedNode(nullptr);
                        }

                        pin.setConnected(false);
                        if (val)
                        {
                            std::string valString = val->getValueString();
                            pin._input->setValueString(val->getValueString());
                            updateMaterials(pin._input, pin._input->getValue(), true);
                        }
                    }
                }
                else if (pin._input->getConnectedOutput())
                {
                    if (pin._input->getConnectedOutput()->getName() == node->getName())
                    {
                        mx::ValuePtr val = nodeDef->getActiveInput(pin._input->getName())->getValue();
                        std::cout << nodeDef->asString() << std::endl;
                        if (downNode->getNode()->getType() == "surfaceshader")
                        {
                            pin._input->setConnectedOutput(nullptr);
                        }
                        else
                        {
                            pin._input->setConnectedNode(nullptr);
                        }

                        pin.setConnected(false);
                        if (val)
                        {
                            std::string valString = val->getValueString();
                            pin._input->setValueString(val->getValueString());
                            updateMaterials(pin._input, pin._input->getValue(), true);
                        }
                    }
                }
            }
        }
        else if (downNode->getNodeGraph())
        {
            // set default values for nodegraph node pins ie nodegraph inputs
            mx::NodeDefPtr nodeDef = downNode->getNodeGraph()->getNodeDef();
            for (Pin pin : downNode->inputPins)
            {
                if (downNode->getConnectedNode(pin._name))
                {
                    if (downNode->getConnectedNode(pin._name) == node)
                    {

                        if (node->getInput())
                        {
                            downNode->getNodeGraph()->getInput(pin._name)->removeAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE);
                            setDefaults(node->getInput());
                        }
                        pin._input->setConnectedNode(nullptr);
                        pin.setConnected(false);
                        setDefaults(pin._input);
                        updateMaterials(pin._input, pin._input->getValue(), true);
                    }
                }
            }
        }
    }

    // remove from NodeGraph
    // all link information is handled in delete link which is called before this
    int nodeNum = findNode(node->getId());
    if (_isNodeGraph)
    {
        _currNodeGraph->removeChild(node->getName());
    }
    else
    {

        _graphDoc->removeChild(node->getName());
    }

    _graphNodes.erase(_graphNodes.begin() + nodeNum);
}

void Graph::upNodeGraph()
{
    if (!_graphStack.empty())
    {
        savePosition();
        _graphNodes = _graphStack.top();
        _graphStack.pop();
        _currGraphName.pop_back();
        _initial = true;
        ed::NavigateToContent();
        if (_currUiNode)
        {
            ed::DeselectNode(_currUiNode->getId());
            _currUiNode = nullptr;
        }
        _prevUiNode = nullptr;
        _currNodeGraph = nullptr;
        _isNodeGraph = false;
        _initial = true;
    }
}

void Graph::graphButtons()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.15f, .15f, .15f, 1.0f));

    // buttons for loading and saving a .mtlx
    std::string fileName = "";
    // new Material button
    if (ImGui::Button("New Material"))
    {
        _graphNodes.clear();
        _currLinks.clear();
        _currEdge.clear();
        _newLinks.clear();
        _graphDoc = nullptr;
        if (_currUiNode != nullptr)
        {
            ed::DeselectNode(_currUiNode->getId());
            _currUiNode = nullptr;
        }
        _prevUiNode = nullptr;
        _currNodeGraph = nullptr;
        _isNodeGraph = false;
        _currGraphName.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Material"))
    {
        // deselect node before loading new file
        if (_currUiNode != nullptr)
        {
            ed::DeselectNode(_currUiNode->getId());
            _currUiNode = nullptr;
        }

        _fileDialog.SetTitle("Open File Window");
        _fileDialog.Open();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Material"))
    {
        _fileDialogSave.SetTitle("Save File Window");
        _fileDialogSave.Open();
    }
    ImGui::SameLine();
    if (ImGui::Button("Auto Layout"))
    {
        _autoLayout = true;
    }

    // split window into panes for NodeEditor
    static float leftPaneWidth = 375.0f;
    static float rightPaneWidth = 750.0f;
    Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 20.0f, 20.0f);
    // create back button and graph hiearchy name display
    ImGui::Indent(leftPaneWidth + 15.f);
    if (ImGui::Button("<"))
    {
        upNodeGraph();
    }
    ImGui::SameLine();
    if (!_currGraphName.empty())
    {
        for (std::string name : _currGraphName)
        {
            ImGui::Text("%s", name.c_str());
            ImGui::SameLine();
            if (name != _currGraphName.back())
            {
                ImGui::Text(">");
                ImGui::SameLine();
            }
        }
    }

    ImGui::Unindent(leftPaneWidth + 15.f);
    ImGui::PopStyleColor();
    ImGui::NewLine();
    // creating two windows using splitter
    float paneWidth = (leftPaneWidth - 2.0f);
    ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

    // renderView window
    ImVec2 wsize = ImVec2((float) _renderer->_screenWidth, (float) _renderer->_screenHeight);
    float aspectRatio = _renderer->_pixelRatio;
    ImVec2 screenSize = ImVec2(paneWidth, paneWidth / aspectRatio);
    _renderer->_screenWidth = (unsigned int) screenSize[0];
    _renderer->_screenHeight = (unsigned int) screenSize[1];

    if (_renderer != nullptr)
    {

        glEnable(GL_FRAMEBUFFER_SRGB);
        _renderer->getViewCamera()->setViewportSize(mx::Vector2(screenSize[0], screenSize[1]));
        GLuint64 my_image_texture = _renderer->_textureID;
        mx::Vector2 vec = _renderer->getViewCamera()->getViewportSize();
        // current image has correct color space but causes problems for gui
        ImGui::Image((ImTextureID) my_image_texture, screenSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::Separator();

    // property editor for current nodes
    propertyEditor();
    ImGui::EndChild();
    ImGui::SameLine(0.0f, 12.0f);
}
void Graph::propertyEditor()
{
    ImGui::Text("Node Property Editor");
    if (_currUiNode)
    {
        // set and edit name
        ImGui::Text("Name: ");
        ImGui::SameLine();
        std::string original = _currUiNode->getName();
        std::string temp = original;
        ImGui::PushItemWidth(100.0f);
        ImGui::InputText("##edit", &temp);
        ImGui::PopItemWidth();
        std::string docString = "NodeDef Doc String: \n";
        if (_currUiNode->getNode())
        {
            mx::OutputPtr testOut = _currUiNode->getNode()->getOutput("output");
            if (temp != original)
            {

                std::string name = _currUiNode->getNode()->getParent()->createValidChildName(temp);

                std::vector<UiNodePtr> downstreamNodes = _currUiNode->getOutputConnections();
                for (UiNodePtr nodes : downstreamNodes)
                {
                    if (nodes->getInput() == nullptr)
                    {
                        for (mx::InputPtr input : nodes->getNode()->getActiveInputs())
                        {
                            if (input->getConnectedNode() == _currUiNode->getNode())
                            {
                                _currUiNode->getNode()->setName(name);
                                nodes->getNode()->setConnectedNode(input->getName(), _currUiNode->getNode());
                            }
                        }
                    }
                }
                _currUiNode->setName(name);
                _currUiNode->getNode()->setName(name);
            }
        }
        else if (_currUiNode->getInput())
        {
            if (temp != original)
            {

                std::string name = _currUiNode->getInput()->getParent()->createValidChildName(temp);

                std::vector<UiNodePtr> downstreamNodes = _currUiNode->getOutputConnections();
                for (UiNodePtr nodes : downstreamNodes)
                {
                    if (nodes->getInput() == nullptr)
                    {
                        if (nodes->getNode())
                        {
                            for (mx::InputPtr input : nodes->getNode()->getActiveInputs())
                            {
                                if (input->getInterfaceInput() == _currUiNode->getInput())
                                {
                                    _currUiNode->getInput()->setName(name);
                                    mx::ValuePtr val = _currUiNode->getInput()->getValue();
                                    mx::Vector3 test;
                                    std::string type = _currUiNode->getInput()->getType();
                                    input->setInterfaceName(name);
                                    mx::InputPtr pt = input->getInterfaceInput();
                                }
                            }
                        }
                        else
                        {
                            nodes->getOutput()->setConnectedNode(_currUiNode->getNode());
                        }
                    }
                }

                _currUiNode->getInput()->setName(name);
                _currUiNode->setName(name);
            }
        }
        else if (_currUiNode->getOutput())
        {
            if (temp != original)
            {
                std::string name = _currUiNode->getOutput()->getParent()->createValidChildName(temp);
                _currUiNode->getOutput()->setName(name);
                _currUiNode->setName(name);
            }
        }
        else if (_currUiNode->getCategory() == "group")
        {
            _currUiNode->setName(temp);
        }
        else if (_currUiNode->getCategory() == "nodegraph")
        {
            if (temp != original)
            {
                std::string name = _currUiNode->getNodeGraph()->getParent()->createValidChildName(temp);
                _currUiNode->getNodeGraph()->setName(name);
                _currUiNode->setName(name);
            }
        }

        ImGui::Text("Category:");
        ImGui::SameLine();
        // change button color to match background
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.096f, .096f, .096f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.1f, .1f, .1f, 1.0f));
        if (_currUiNode->getNode())
        {
            ImGui::Text("%s", _currUiNode->getNode()->getCategory().c_str());
            docString += _currUiNode->getNode()->getCategory().c_str();
            docString += ":";
            docString += _currUiNode->getNode()->getNodeDef()->getDocString() + "\n";
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("%s", _currUiNode->getNode()->getNodeDef()->getDocString().c_str());
            }

            std::vector<Pin> inputs = _currUiNode->inputPins;

            ImGui::Text("Inputs:");
            ImGui::Indent();
            for (size_t i = 0; i < inputs.size(); i++)
            {
                mx::OutputPtr out = inputs[i]._input->getConnectedOutput();
                // setting comment help box
                ImGui::PushID(int(inputs[i]._pinId.Get()));
                ImGui::Text("%s", inputs[i]._input->getName().c_str());
                std::string testStr = _currUiNode->getNode()->getNodeDefString();
                mx::InputPtr tempInt = _currUiNode->getNode()->getNodeDef()->getActiveInput(inputs[i]._input->getName());
                docString += inputs[i]._name;
                docString += ": ";
                if (tempInt)
                {
                    std::string docStr = _currUiNode->getNode()->getNodeDef()->getActiveInput(inputs[i]._input->getName())->getDocString();
                    if (docString != "")
                    {
                        docString += docStr;
                    }
                }
                docString += "\t \n";
                ImGui::SameLine();
                std::string typeText = " [" + inputs[i]._input->getType() + "]";
                ImGui::Text("%s", typeText.c_str());

                // setting constant sliders for input values
                if (!inputs[i].getConnected())
                {
                    setConstant(_currUiNode, inputs[i]._input);
                }

                ImGui::PopID();
            }
            ImGui::Unindent();
            ImGui::Checkbox("Show all inputs", &_currUiNode->_showAllInputs);
        }

        else if (_currUiNode->getInput() != nullptr)
        {
            ImGui::Text("%s", _currUiNode->getCategory().c_str());
            std::vector<Pin> inputs = _currUiNode->inputPins;
            ImGui::Text("Inputs:");
            ImGui::Indent();
            for (size_t i = 0; i < inputs.size(); i++)
            {

                // setting comment help box
                ImGui::PushID(int(inputs[i]._pinId.Get()));
                ImGui::Text("%s", inputs[i]._input->getName().c_str());

                ImGui::SameLine();
                std::string typeText = " [" + inputs[i]._input->getType() + "]";
                ImGui::Text("%s", typeText.c_str());
                // setting constant sliders for input values
                if (!inputs[i].getConnected())
                {
                    setConstant(_currUiNode, inputs[i]._input);
                }
                ImGui::PopID();
            }
            ImGui::Unindent();
        }
        else if (_currUiNode->getOutput() != nullptr)
        {
            ImGui::Text("%s", _currUiNode->getOutput()->getCategory().c_str());
        }
        else if (_currUiNode->getNodeGraph() != nullptr)
        {
            std::vector<Pin> inputs = _currUiNode->inputPins;
            ImGui::Text("%s", _currUiNode->getCategory().c_str());
            ImGui::Text("Inputs:");
            ImGui::Indent();
            int count = 0;
            for (Pin input : inputs)
            {
                // setting comment help box
                ImGui::PushID(int(input._pinId.Get()));
                ImGui::Text("%s", input._input->getName().c_str());

                docString += _currUiNode->getNodeGraph()->getActiveInput(input._input->getName())->getDocString();

                ImGui::SameLine();
                std::string typeText = " [" + input._input->getType() + "]";
                ImGui::Text("%s", typeText.c_str());
                std::vector<mx::InputPtr> actives = _currUiNode->getNodeGraph()->getActiveInputs();
                if (!input._input->getConnectedNode() && _currUiNode->getNodeGraph()->getActiveInput(input._input->getName()))
                {
                    setConstant(_currUiNode, input._input);
                }

                ImGui::PopID();
                count++;
            }
            ImGui::Unindent();
            ImGui::Checkbox("Show all inputs", &_currUiNode->_showAllInputs);
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        if (ImGui::Button("Node Info"))
        {
            ImGui::OpenPopup("docstring");
        }

        if (ImGui::BeginPopup("docstring"))
        {
            ImGui::Text("%s", docString.c_str());
            ImGui::EndPopup();
        }
    }
}
void Graph::addNodePopup(bool cursor)
{
    const bool open_AddPopup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(GLFW_KEY_TAB);
    if (open_AddPopup)
    {
        cursor = true;
        ImGui::OpenPopup("add node");
    }
    if (ImGui::BeginPopup("add node"))
    {
        _popup = true;
        // check if there is a document
        if (_graphDoc == nullptr)
        {
            // when creating files from scratch
            mx::DocumentPtr doc = mx::createDocument();
            doc->importLibrary(_stdLib);
            _graphDoc = doc;
            addExtraNodes();
        }

        ImGui::Text("Add Node");
        ImGui::Separator();
        static char input[16]{ "" };
        if (cursor)
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##input", input, sizeof(input));
        std::string subs(input);
        // input string length
        // filter extra nodes - includes inputs, outputs, groups, and node graphs
        for (std::unordered_map<std::string, std::vector<std::vector<std::string>>>::iterator it = _extraNodes.begin(); it != _extraNodes.end(); ++it)
        {
            // filter out list of nodes
            if (subs.size() > 0)
            {
                for (size_t i = 0; i < it->second.size(); i++)
                {
                    std::string str(it->second[i][0]);
                    std::string nodeName = it->second[i][0];
                    if (str.find(subs) != std::string::npos)
                    {
                        if (ImGui::MenuItem(nodeName.substr(3, nodeName.length()).c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
                        {
                            addNode(it->second[i][2], nodeName.substr(3, nodeName.length()), it->second[i][1]);
                            _addNewNode = true;
                            memset(input, '\0', sizeof(input));
                        }
                    }
                }
            }
            else
            {
                ImGui::SetNextWindowSizeConstraints(ImVec2(100, 10), ImVec2(250, 300));
                if (ImGui::BeginMenu(it->first.c_str()))
                {
                    for (size_t j = 0; j < it->second.size(); j++)
                    {
                        std::string name = it->second[j][0];
                        if (ImGui::MenuItem(name.substr(3, name.length()).c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
                        {
                            addNode(it->second[j][2], name.substr(3, name.length()), it->second[j][1]);
                            _addNewNode = true;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
        }
        // filter nodedefs and add to menu if matches filter
        for (std::unordered_map<std::string, std::vector<mx::NodeDefPtr>>::iterator it = _nodesToAdd.begin(); it != _nodesToAdd.end(); ++it)
        {
            // filter out list of nodes
            if (subs.size() > 0)
            {
                for (size_t i = 0; i < it->second.size(); i++)
                {
                    std::string str(it->second[i]->getName());
                    std::string nodeName = it->second[i]->getName();
                    if (str.find(subs) != std::string::npos)
                    {
                        if (ImGui::MenuItem(it->second[i]->getName().substr(3, nodeName.length()).c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
                        {
                            addNode(it->second[i]->getNodeString(), it->second[i]->getName().substr(3, nodeName.length()), it->second[i]->getType());
                            _addNewNode = true;
                            memset(input, '\0', sizeof(input));
                        }
                    }
                }
            }
            else
            {
                ImGui::SetNextWindowSizeConstraints(ImVec2(100, 10), ImVec2(250, 300));
                if (ImGui::BeginMenu(it->first.c_str()))
                {
                    for (size_t i = 0; i < it->second.size(); i++)
                    {

                        std::string name = it->second[i]->getName();
                        if (ImGui::MenuItem(it->second[i]->getName().substr(3, name.length()).c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
                        {
                            addNode(it->second[i]->getNodeString(), it->second[i]->getName().substr(3, name.length()), it->second[i]->getType());
                            _addNewNode = true;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
        }

        cursor = false;
        ImGui::EndPopup();
    }
}
void Graph::searchNodePopup(bool cursor)
{
    const bool open_search = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyDown(GLFW_KEY_F) && ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL);
    if (open_search)
    {
        cursor = true;
        ImGui::OpenPopup("search");
    }
    if (ImGui::BeginPopup("search"))
    {
        ed::NavigateToSelection();
        static ImGuiTextFilter filter;
        ImGui::Text("Search for Node:");
        static char input[16]{ "" };
        ImGui::SameLine();
        if (cursor)
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##input", input, sizeof(input));

        if (std::string(input).size() > 0)
        {

            for (UiNodePtr node : _graphNodes)
            {
                if (node->getName().find(std::string(input)) != std::string::npos)
                {

                    if (ImGui::MenuItem(node->getName().c_str()) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
                    {
                        _searchNodeId = node->getId();
                        memset(input, '\0', sizeof(input));
                    }
                }
            }
        }
        cursor = false;
        ImGui::EndPopup();
    }
}
// sets up graph editor
void Graph::drawGraph(ImVec2 mousePos)
{
    if (_searchNodeId > 0)
    {
        ed::SelectNode(_searchNodeId);
        ed::NavigateToSelection();
        _searchNodeId = -1;
    }

    std::string currName;
    bool TextCursor = false;
    // center imgui window and setting size
    ImGuiIO& io2 = ImGui::GetIO();
    ImGui::SetNextWindowSize(io2.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(io2.DisplaySize.x * 0.5f, io2.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("MaterialX", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    io2.ConfigFlags = ImGuiConfigFlags_IsSRGB | ImGuiConfigFlags_NavEnableKeyboard;
    io2.MouseDoubleClickTime = .5;
    // increase default font size
    ImFont* f = ImGui::GetFont();
    f->FontSize = 14;

    graphButtons();

    ed::Begin("My Editor");
    {
        ed::Suspend();
        // set up pop ups for adding a node when tab is pressed

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
        ImGui::SetNextWindowSize({ 250.0f, 300.0f });
        addNodePopup(TextCursor);
        searchNodePopup(TextCursor);
        ImGui::PopStyleVar();

        ed::Resume();
        std::vector<ed::NodeId> selectedNodes;
        std::vector<ed::LinkId> selectedLinks;
        selectedNodes.resize(ed::GetSelectedObjectCount());
        selectedLinks.resize(ed::GetSelectedObjectCount());

        int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
        int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

        selectedNodes.resize(nodeCount);
        selectedLinks.resize(linkCount);
        if (io2.KeyCtrl && io2.MouseDown[0])
        {
            _ctrlClick = true;
        }

        // setting current node based off of selected node
        if (selectedNodes.size() > 0)
        {
            int graphPos = findNode(int(selectedNodes[0].Get()));
            if (graphPos > -1)
            {
                // only selected not if its not the same as previously selected
                if (!_prevUiNode || (_prevUiNode->getName() != _graphNodes[graphPos]->getName()))
                {
                    _currUiNode = _graphNodes[graphPos];
                    // update render material if needed
                    if (_currUiNode->getNode())
                    {
                        if (_currUiNode->getNode()->getType() == mx::SURFACE_SHADER_TYPE_STRING || _currUiNode->getNode()->getType() == mx::MATERIAL_TYPE_STRING)
                        {
                            setRenderMaterial(_currUiNode);
                        }
                    }
                    else if (_currUiNode->getNodeGraph())
                    {
                        setRenderMaterial(_currUiNode);
                    }
                    _prevUiNode = _currUiNode;
                }
            }
        }

        // check if keyboard shortcuts for copy/cut/paste have been used
        if (ed::BeginShortcut())
        {
            if (ed::AcceptCopy())
            {
                _copiedNodes.clear();
                for (ed::NodeId selected : selectedNodes)
                {
                    int pos = findNode((int) selected.Get());
                    if (pos >= 0)
                    {
                        _copiedNodes.insert(std::pair<UiNodePtr, UiNodePtr>(_graphNodes[pos], nullptr));
                    }
                }
            }
            else if (ed::AcceptCut())
            {
                _copiedNodes.clear();
                // same as copy but remove from graphNodes
                for (ed::NodeId selected : selectedNodes)
                {
                    int pos = findNode((int) selected.Get());
                    if (pos >= 0)
                    {
                        _copiedNodes.insert(std::pair<UiNodePtr, UiNodePtr>(_graphNodes[pos], nullptr));
                    }
                }
                cutNodes();
                _isCut = true;
            }
            else if (ed::AcceptPaste())
            {
                for (std::map<UiNodePtr, UiNodePtr>::iterator iter = _copiedNodes.begin(); iter != _copiedNodes.end(); iter++)
                {
                    copyUiNode(iter->first);
                }
                _addNewNode = true;
            }
        }
        // set y position of first node
        std::vector<int> outputNum = CreateNodes(_isNodeGraph);

        // address copy information if applicable and relink graph if a new node has been added
        if (_addNewNode)
        {
            copyInputs();
            linkGraph();
            ImVec2 canvasPos = ed::ScreenToCanvas(mousePos);
            // place the copied nodes or the individual new nodes
            if ((int) _copiedNodes.size() > 0)
            {
                positionPasteBin(canvasPos);
            }
            else
            {
                ed::SetNodePosition(_graphNodes.back()->getId(), canvasPos);
            }
            _copiedNodes.clear();
            _addNewNode = false;
        }
        // layout and link graph during the initial call of drawGraph()
        if (_initial || _autoLayout)
        {
            _currLinks.clear();
            float y = 0.f;
            _levelMap = std::unordered_map<int, std::vector<UiNodePtr>>();
            // start layout with output or material nodes since layout algorithm works right to left
            for (int outN : outputNum)
            {
                layoutPosition(_graphNodes[outN], ImVec2(1200.f, y), true, 0);
                y += 350;
            }
            // if there are no output or material nodes but the nodes have position layout each individual node
            if (_graphNodes.size() > 0)
            {

                if (outputNum.size() == 0 && _graphNodes[0]->getMxElement())
                {
                    if (_graphNodes[0]->getMxElement()->hasAttribute("xpos"))
                    {
                        for (UiNodePtr node : _graphNodes)
                        {
                            layoutPosition(node, ImVec2(0, 0), true, 0);
                        }
                    }
                }
            }
            linkGraph();
            findYSpacing(0.f);
            layoutInputs();
            // automatically frame node graph upon loading
            ed::NavigateToContent();
        }
        if (_delete)
        {
            linkGraph();

            _delete = false;
        }
        connectLinks();
        // set to false after intial layout so that nodes can be moved
        _initial = false;
        _autoLayout = false;
        // delete selected nodes and their links if delete key is pressed or if the shortcut for cut is used
        if (ImGui::IsKeyReleased(GLFW_KEY_DELETE) || _isCut)
        {
            if (selectedNodes.size() > 0)
            {
                for (ed::NodeId id : selectedNodes)
                {

                    if (int(id.Get()) > 0)
                    {
                        int pos = findNode(int(id.Get()));
                        if (pos >= 0)
                        {

                            deleteNode(_graphNodes[pos]);
                            _delete = true;
                            ed::DeselectNode(id);
                            ed::DeleteNode(id);
                            _currUiNode = nullptr;
                        }
                    }
                }
                linkGraph();
            }
            _isCut = false;
        }

        // hotkey to frame selected node(s)
        if (ImGui::IsKeyReleased(GLFW_KEY_F))
        {
            ed::NavigateToSelection();
        }

        // go back up from inside a subgraph
        if (ImGui::IsKeyReleased(GLFW_KEY_U) && (!ImGui::IsPopupOpen("add node")) && (!ImGui::IsPopupOpen("search")))
        {
            upNodeGraph();
        }
        // adding new link
        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId, filterPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                AddLink(inputPinId, outputPinId);
            }
            if (ed::QueryNewNode(&filterPinId))
            {
                if (getPin(filterPinId)._type != "null")
                {
                    _pinFilterType = getPin(filterPinId)._type;
                }
            }
        }
        else
        {
            _pinFilterType = "";
        }
        ed::EndCreate();
        // deleting link
        if (ed::BeginDelete())
        {
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                deleteLink(deletedLinkId);
            }
        }
        ed::EndDelete();
    }

    // diving into a node that has a subgraph
    ed::NodeId clickedNode = ed::GetDoubleClickedNode();
    if (clickedNode.Get() > 0)
    {
        if (_currUiNode != nullptr)
        {
            if (_currUiNode->getNode() != nullptr)
            {

                mx::InterfaceElementPtr impl = _currUiNode->getNode()->getImplementation();
                // only dive if current node is a node graph
                if (impl && impl->isA<mx::NodeGraph>())
                {
                    _graphStack.push(_graphNodes);
                    mx::NodeGraphPtr implGraph = impl->asA<mx::NodeGraph>();
                    _initial = true;
                    _graphNodes.clear();
                    ed::DeselectNode(_currUiNode->getId());
                    _currUiNode = nullptr;
                    _currNodeGraph = implGraph;
                    _currGraphName.push_back(implGraph->getName());
                    buildUiNodeGraph(implGraph);
                    ed::NavigateToContent();
                }
            }
            else if (_currUiNode->getNodeGraph() != nullptr)
            {
                _graphStack.push(_graphNodes);
                mx::NodeGraphPtr implGraph = _currUiNode->getNodeGraph();
                _currUiNode->getOutputConnections();
                _initial = true;
                _graphNodes.clear();
                _isNodeGraph = true;
                setRenderMaterial(_currUiNode);
                ed::DeselectNode(_currUiNode->getId());
                _currUiNode = nullptr;
                _currNodeGraph = implGraph;
                _currGraphName.push_back(implGraph->getName());
                buildUiNodeGraph(implGraph);
                ed::NavigateToContent();
            }
        }
    }

    ed::Suspend();
    _fileDialogSave.Display();
    // saving file
    if (_fileDialogSave.HasSelected())
    {

        std::string message;
        if (!_graphDoc->validate(&message))
        {
            std::cerr << "*** Validation warnings for " << _materialFilename.getBaseName() << " ***" << std::endl;
            std::cerr << message;
        }
        std::string fileName = _fileDialogSave.GetSelected().string();
        mx::FilePath name = _fileDialogSave.GetSelected().string();
        ed::Resume();
        savePosition();

        writeText(fileName, name);
        _fileDialogSave.ClearSelected();
    }
    else
    {
        ed::Resume();
    }

    ed::End();
    ImGui::End();
    _fileDialog.Display();
    // create and load document from selected file
    if (_fileDialog.HasSelected())
    {
        mx::FilePath fileName = mx::FilePath(_fileDialog.GetSelected().string());

        _currGraphName.clear();
        std::string graphName = fileName.getBaseName();
        _currGraphName.push_back(graphName.substr(0, graphName.length() - 5));
        mx::DocumentPtr doc = loadDocument(fileName);

        _graphDoc = doc;

        _initial = true;
        std::vector<mx::NodeGraphPtr> nodeGraphs = _graphDoc->getNodeGraphs();
        std::vector<mx::InputPtr> inputNodes = _graphDoc->getActiveInputs();
        std::vector<mx::NodePtr> docNodes = _graphDoc->getNodes();
        buildUiBaseGraph(nodeGraphs, docNodes, inputNodes);
        _graphDoc->importLibrary(_stdLib);
        _renderer->loadDocument(fileName, _stdLib);
        if (_nodesToAdd.size() == 0)
        {
            std::vector<mx::NodeDefPtr> nodeDefs = _stdLib->getNodeDefs();
            for (size_t i = 0; i < nodeDefs.size(); i++)
            {
                // nodeDef group is the key for the map
                std::string group = nodeDefs[i]->getNodeGroup();
                std::unordered_map<std::string, mx::NodeDefPtr> test;
                std::unordered_map<std::string, std::vector<mx::NodeDefPtr>>::iterator it = _nodesToAdd.find(group);
                if (it == _nodesToAdd.end())
                {
                    std::vector<mx::NodeDefPtr> nodes;
                    _nodesToAdd[group] = nodes;
                }
                _nodesToAdd[group].push_back(nodeDefs[i]);
            }
        }
        addExtraNodes();
        _fileDialog.ClearSelected();
    }

    _fileDialogConstant.Display();
}

// return node location in graphNodes vector based off of node id
int Graph::findNode(int nodeId)
{
    int count = 0;
    for (size_t i = 0; i < _graphNodes.size(); i++)
    {
        if (_graphNodes[i]->getId() == nodeId)
        {
            return count;
        }
        count++;
    }
    return -1;
}

// find a link based on an attribute id
std::vector<int> Graph::findLinkId(int id)
{
    std::vector<int> ids;
    for (const Link& link : _currLinks)
    {
        if (link._startAttr == id || link._endAttr == id)
        {
            ids.push_back(link.id);
        }
    }
    return ids;
}
// check if current edge is already in edge vector
bool Graph::edgeExists(Edge newEdge)
{
    if (_currEdge.size() > 0)
    {
        for (Edge edge : _currEdge)
        {
            if (edge.getDown()->getId() == newEdge.getDown()->getId())
            {
                if (edge.getUp()->getId() == newEdge.getUp()->getId())
                {
                    if (edge.getInput() == newEdge.getInput())
                    {
                        return true;
                    }
                }
            }
        }
    }
    else
    {
        return false;
    }
    return false;
}

// check if a link exists in currLink vector
bool Graph::linkExists(Link newLink)
{
    for (const auto& link : _currLinks)
    {
        if (link._startAttr == newLink._startAttr)
        {
            if (link._endAttr == newLink._endAttr)
            {
                // link exists
                return true;
            }
        }
        else if (link._startAttr == newLink._endAttr)
        {
            if (link._endAttr == newLink._startAttr)
            {
                // link exists
                return true;
            }
        }
    }
    return false;
}

// set materialX attribute positions for nodes which changed pos
void Graph::savePosition()
{
    for (UiNodePtr node : _graphNodes)
    {
        if (node->getMxElement() != nullptr)
        {
            ImVec2 pos = ed::GetNodePosition(node->getId());
            pos.x /= DEFAULT_NODE_SIZE.x;
            pos.y /= DEFAULT_NODE_SIZE.y;
            node->getMxElement()->setAttribute("xpos", std::to_string(pos.x));
            node->getMxElement()->setAttribute("ypos", std::to_string(pos.y));
        }
    }
}
void Graph::writeText(std::string fileName, mx::FilePath filePath)
{
    if (filePath.getExtension() != mx::MTLX_EXTENSION)
    {
        filePath.addExtension(mx::MTLX_EXTENSION);
    }

    mx::XmlWriteOptions writeOptions;
    writeOptions.elementPredicate = _renderer->getElementPredicate();
    mx::writeToXmlFile(_graphDoc, filePath, &writeOptions);
}
