#include <MaterialXGenShader/DagNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenImplementation.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <iostream>
#include <sstream>
#include <stack>

namespace MaterialX
{

void DagInput::makeConnection(DagOutput* src)
{
    this->connection = src;
    src->connections.insert(this);
}

void DagInput::breakConnection()
{
    if (this->connection)
    {
        this->connection->connections.erase(this);
        this->connection = nullptr;
    }
}

void DagOutput::makeConnection(DagInput* dst)
{
    dst->connection = this;
    this->connections.insert(dst);
}

void DagOutput::breakConnection(DagInput* dst)
{
    this->connections.erase(dst);
    dst->connection = nullptr;
}

void DagOutput::breakConnection()
{
    for (DagInput* input : this->connections)
    {
        input->connection = nullptr;
    }
    this->connections.clear();
}

DagEdgeIterator DagOutput::traverseUpstream()
{
    return DagEdgeIterator(this);
}

namespace
{
    DagNodePtr createEmptyNode()
    {
        DagNodePtr node = std::make_shared<DagNode>("");
        node->addContextID(ShaderGenerator::CONTEXT_DEFAULT);
        return node;
    }
}

const DagNodePtr DagNode::NONE = createEmptyNode();

const string DagNode::SXCLASS_ATTRIBUTE = "sxclass";
const string DagNode::CONSTANT = "constant";
const string DagNode::IMAGE = "image";
const string DagNode::COMPARE = "compare";
const string DagNode::SWITCH = "switch";
const string DagNode::BSDF_R = "R";
const string DagNode::BSDF_T = "T";

bool DagNode::referencedConditionally() const
{
    if (_scopeInfo.type == DagNode::ScopeInfo::Type::SINGLE)
    {
        int numBranches = 0;
        uint32_t mask = _scopeInfo.conditionBitmask;
        for (; mask != 0; mask >>= 1)
        {
            if (mask & 1)
            {
                numBranches++;
            }
        }
        return numBranches > 0;
    }
    return false;
}

void DagNode::ScopeInfo::adjustAtConditionalInput(DagNode* condNode, int branch, const uint32_t fullMask)
{
    if (type == ScopeInfo::Type::GLOBAL || (type == ScopeInfo::Type::SINGLE && conditionBitmask == fullConditionMask))
    {
        type = ScopeInfo::Type::SINGLE;
        conditionalNode = condNode;
        conditionBitmask = 1 << branch;
        fullConditionMask = fullMask;
    }
    else if (type == ScopeInfo::Type::SINGLE)
    {
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

void DagNode::ScopeInfo::merge(const ScopeInfo &fromScope)
{
    if (type == ScopeInfo::Type::UNKNOWN || fromScope.type == ScopeInfo::Type::GLOBAL)
    {
        *this = fromScope;
    }
    else if (type == ScopeInfo::Type::GLOBAL)
    {

    }
    else if (type == ScopeInfo::Type::SINGLE && fromScope.type == ScopeInfo::Type::SINGLE && conditionalNode == fromScope.conditionalNode)
    {
        conditionBitmask |= fromScope.conditionBitmask;

        // This node is needed for all branches so it is no longer conditional
        if (conditionBitmask == fullConditionMask)
        {
            type = ScopeInfo::Type::GLOBAL;
            conditionalNode = nullptr;
        }
    }
    else
    {
        // NOTE: Right now multiple scopes is not really used, it works exactly as ScopeInfo::Type::GLOBAL
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

DagNode::DagNode(const string& name)
    : _name(name)
    , _classification(0)
    , _samplingInput(nullptr)
    , _impl(nullptr)
{
}

static bool elementCanBeSampled2D(const Element& element)
{
    const string TEXCOORD_NAME("texcoord");
    return (element.getName() == TEXCOORD_NAME);
}

static bool elementCanBeSampled3D(const Element& element)
{
    const string POSITION_NAME("position");
    return (element.getName() == POSITION_NAME);
}

DagNodePtr DagNode::create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance)
{
    DagNodePtr newNode = std::make_shared<DagNode>(name);

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(shadergen.getTarget(), shadergen.getLanguage());
    if (impl)
    {
        newNode->_impl = shadergen.getImplementation(impl);
    }
    if (!newNode->_impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    // Check for classification based on group name
    unsigned int groupClassification = 0;
    const string TEXTURE2D_GROUPNAME("texture2d");
    const string TEXTURE3D_GROUPNAME("texture3d");
    const string PROCEDURAL2D_GROUPNAME("procedural2d");
    const string PROCEDURAL3D_GROUPNAME("procedural3d");
    const string CONVOLUTION2D_GROUPNAME("convolution2d");
    string groupName = nodeDef.getNodeGroup();
    if (!groupName.empty())
    {
        if (groupName == TEXTURE2D_GROUPNAME || groupName == PROCEDURAL2D_GROUPNAME)
        {
            groupClassification = Classification::SAMPLE2D;
        }
        else if (groupName == TEXTURE3D_GROUPNAME || groupName == PROCEDURAL3D_GROUPNAME)
        {
            groupClassification = Classification::SAMPLE3D;
        }
        else if (groupName == CONVOLUTION2D_GROUPNAME)
        {
            groupClassification = Classification::CONVOLUTION2D;
        }
    }
    newNode->_samplingInput = nullptr;

    // Create interface from nodedef
    const vector<ValueElementPtr> nodeDefInputs = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& elem : nodeDefInputs)
    {
        if (elem->isA<Output>())
        {
            newNode->addOutput(elem->getName(), TypeDesc::get(elem->getType()));
        }
        else
        {
            DagInput* input = newNode->addInput(elem->getName(), TypeDesc::get(elem->getType()));
            if (!elem->getValueString().empty())
            {
                input->value = elem->getValue();
            }

            // Determine if this input can be sampled
            if ((groupClassification == Classification::SAMPLE2D && elementCanBeSampled2D(*elem)) ||
                (groupClassification == Classification::SAMPLE3D && elementCanBeSampled3D(*elem)))
            {
                newNode->_samplingInput = input;
            }
        }
    }

    // Add a default output if needed
    if (newNode->numOutputs() == 0)
    {
        newNode->addOutput("out", TypeDesc::get(nodeDef.getType()));
    }

    // Assign input values from the node instance
    if (nodeInstance)
    {
        const vector<ValueElementPtr> nodeInstanceInputs = nodeInstance->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& elem : nodeInstanceInputs)
        {
            if (!elem->getValueString().empty())
            {
                DagInput* input = newNode->getInput(elem->getName());
                if (input)
                {       
                    input->value = elem->getValue();
                }
            }
        }
    }

    //
    // Set node classification, defaulting to texture node
    //
    newNode->_classification = Classification::TEXTURE;

    // First, check for specific output types
    const DagOutput* primaryOutput = newNode->getOutput();
    if (primaryOutput->type == Type::SURFACESHADER)
    {
        newNode->_classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (primaryOutput->type == Type::LIGHTSHADER)
    {
        newNode->_classification = Classification::LIGHT | Classification::SHADER;
    }
    else if (primaryOutput->type == Type::BSDF)
    {
        newNode->_classification = Classification::BSDF | Classification::CLOSURE;

        // Add additional classifications if the BSDF is restricted to
        // only reflection or transmission
        const string& bsdfType = nodeDef.getAttribute("bsdf");
        if (bsdfType == BSDF_R)
        {
            newNode->_classification |= Classification::BSDF_R;
        }
        else if (bsdfType == BSDF_T)
        {
            newNode->_classification |= Classification::BSDF_T;
        }
    }
    else if (primaryOutput->type == Type::EDF)
    {
        newNode->_classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (primaryOutput->type == Type::VDF)
    {
        newNode->_classification = Classification::VDF | Classification::CLOSURE;
    }
    // Second, check for specific nodes types
    else if (nodeDef.getNodeString() == CONSTANT)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONSTANT;
    }
    else if (nodeDef.getNodeString() == IMAGE || nodeDef.getAttribute(SXCLASS_ATTRIBUTE) == IMAGE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }
    else if (nodeDef.getNodeString() == COMPARE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::IFELSE;
    }
    else if (nodeDef.getNodeString() == SWITCH)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::SWITCH;
    }

    // Add in group classification
    newNode->_classification |= groupClassification;

    // Let the shader generator assign in which contexts to use this node
    shadergen.addNodeContextIDs(newNode.get());

    return newNode;
}

DagInput* DagNode::getInput(const string& name)
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

DagOutput* DagNode::getOutput(const string& name)
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

const DagInput* DagNode::getInput(const string& name) const
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

const DagOutput* DagNode::getOutput(const string& name) const
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

DagInput* DagNode::addInput(const string& name, const TypeDesc* type)
{
    if (getInput(name))
    {
        throw ExceptionShaderGenError("An input named '" + name + "' already exists on node '" + _name + "'");
    }

    DagInputPtr input = std::make_shared<DagInput>();
    input->name = name;
    input->type = type;
    input->node = this;
    input->value = nullptr;
    input->connection = nullptr;
    _inputMap[name] = input;
    _inputOrder.push_back(input.get());

    return input.get();
}

DagOutput* DagNode::addOutput(const string& name, const TypeDesc* type)
{
    if (getOutput(name))
    {
        throw ExceptionShaderGenError("An output named '" + name + "' already exists on node '" + _name + "'");
    }

    DagOutputPtr output = std::make_shared<DagOutput>();
    output->name = name;
    output->type = type;
    output->node = this;
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

void DagNode::renameInput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _inputMap.find(name);
        if (it != _inputMap.end())
        {
            it->second->name = newName;
            _inputMap[newName] = it->second;
            _inputMap.erase(it);
        }
    }
}

void DagNode::renameOutput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _outputMap.find(name);
        if (it != _outputMap.end())
        {
            it->second->name = newName;
            _outputMap[newName] = it->second;
            _outputMap.erase(it);
        }
    }
}

