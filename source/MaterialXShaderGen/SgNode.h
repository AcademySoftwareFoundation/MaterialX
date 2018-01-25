#ifndef MATERIALX_SGNODE_H
#define MATERIALX_SGNODE_H

#include <MaterialXCore/Node.h>

#include <MaterialXShaderGen/SgImplementation.h>

#include <set>

namespace MaterialX
{

class SgInput;
class SgOutput;
class SgEdge;
class SgEdgeIterator;
class SgNode;
class SgNodeGraph;

using SgInputPtr = shared_ptr<class SgInput>;
using SgOutputPtr = shared_ptr<class SgOutput>;
using SgNodePtr = shared_ptr<class SgNode>;
using SgNodeGraphPtr = shared_ptr<class SgNodeGraph>;

/// An input on an SgNode
class SgInput
{
public:
    string name;
    string type;
    SgNode* node;
    ValuePtr value;
    SgOutput* connection;
    string channels;
    bool published;

    void makeConnection(SgOutput* src);
    void breakConnection();
};

/// An output on an SgNode
class SgOutput
{
public:
    string name;
    string type;
    SgNode* node;
    ValuePtr value;
    std::set<SgInput*> connections;
    bool published;

    void makeConnection(SgInput* dst);
    void breakConnection(SgInput* dst);
    void breakConnection();

    SgEdgeIterator traverseUpstream();
};

/// Class representing a node setup for shader generation
class SgNode
{
public:
    /// Flags for classifying nodes into different categories.
    class Classification
    {
    public:
        // Node classes
        static const unsigned int TEXTURE     = 1 << 0;  // Any node that outputs floats, colors, vectors, etc.
        static const unsigned int CLOSURE     = 1 << 1;  // Any node that represents light integration
        static const unsigned int SHADER      = 1 << 2;  // Any node that outputs a shader
        // Specific texture node types
        static const unsigned int FILETEXTURE = 1 << 3;  // A file texture node
        static const unsigned int CONDITIONAL = 1 << 4;  // A conditional node
        static const unsigned int CONSTANT    = 1 << 5;  // A constant node
        // Specific closure types
        static const unsigned int BSDF        = 1 << 6;  // A BDFS node 
        static const unsigned int EDF         = 1 << 7;  // A EDF node
        static const unsigned int VDF         = 1 << 8;  // A VDF node 
        // Specific shader types
        static const unsigned int SURFACE     = 1 << 9;  // A surface shader node
        static const unsigned int VOLUME      = 1 << 10; // A volume shader node
        static const unsigned int LIGHT       = 1 << 11; // A light shader node
        // Specific conditional types
        static const unsigned int IFELSE      = 1 << 12; // An if-else statement
        static const unsigned int SWITCH      = 1 << 13; // A switch statement
    };

    /// Information on source code scope for the node.
    ///
    /// TODO: Refactor the scope handling, using scope id's instead
    ///
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
        void adjustAtConditionalInput(SgNode* condNode, int branch, const uint32_t condMask);
        bool usedByBranch(int branchIndex) const { return (conditionBitmask & (1 << branchIndex)) != 0; }

        Type type;
        SgNode* conditionalNode;
        uint32_t conditionBitmask;
        uint32_t fullConditionMask;
    };

public:
    SgNode(const string& name);

    static SgNodePtr creator(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance = nullptr);

    virtual bool isNodeGraph() const { return false; }

    /// Return true if this node matches the given classification.
    bool hasClassification(unsigned int c) const
    {
        return (_classification & c) == c;
    }

    /// Return the name of this node.
    const string& getName() const
    {
        return _name;
    }

    /// Return the implementation used for this node.
    SgImplementation* getImplementation()
    {
        return _impl.get();
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

    /// Get number of inputs/outputs
    size_t numInputs() const { return _inputOrder.size(); }
    size_t numOutputs() const { return _outputOrder.size(); }

    /// Get inputs/outputs by index
    SgInput* getInput(size_t index) { return _inputOrder[index]; }
    SgOutput* getOutput(size_t index = 0) { return _outputOrder[index]; }
    const SgInput* getInput(size_t index) const { return _inputOrder[index]; }
    const SgOutput* getOutput(size_t index = 0) const { return _outputOrder[index]; }

    /// Get inputs/outputs by name
    SgInput* getInput(const string& name);
    SgOutput* getOutput(const string& name);
    const SgInput* getInput(const string& name) const;
    const SgOutput* getOutput(const string& name) const;

    /// Get vector of inputs/outputs
    const vector<SgInput*>& getInputs() const { return _inputOrder; }
    const vector<SgOutput*>& getOutputs() const { return _outputOrder; }

protected:
    SgInput* addInput(const string& name, const string& type);
    SgOutput* addOutput(const string& name, const string& type);

    string _name;
    unsigned int _classification;

    std::unordered_map<string, SgInputPtr> _inputMap;
    vector<SgInput*> _inputOrder;

    std::unordered_map<string, SgOutputPtr> _outputMap;
    vector<SgOutput*> _outputOrder;

    SgImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const SgNode*> _usedClosures;

    friend class SgNodeGraph;
};

/// An internal input socket in a graph,
/// used for connecting internal nodes to the outside
using SgInputSocket = SgOutput;

/// An internal input socket in a graph,
/// used for connecting internal nodes to the outside
using SgOutputSocket = SgInput;

/// Class representing a node graph setup for shader generation
class SgNodeGraph : public SgNode
{
public:
    SgNodeGraph(const string& name);

