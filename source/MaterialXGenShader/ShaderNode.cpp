//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderNode.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

//
// ShaderPort methods
//

ShaderPort::ShaderPort(ShaderNode* node, const TypeDesc* type, const string& name, ValuePtr value) :
    _node(node),
    _type(type),
    _name(name),
    _variable(name),
    _value(value),
    _flags(0)
{
}

//
// ShaderInput methods
//

ShaderInput::ShaderInput(ShaderNode* node, const TypeDesc* type, const string& name) :
    ShaderPort(node, type, name),
    _connection(nullptr)
{
}

void ShaderInput::makeConnection(ShaderOutput* src)
{
    _connection = src;
    src->_connections.insert(this);
}

void ShaderInput::breakConnection()
{
    if (_connection)
    {
        _connection->_connections.erase(this);
        _connection = nullptr;
    }
}

//
// ShaderOutput methods
//

ShaderOutput::ShaderOutput(ShaderNode* node, const TypeDesc* type, const string& name) :
    ShaderPort(node, type, name)
{
}

void ShaderOutput::makeConnection(ShaderInput* dst)
{
    dst->_connection = this;
    _connections.insert(dst);
}

void ShaderOutput::breakConnection(ShaderInput* dst)
{
    _connections.erase(dst);
    dst->_connection = nullptr;
}