namespace
{
    static const DagEdgeIterator NULL_EDGE_ITERATOR(nullptr);
}

DagEdgeIterator::DagEdgeIterator(DagOutput* output)
    : _upstream(output)
    , _downstream(nullptr)
{
}

DagEdgeIterator& DagEdgeIterator::operator++()
{
    if (_upstream && _upstream->node->numInputs())
    {
        // Traverse to the first upstream edge of this element.
        _stack.push_back(StackFrame(_upstream, 0));

        DagInput* input = _upstream->node->getInput(0);
        DagOutput* output = input->connection;

        if (output && !output->node->isAGraph())
        {
            extendPathUpstream(output, input);
            return *this;
        }
    }

    while (true)
    {
        if (_upstream)
        {
            returnPathDownstream(_upstream);
        }

        if (_stack.empty())
        {
            // Traversal is complete.
            *this = DagEdgeIterator::end();
            return *this;
        }

        // Traverse to our siblings.
        StackFrame& parentFrame = _stack.back();
        while (parentFrame.second + 1 < parentFrame.first->node->numInputs())
        {
            DagInput* input = parentFrame.first->node->getInput(++parentFrame.second);
            DagOutput* output = input->connection;

            if (output && !output->node->isAGraph())
            {
                extendPathUpstream(output, input);
                return *this;
            }
        }

        // Traverse to our parent's siblings.
        returnPathDownstream(parentFrame.first);
        _stack.pop_back();
    }

    return *this;
}

const DagEdgeIterator& DagEdgeIterator::end()
{
    return NULL_EDGE_ITERATOR;
}

void DagEdgeIterator::extendPathUpstream(DagOutput* upstream, DagInput* downstream)
{
    // Check for cycles.
    if (_path.count(upstream))
    {
        throw ExceptionFoundCycle("Encountered cycle at element: " + upstream->node->getName() + "." + upstream->name);
    }

    // Extend the current path to the new element.
    _path.insert(upstream);
    _upstream = upstream;
    _downstream = downstream;
}

void DagEdgeIterator::returnPathDownstream(DagOutput* upstream)
{
    _path.erase(upstream);
    _upstream = nullptr;
    _downstream = nullptr;
}

} // namespace MaterialX
