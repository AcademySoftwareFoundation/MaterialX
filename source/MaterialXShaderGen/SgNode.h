#ifndef MATERIALX_SGNODE_H
#define MATERIALX_SGNODE_H

#include <MaterialXCore/Node.h>

#include <MaterialXShaderGen/SgImplementation.h>
#include <set>

namespace MaterialX
{

using SgNodePtr = shared_ptr<class SgNode>;

/// Caches required node data for shader generation.
class SgNode
{
public:
    /// Flags for classifying nodes into different categories.
    class Classification
    {
    public:
        // Node classes
        static const unsigned int TEXTURE = 1 << 0; // Any node that outputs floats, colors, vectors, etc.
        static const unsigned int CLOSURE = 1 << 1; // Any node that represents light integration
        static const unsigned int SHADER  = 1 << 2; // Any node that outputs a shader
        // Specific closure types
        static const unsigned int BSDF    = 1 << 3; // A BDFS node 
        static const unsigned int EDF     = 1 << 4; // A EDF node
        static const unsigned int VDF     = 1 << 5; // A VDF node 
        // Specific shader types
        static const unsigned int SURFACE = 1 << 6; // A surface shader node
        static const unsigned int VOLUME  = 1 << 7; // A volume shader node
        static const unsigned int LIGHT   = 1 << 8; // A light shader node
    };

    /// Information on source code scope for the node.
    struct ScopeInfo
    {
        enum class Type
        {
            UNKNOWN,
            GLOBAL,
            SINGLE,
            MULTIPLE
        };

        ScopeInfo() : type(Type::UNKNOWN), conditionalNode(nullptr), conditionBitmask(0), fullConditionMask(0) {}

        void merge(const ScopeInfo& fromScope);
        void adjustAtConditionalInput(const NodePtr& condNode, int branch, const uint32_t condMask);
        bool usedByBranch(int branchIndex) const { return (conditionBitmask & (1 << branchIndex)) != 0; }

        Type type;
        NodePtr conditionalNode;
        uint32_t conditionBitmask;
        uint32_t fullConditionMask;
    };

public:
    SgNode(NodePtr node, ShaderGenerator& shadergen);

    /// Return true if this node matches the given classification.
    bool hasClassification(unsigned int c) const
    {
        return (_classification & c) == c;
    }

    /// Return the name of this node.
    const string& getName() const
    {
        return _node->getName();
    }

    /// Return a reference to this node.
    const Node& getNode() const
    {
        return *_node;
    }

    /// Return a pointer to this node.
    const NodePtr& getNodePtr() const
    {
        return _node;
    }

    /// Return the nodedef for this node.
    const NodeDef& getNodeDef() const
    {
        return *_nodeDef;
    }

    /// Return a port (input or parameter) with the given name
    /// If the port is not created on the node the port on the
    /// nodedef will be returned instead.
    /// Throws exception if no port is found.
    const ValueElement& getPort(const string& name) const;

    /// Return the implementation used for this node.
    const SgImplementationPtr& getImplementation() const
    {
        return _impl;
    }

    /// Return the scope info for this node.
    ScopeInfo& getScopeInfo()
    {
        return _scopeInfo;
    }

    /// Return the scope info for this node.
    const ScopeInfo& getScopeInfo() const
    {
        return _scopeInfo;
    }

    /// Returns true if this node is only referenced by a conditional.
    bool referencedConditionally() const;

    /// Returns true if the given node is a closure used by this node.
    bool isUsedClosure(const SgNode* node) const
    {
        return _usedClosures.count(node) > 0;
    }

private:
    unsigned int _classification;
    NodePtr _node;
    NodeDefPtr _nodeDef;
    SgImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const SgNode*> _usedClosures;

    friend class Shader;
};


} // namespace MaterialX

#endif
