#ifndef MATERIALX_SGNODE_H
#define MATERIALX_SGNODE_H

#include <MaterialXCore/Node.h>

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

class SgInput;
class SgOutput;
class SgNode;
class SgNodeGraph;

using SgInputPtr = shared_ptr<class SgInput>;
using SgOutputPtr = shared_ptr<class SgOutput>;
using SgNodePtr = shared_ptr<class SgNode>;
using SgNodeGraphPtr = shared_ptr<class SgNodeGraph>;

class SgInput
{
public:
    string name;
    ValuePtr value;
    SgOutput* connection;
    SgNode* parent;
};

class SgOutput
{
public:
    string name;
    set<SgInput*> connections;
    SgNode* parent;
};

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
    SgNode();

    static SgNodePtr creator(NodePtr node, ShaderGenerator& shadergen);

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

    void setImplementation(SgImplementationPtr impl)
    {
        _impl = impl;
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

    SgInputPtr getInput(const string& name);
    SgOutputPtr getOutput(const string& name);
    SgOutputPtr getOutput();

    const vector<SgInput*>& getInputs() const { return _inputOrder; }
    const vector<SgOutput*>& getOutputs() const { return _outputOrder; }

    virtual bool isNodeGraph() const { return false; }

protected:
    NodePtr _node;
    NodeDefPtr _nodeDef;

    unordered_map<string, SgInputPtr> _inputMap;
    vector<SgInput*> _inputOrder;

    unordered_map<string, SgOutputPtr> _outputMap;
    vector<SgOutput*> _outputOrder;

    unsigned int _classification;
    SgImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    set<const SgNode*> _usedClosures;

    friend class Shader;
};

class SgNodeGraph : public SgNode
{
public:
    using InternalInputs = unordered_map<string, set<SgInput*>>;
    using InternalOutputs = unordered_map<string, SgOutput*>;

public:
    static SgNodeGraphPtr creator(ElementPtr element, ShaderGenerator& shadergen);

    bool isNodeGraph() const override { return true; }

    SgNodePtr getNode(const string& name);

    const vector<SgNode*>& getNodeOrder() const { return _nodeOrder; }

    const InternalInputs& getInternalInputs() const { return _internalInputs; }

    const InternalOutputs& getInternalOutputs() const { return _internalOutputs; }

    void flattenSubgraphs();

    void topologicalSort();

protected:
    unordered_map<string, SgNodePtr> _nodeMap;
    vector<SgNode*> _nodeOrder;

    InternalInputs _internalInputs;
    InternalOutputs _internalOutputs;
};

} // namespace MaterialX

#endif