void ShaderOutput::breakConnection()
{
    for (ShaderInput* input : _connections)
    {
        input->_connection = nullptr;
    }
    _connections.clear();
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
const string ShaderNode::TRANSFORM_POINT = "transformpoint";
const string ShaderNode::TRANSFORM_VECTOR = "transformvector";
const string ShaderNode::TRANSFORM_NORMAL = "transformnormal";
const string ShaderNode::TEXTURE2D_GROUPNAME = "texture2d";
const string ShaderNode::TEXTURE3D_GROUPNAME = "texture3d";
const string ShaderNode::PROCEDURAL2D_GROUPNAME = "procedural2d";
const string ShaderNode::PROCEDURAL3D_GROUPNAME = "procedural3d";
const string ShaderNode::CONVOLUTION2D_GROUPNAME = "convolution2d";

//
// ShaderNode methods
//

ShaderNode::ShaderNode(const ShaderGraph* parent, const string& name) :
    _parent(parent),
    _name(name),
    _classification(0),
    _impl(nullptr)
{
}

bool ShaderNode::referencedConditionally() const
{
    if (_scopeInfo.type == ShaderNode::ScopeInfo::SINGLE)
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
    if (type == ScopeInfo::GLOBAL || (type == ScopeInfo::SINGLE && conditionBitmask == fullConditionMask))
    {
        type = ScopeInfo::SINGLE;
        conditionalNode = condNode;
        conditionBitmask = 1 << branch;
        fullConditionMask = fullMask;
    }
    else if (type == ScopeInfo::SINGLE)
    {
        type = ScopeInfo::MULTIPLE;
        conditionalNode = nullptr;
    }
}

void ShaderNode::ScopeInfo::merge(const ScopeInfo &fromScope)
{
    if (type == ScopeInfo::UNKNOWN || fromScope.type == ScopeInfo::GLOBAL)
    {
        *this = fromScope;
    }
    else if (type == ScopeInfo::GLOBAL)
    {

    }
    else if (type == ScopeInfo::SINGLE && fromScope.type == ScopeInfo::SINGLE && conditionalNode == fromScope.conditionalNode)
    {
        conditionBitmask |= fromScope.conditionBitmask;

        // This node is needed for all branches so it is no longer conditional
        if (conditionBitmask == fullConditionMask)
        {
            type = ScopeInfo::GLOBAL;
            conditionalNode = nullptr;
        }
    }
    else
    {
        // NOTE: Right now multiple scopes is not really used, it works exactly as ScopeInfo::GLOBAL
        type = ScopeInfo::MULTIPLE;
        conditionalNode = nullptr;
    }
}

ShaderNodePtr ShaderNode::create(const ShaderGraph* parent, const string& name, const NodeDef& nodeDef, GenContext& context)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(parent, name);

    const ShaderGenerator& shadergen = context.getShaderGenerator();

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(shadergen.getTarget(), shadergen.getLanguage());
    if (impl)
    {
        newNode->_impl = shadergen.getImplementation(*impl, context);
    }
    if (!newNode->_impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    // Check for classification based on group name
    unsigned int groupClassification = 0;
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

    // Create interface from nodedef
    const vector<ValueElementPtr> nodeDefPorts = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& port : nodeDefPorts)
    {
        const TypeDesc* portType = TypeDesc::get(port->getType());
        if (port->isA<Output>())
        {
            newNode->addOutput(port->getName(), portType);
        }
        else
        {
            const string& portValue = port->getValueString();
            std::pair<const TypeDesc*, ValuePtr> enumResult;
            if (context.getShaderGenerator().remapEnumeration(*port, portValue, enumResult))
            {
                ShaderInput* input = newNode->addInput(port->getName(), enumResult.first);
                input->setValue(enumResult.second);
            }
            else
            {
                ShaderInput* input = newNode->addInput(port->getName(), portType);
                if (!portValue.empty())
                {
                    input->setValue(port->getValue());
                }
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
    if (primaryOutput->getType() == Type::SURFACESHADER)
    {
        newNode->_classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (primaryOutput->getType() == Type::LIGHTSHADER)
    {
        newNode->_classification = Classification::LIGHT | Classification::SHADER;
    }
    else if (primaryOutput->getType() == Type::BSDF)
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
    else if (primaryOutput->getType() == Type::EDF)
    {
        newNode->_classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (primaryOutput->getType() == Type::VDF)
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

    if(nodeDef.getNodeString() == TRANSFORM_POINT)
    {
        newNode->_classification |= Classification::TRANSFORM_POINT;
    }
    else if(nodeDef.getNodeString() == TRANSFORM_VECTOR)
    {
        newNode->_classification |= Classification::TRANSFORM_VECTOR;
    }
    else if(nodeDef.getNodeString() == TRANSFORM_NORMAL)
    {
        newNode->_classification |= Classification::TRANSFORM_NORMAL;
    }

    // Add in group classification
    newNode->_classification |= groupClassification;

    return newNode;
}

ShaderNodePtr ShaderNode::create(const ShaderGraph* parent, const string& name, ShaderNodeImplPtr impl, unsigned int classification)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(parent, name);
    newNode->_impl = impl;
    newNode->_classification = classification;
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
            input->setPath(nodeValue->getNamePath());
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
        if (input && input->getPath().empty())
        {
            input->setPath(nodePath + NAME_PATH_SEPARATOR + nodeInput->getName());
        }
    }
    const vector<ParameterPtr> nodeParameters = nodeDef.getChildrenOfType<Parameter>();
    for (const ParameterPtr& nodeParameter : nodeParameters)
    {
        ShaderInput* input = getInput(nodeParameter->getName());
        if (input && input->getPath().empty())
        {
            input->setPath(nodePath + NAME_PATH_SEPARATOR + nodeParameter->getName());
        }
    }
}

void ShaderNode::setValues(const Node& node, const NodeDef& nodeDef, GenContext& context)
{
    // Copy input values from the given node
    const vector<ValueElementPtr> nodeValues = node.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& nodeValue : nodeValues)
    {
        ShaderInput* input = getInput(nodeValue->getName());
        ValueElementPtr nodeDefInput = nodeDef.getChildOfType<ValueElement>(nodeValue->getName());
        if (input && nodeDefInput)
        {
            const string& valueString = nodeValue->getValueString();
            std::pair<const TypeDesc*, ValuePtr> enumResult;
            if (context.getShaderGenerator().remapEnumeration(*nodeDefInput, valueString, enumResult))
            {
                input->setValue(enumResult.second);
            }
            else if (!valueString.empty())
            {
                input->setValue(nodeValue->getResolvedValue());
            }
        }
    }
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

    ShaderInputPtr input = std::make_shared<ShaderInput>(this, type, name);
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

    ShaderOutputPtr output = std::make_shared<ShaderOutput>(this, type, name);
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

} // namespace MaterialX
