#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
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

void ShaderInput::makeConnection(ShaderOutput* src)
{
    this->connection = src;
    src->connections.insert(this);
}

void ShaderInput::breakConnection()
{
    if (this->connection)
    {
        this->connection->connections.erase(this);
        this->connection = nullptr;
    }
}

void ShaderOutput::makeConnection(ShaderInput* dst)
{
    dst->connection = this;
    this->connections.insert(dst);
}

void ShaderOutput::breakConnection(ShaderInput* dst)
{
    this->connections.erase(dst);
    dst->connection = nullptr;
}

void ShaderOutput::breakConnection()
{
    for (ShaderInput* input : this->connections)
    {
        input->connection = nullptr;
    }
    this->connections.clear();
}

namespace
{
    ShaderNodePtr createEmptyNode()
    {
        return std::make_shared<ShaderNode>(nullptr, "");
    }
}

const ShaderNodePtr ShaderNode::NONE = createEmptyNode();

const string ShaderNode::CONSTANT = "constant";
const string ShaderNode::IMAGE = "image";
const string ShaderNode::COMPARE = "compare";
const string ShaderNode::SWITCH = "switch";
const string ShaderNode::BSDF_R = "R";
const string ShaderNode::BSDF_T = "T";

bool ShaderNode::referencedConditionally() const
{
    if (_scopeInfo.type == ShaderNode::ScopeInfo::Type::SINGLE)
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

void ShaderNode::ScopeInfo::adjustAtConditionalInput(ShaderNode* condNode, int branch, const uint32_t fullMask)
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

void ShaderNode::ScopeInfo::merge(const ScopeInfo &fromScope)
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

ShaderNode::ShaderNode(const ShaderGraph* parent, const string& name)
    : _parent(parent)
    , _name(name)
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

ShaderNodePtr ShaderNode::create(const ShaderGraph* parent, const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, GenContext& context)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(parent, name);

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(shadergen.getTarget(), shadergen.getLanguage());
    if (impl)
    {
        newNode->_impl = shadergen.getImplementation(impl, context);
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
            ShaderInput* input = nullptr;
            const TypeDesc* enumerationType = nullptr;
            ValuePtr enumValue = shadergen.remapEnumeration(elem, nodeDef, enumerationType);
            if (enumerationType)
            {
                input = newNode->addInput(elem->getName(), enumerationType);
                if (enumValue)
                {
                    input->value = enumValue;
                }
            }
            else
            {
                const TypeDesc* elemTypeDesc = TypeDesc::get(elem->getType());
                input = newNode->addInput(elem->getName(), elemTypeDesc);
                if (!elem->getValueString().empty())
                {
                    input->value = elem->getValue();
                }
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

    //
    // Set node classification, defaulting to texture node
    //
    newNode->_classification = Classification::TEXTURE;

    // First, check for specific output types
    const ShaderOutput* primaryOutput = newNode->getOutput();
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
    else if (nodeDef.getNodeString() == COMPARE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::IFELSE;
    }
    else if (nodeDef.getNodeString() == SWITCH)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::SWITCH;
    }
    // Third, check for file texture classification by group name
    else if (groupName == TEXTURE2D_GROUPNAME || groupName == TEXTURE3D_GROUPNAME)
    {
        newNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }

    // Add in group classification
    newNode->_classification |= groupClassification;

    return newNode;
}

void ShaderNode::setPaths(const Node& node, const NodeDef& nodeDef, bool includeNodeDefInputs)
{
    // Set element paths for children on the node
    const vector<ValueElementPtr> nodeValues = node.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& nodeValue : nodeValues)
    {
        ShaderInput* input = getInput(nodeValue->getName());
        if (input)
        {
            input->path = nodeValue->getNamePath();
        }
    }

    if (!includeNodeDefInputs)
    {
        return;
    }

    // Set element paths based on the node definition. Note that these
    // paths don't actually exist at time of shader generation since there
    // are no inputs/parameters specified on the node itself
    //
    const vector<InputPtr> nodeInputs = nodeDef.getChildrenOfType<Input>();
    const string& nodePath = node.getNamePath();
    for (const ValueElementPtr& nodeInput : nodeInputs)
    {
        ShaderInput* input = getInput(nodeInput->getName());
        if (input && input->path.empty())
        {
            input->path = nodePath + NAME_PATH_SEPARATOR + nodeInput->getName();
        }
    }
    const vector<ParameterPtr> nodeParameters = nodeDef.getChildrenOfType<Parameter>();
    for (const ParameterPtr& nodeParameter : nodeParameters)
    {
        ShaderInput* input = getInput(nodeParameter->getName());
        if (input && input->path.empty())
        {
            input->path = nodePath + NAME_PATH_SEPARATOR + nodeParameter->getName();
        }
    }
}