    static SgNodeGraphPtr creator(const string& name, ElementPtr element, ShaderGenerator& shadergen);
    static SgNodeGraphPtr creator(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen);

    virtual bool isNodeGraph() const { return true; }

    /// Get an internal node by name
    SgNode* getNode(const string& name);

    /// Get a vector of all nodes in order
    const vector<SgNode*>& getNodes() const { return _nodeOrder; }

    /// Get number of sockets
    size_t numInputSockets() const { return numOutputs(); }
    size_t numOutputSockets() const { return numInputs(); }

    /// Get socket by index
    SgInputSocket* getInputSocket(size_t index) { return getOutput(index); }
    SgOutputSocket* getOutputSocket(size_t index = 0) { return getInput(index); }
    const SgInputSocket* getInputSocket(size_t index) const { return getOutput(index); }
    const SgOutputSocket* getOutputSocket(size_t index = 0) const { return getInput(index); }

    /// Get socket by name
    SgInputSocket* getInputSocket(const string& name) { return getOutput(name); }
    SgOutputSocket* getOutputSocket(const string& name) { return getInput(name); }
    const SgInputSocket* getInputSocket(const string& name) const { return getOutput(name); }
    const SgOutputSocket* getOutputSocket(const string& name) const { return getInput(name); }

    /// Get vector of sockets
    const vector<SgInputSocket*>& getInputSockets() const { return _outputOrder; }
    const vector<SgOutputSocket*>& getOutputSockets() const { return _inputOrder; }

protected:
    SgNode* addNode(const Node& node, ShaderGenerator& shadergen);
    SgNode* addNode(const ShaderRef& shaderRef, ShaderGenerator& shadergen);

    SgInputSocket* addInputSocket(const string& name, const string& type);
    SgOutputSocket* addOutputSocket(const string& name, const string& type);

    /// Perform all post-build operations on the graph.
    void finalize();

    /// Optimize the graph, removing redundant paths.
    void optimize();

    /// Sort the nodes in topological order.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    void topologicalSort();

    /// Calculate scopes for all nodes in the graph
    void calculateScopes();

    /// Break all connections on a node
    static void disconnect(SgNode* node);

    std::unordered_map<string, SgNodePtr> _nodeMap;
    std::vector<SgNode*> _nodeOrder;
};

/// An edge returned during SgNode traversal
class SgEdge
{
public:
    SgEdge(SgOutput* up, SgInput* down)
        : upstream(up)
        , downstream(down)
    {}
    SgOutput* upstream;
    SgInput* downstream;
};

/// Iterator class for traversing edges between SgNodes
class SgEdgeIterator
{
public:
    SgEdgeIterator(SgOutput* output);
    ~SgEdgeIterator() { }

    bool operator==(const SgEdgeIterator& rhs) const
    {
        return _upstream == rhs._upstream &&
            _downstream == rhs._downstream &&
            _stack == rhs._stack;
    }
    bool operator!=(const SgEdgeIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current output in the traversal.
    SgEdge operator*() const
    {
        return SgEdge(_upstream, _downstream);
    }

    /// Iterate to the next edge in the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    SgEdgeIterator& operator++();

    SgEdgeIterator& begin()
    {
        return *this;
    }

    static const SgEdgeIterator& end();

private:
    void extendPathUpstream(SgOutput* upstream, SgInput* downstream);
    void returnPathDownstream(SgOutput* upstream);

    SgOutput* _upstream;
    SgInput* _downstream;
    using StackFrame = std::pair<SgOutput*, size_t>;
    std::vector<StackFrame> _stack;
    std::set<SgOutput*> _path;
};

} // namespace MaterialX

#endif
