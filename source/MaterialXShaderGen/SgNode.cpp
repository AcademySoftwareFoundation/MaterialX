#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/NodeImplementation.h>
#include <MaterialXShaderGen/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

bool SgNode::referencedConditionally() const
{
    if (_scopeInfo.type == SgNode::ScopeInfo::Type::SINGLE)
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

void SgNode::ScopeInfo::adjustAtConditionalInput(const NodePtr& condNode, int branch, const uint32_t fullMask)
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

void SgNode::ScopeInfo::merge(const ScopeInfo &fromScope)
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
        // NOTE: Right now multiple scopes is not really used, it works exactly as GLOBAL_SCOPE
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}


SgNode::SgNode(NodePtr node, ShaderGenerator& shadergen)
    : _classification(0)
    , _node(node)
    , _nodeDef(nullptr)
    , _impl(nullptr)
{
    if (!_node)
    {
        return;
    }

    _nodeDef = node->getReferencedNodeDef();
    if (!_nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node->getName() + "'");
    }

    _impl = shadergen.getNodeImplementation(*_nodeDef);

    // Set node classification
    _classification = Classification::TEXTURE;
    if (_nodeDef->getType() == kSURFACE)
    {
        _classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (_nodeDef->getType() == kBSDF)
    {
        _classification = Classification::BSDF | Classification::CLOSURE;
    }
    else if (_nodeDef->getType() == kEDF)
    {
        _classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (_nodeDef->getType() == kVDF)
    {
        _classification = Classification::VDF | Classification::CLOSURE;
    }
}

const ValueElement& SgNode::getPort(const string& name) const
{
    ValueElementPtr port = _node->getChildOfType<ValueElement>(name);
    if (!port)
    {
        port = _nodeDef->getChildOfType<ValueElement>(name);
        if (!port)
        {
            throw ExceptionShaderGenError("Node '" + _node->getName() + "' has no input port named '" + name + "'");
        }
    }
    return *port;
}

} // namespace MaterialX
