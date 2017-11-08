#ifndef MATERIALX_SGNODE_H
#define MATERIALX_SGNODE_H

#include <MaterialXCore/Node.h>

#include <MaterialXShaderGen/SgImplementation.h>

#include <stack>

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

class SgInput
{
public:
    string name;
    string type;
    SgNode* node;
    ValuePtr value;
    SgOutput* connection;
    string channels;
    bool isSocket;

    void makeConnection(SgOutput* src);
    void breakConnection(SgOutput* src);
};

class SgOutput
{
public:
    string name;
    string type;
    SgNode* node;
    set<SgInput*> connections;
    bool isSocket;

    void makeConnection(SgInput* dst);
    void breakConnection(SgInput* dst);

    SgEdgeIterator traverseUpstream();
};

class SgNode
{
public:
    /// Flags for classifying nodes into different categories.
    class Classification
    {
    public:
        // Node classes
        static const unsigned int TEXTURE = 1 << 0;  // Any node that outputs floats, colors, vectors, etc.
        static const unsigned int CLOSURE = 1 << 1;  // Any node that represents light integration
        static const unsigned int SHADER = 1 << 2;  // Any node that outputs a shader
        // Specific texture node types
        static const unsigned int FILETEXTURE = 1 << 3;  // A file texture node
        static const unsigned int CONDITIONAL = 1 << 4;  // A conditional nodes 
        // Specific closure types
        static const unsigned int BSDF = 1 << 5;  // A BDFS node 
        static const unsigned int EDF = 1 << 6;  // A EDF node
        static const unsigned int VDF = 1 << 7;  // A VDF node 
        // Specific shader types
        static const unsigned int SURFACE = 1 << 8;  // A surface shader node
        static const unsigned int VOLUME = 1 << 9;  // A volume shader node
        static const unsigned int LIGHT = 1 << 10; // A light shader node
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
    SgNode(const string& name);

    static SgNodePtr creator(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance = nullptr);

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

    size_t numInputs() const { return _inputOrder.size(); }
    size_t numOutputs() const { return _outputOrder.size(); }

    SgInput* getInput(size_t i) { return _inputOrder[i]; }
    SgOutput* getOutput(size_t i) { return _outputOrder[i]; }

    SgInputPtr getInput(const string& name);
    SgOutputPtr getOutput(const string& name);

    const SgInput* getInput(const string& name) const;
    const SgOutput* getOutput(const string& name) const;

    SgOutput* getOutput() { return _outputOrder[0]; }
    const SgOutput* getOutput() const { return _outputOrder[0]; }

    const vector<SgInput*>& getInputs() const { return _inputOrder; }
    const vector<SgOutput*>& getOutputs() const { return _outputOrder; }

protected:
    virtual SgInput* addInput(const string& name, const string& type, const string& channels, ValuePtr value = nullptr);
    virtual SgOutput* addOutput(const string& name, const string& type, const string& channels);

    string _name;
    unsigned int _classification;

    unordered_map<string, SgInputPtr> _inputMap;
    vector<SgInput*> _inputOrder;

    unordered_map<string, SgOutputPtr> _outputMap;
    vector<SgOutput*> _outputOrder;

    SgImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    set<const SgNode*> _usedClosures;

    friend class Shader;
};

class SgNodeGraph : public SgNode
{
public:
    SgNodeGraph(const string& name);

    static SgNodeGraphPtr creator(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen);
    static SgNodeGraphPtr creator(const string& name, ElementPtr element, ShaderGenerator& shadergen);

    SgNodePtr getNode(const string& name);

    const vector<SgNode*>& getNodes() const { return _nodeOrder; }

    SgOutput* getInputSocket(const string& name);
    SgInput* getOutputSocket(const string& name);
    const SgOutput* getInputSocket(const string& name) const;
    const SgInput* getOutputSocket(const string& name) const;

    const vector<SgOutput*>& getInputSockets() const { return _inputSocketOrder; }
    const vector<SgInput*>& getOutputSockets() const { return _outputSocketOrder; }

    void flattenSubgraphs();
    void topologicalSort();

protected:
    SgInput* addInput(const string& name, const string& type, const string& channels = EMPTY_STRING, ValuePtr value = nullptr) override;
    SgOutput* addOutput(const string& name, const string& type, const string& channels = EMPTY_STRING) override;

    void finalize();

    unordered_map<string, SgNodePtr> _nodeMap;
    vector<SgNode*> _nodeOrder;

    unordered_map<string, SgOutputPtr> _inputSocketMap;
    vector<SgOutput*> _inputSocketOrder;

    unordered_map<string, SgInputPtr> _outputSocketMap;
    vector<SgInput*> _outputSocketOrder;
};


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
    using StackFrame = pair<SgOutput*, int>;
    vector<StackFrame> _stack;
    set<SgOutput*> _path;
};

} // namespace MaterialX

#endif