void ShaderNode::setValues(const Node& node, const NodeDef& nodeDef, ShaderGenerator& shadergen)
{
    // Copy input values from the given node
    const vector<ValueElementPtr> nodeValues = node.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& nodeValue : nodeValues)
    {
        const string& valueString = nodeValue->getValueString();
        ShaderInput* input = getInput(nodeValue->getName());
        if (input)
        {
            const TypeDesc* enumerationType = nullptr;
            ValuePtr value = shadergen.remapEnumeration(nodeValue, nodeDef, enumerationType);
            if (value)
            {
                input->value = value;
            }
            else if (!valueString.empty())
            {
                input->value = nodeValue->getValue();
            }
        }
    } 
}

ShaderNodePtr ShaderNode::create(const ShaderGraph* parent, const string& name, ShaderNodeImplPtr impl, unsigned int classification)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(parent, name);
    newNode->_impl = impl;
    newNode->_classification = classification;
    return newNode;
}

ShaderNodePtr ShaderNode::createColorTransformNode(const ShaderGraph* parent, const string& name, ShaderNodeImplPtr shaderImpl, const TypeDesc* type)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(parent, name);
    newNode->_impl = shaderImpl;
    newNode->_classification = Classification::TEXTURE | Classification::COLOR_SPACE_TRANSFORM;
    ShaderInput* input = newNode->addInput("in", type);

    if(type == Type::COLOR3)
    {
        input->value = Value::createValue(Color3(0.0f, 0.0f, 0.0f));
    }
    else if(type == Type::COLOR4)
    {
        input->value = Value::createValue(Color4(0.0f, 0.0f, 0.0f, 1.0));
    }
    else
    {
        throw ExceptionShaderGenError("Invalid type specified to createColorTransform: '" + type->getName() + "'");
    }
    newNode->addOutput("out", type);

    return newNode;
}

ShaderInput* ShaderNode::getInput(const string& name)
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

ShaderOutput* ShaderNode::getOutput(const string& name)
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

const ShaderInput* ShaderNode::getInput(const string& name) const
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

const ShaderOutput* ShaderNode::getOutput(const string& name) const
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

ShaderInput* ShaderNode::addInput(const string& name, const TypeDesc* type)
{
    if (getInput(name))
    {
        throw ExceptionShaderGenError("An input named '" + name + "' already exists on node '" + _name + "'");
    }

    ShaderInputPtr input = std::make_shared<ShaderInput>();
    input->name = name;
    input->path = EMPTY_STRING;
    input->variable = name;
    input->type = type;
    input->node = this;
    input->value = nullptr;
    input->connection = nullptr;
    _inputMap[name] = input;
    _inputOrder.push_back(input.get());

    return input.get();
}

ShaderOutput* ShaderNode::addOutput(const string& name, const TypeDesc* type)
{
    if (getOutput(name))
    {
        throw ExceptionShaderGenError("An output named '" + name + "' already exists on node '" + _name + "'");
    }

    ShaderOutputPtr output = std::make_shared<ShaderOutput>();
    output->name = name;
    output->variable = name;
    output->type = type;
    output->node = this;
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

} // namespace MaterialX
