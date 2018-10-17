#ifndef MATERIALX_DAGNODE_H
#define MATERIALX_DAGNODE_H

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/GenImplementation.h>

#include <set>

namespace MaterialX
{

class DagInput;
class DagOutput;
class DagEdge;
class DagEdgeIterator;
class DagNode;

using DagInputPtr = shared_ptr<class DagInput>;
using DagOutputPtr = shared_ptr<class DagOutput>;
using DagNodePtr = shared_ptr<class DagNode>;
using DagInputSet = std::set<DagInput*>;

/// An input on a DagNode
class DagInput
{
public:
    const TypeDesc* type;
    string name;
    DagNode* node;
    ValuePtr value;
    DagOutput* connection;

    void makeConnection(DagOutput* src);
    void breakConnection();
};

/// An output on a DagNode
class DagOutput
{
public:
    const TypeDesc* type;
    string name;
    DagNode* node;
    ValuePtr value;
    DagInputSet connections;

    void makeConnection(DagInput* dst);
    void breakConnection(DagInput* dst);
    void breakConnection();

    DagEdgeIterator traverseUpstream();
};

/// Class representing a node in the shader generation DAG
class DagNode
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
        static const unsigned int BSDF_R      = 1 << 7;  // A BDFS node only for reflection
        static const unsigned int BSDF_T      = 1 << 8;  // A BDFS node only for transmission
        static const unsigned int EDF         = 1 << 9;  // A EDF node
        static const unsigned int VDF         = 1 << 10; // A VDF node 
        // Specific shader types
        static const unsigned int SURFACE     = 1 << 11;  // A surface shader node
        static const unsigned int VOLUME      = 1 << 12; // A volume shader node
        static const unsigned int LIGHT       = 1 << 13; // A light shader node
        // Specific conditional types
        static const unsigned int IFELSE      = 1 << 14; // An if-else statement
        static const unsigned int SWITCH      = 1 << 15; // A switch statement
        // Types based on nodegroup
        static const unsigned int SAMPLE2D    = 1 << 16; // Can be sampled in 2D (uv space)
        static const unsigned int SAMPLE3D    = 1 << 17; // Can be sampled in 3D (position)
        static const unsigned int CONVOLUTION2D = 1 << 18; // Performs a convolution in 2D (uv space)
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
        void adjustAtConditionalInput(DagNode* condNode, int branch, const uint32_t condMask);
        bool usedByBranch(int branchIndex) const { return (conditionBitmask & (1 << branchIndex)) != 0; }

        Type type;
        DagNode* conditionalNode;
        uint32_t conditionBitmask;
        uint32_t fullConditionMask;
    };

    static const DagNodePtr NONE;

    static const string SXCLASS_ATTRIBUTE;
    static const string CONSTANT;
    static const string IMAGE;
    static const string COMPARE;
    static const string SWITCH;
    static const string BSDF_R;
    static const string BSDF_T;

public:
    /// Constructor.
    DagNode(const string& name);

    /// Create a new node from a nodedef and an option node instance.
    static DagNodePtr create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance = nullptr);

    /// Return true if this node is a graph.
    virtual bool isAGraph() const { return false; }

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
    GenImplementation* getImplementation()
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
    bool isUsedClosure(const DagNode* node) const
    {
        return _usedClosures.count(node) > 0;
    }

    /// Add inputs/outputs
    DagInput* addInput(const string& name, const TypeDesc* type);
    DagOutput* addOutput(const string& name, const TypeDesc* type);

    /// Get number of inputs/outputs
    size_t numInputs() const { return _inputOrder.size(); }
    size_t numOutputs() const { return _outputOrder.size(); }

    /// Get inputs/outputs by index
    DagInput* getInput(size_t index) { return _inputOrder[index]; }
    DagOutput* getOutput(size_t index = 0) { return _outputOrder[index]; }
    const DagInput* getInput(size_t index) const { return _inputOrder[index]; }
    const DagOutput* getOutput(size_t index = 0) const { return _outputOrder[index]; }

    /// Get inputs/outputs by name
    DagInput* getInput(const string& name);
    DagOutput* getOutput(const string& name);
    const DagInput* getInput(const string& name) const;
    const DagOutput* getOutput(const string& name) const;

    /// Get vector of inputs/outputs
    const vector<DagInput*>& getInputs() const { return _inputOrder; }
    const vector<DagOutput*>& getOutputs() const { return _outputOrder; }

    /// Rename inputs/outputs
    void renameInput(const string& name, const string& newName);
    void renameOutput(const string& name, const string& newName);

    /// Get input which is used for sampling. If there is none
    /// then a null pointer is returned.
    DagInput* getSamplingInput() const
    {
        return _samplingInput;
    }

    /// Add the given contex id to the set of contexts used for this node.
    void addContextID(int id) { _contextIDs.insert(id); }

    /// Return the set of contexts id's for the contexts used for this node.
    const std::set<int>& getContextIDs() const { return _contextIDs; }

protected:
    string _name;
    unsigned int _classification;

    std::unordered_map<string, DagInputPtr> _inputMap;
    vector<DagInput*> _inputOrder;

    std::unordered_map<string, DagOutputPtr> _outputMap;
    vector<DagOutput*> _outputOrder;

    DagInput* _samplingInput;

    GenImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const DagNode*> _usedClosures;
    std::set<int> _contextIDs;

    friend class Dag;
};

/// An edge returned during DagNode traversal
class DagEdge
{
public:
    DagEdge(DagOutput* up, DagInput* down)
        : upstream(up)
        , downstream(down)
    {}
    DagOutput* upstream;
    DagInput* downstream;
};

/// Iterator class for traversing edges between DagNodes
class DagEdgeIterator
{
public:
    DagEdgeIterator(DagOutput* output);
    ~DagEdgeIterator() { }

    bool operator==(const DagEdgeIterator& rhs) const
    {
        return _upstream == rhs._upstream &&
            _downstream == rhs._downstream &&
            _stack == rhs._stack;
    }
    bool operator!=(const DagEdgeIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current output in the traversal.
    DagEdge operator*() const
    {
        return DagEdge(_upstream, _downstream);
    }

    /// Iterate to the next edge in the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    DagEdgeIterator& operator++();

    DagEdgeIterator& begin()
    {
        return *this;
    }

    static const DagEdgeIterator& end();

private:
    void extendPathUpstream(DagOutput* upstream, DagInput* downstream);
    void returnPathDownstream(DagOutput* upstream);

    DagOutput* _upstream;
    DagInput* _downstream;
    using StackFrame = std::pair<DagOutput*, size_t>;
    std::vector<StackFrame> _stack;
    std::set<DagOutput*> _path;
};

} // namespace MaterialX

#endif
