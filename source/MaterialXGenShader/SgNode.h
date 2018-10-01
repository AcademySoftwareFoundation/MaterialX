#ifndef MATERIALX_SGNODE_H
#define MATERIALX_SGNODE_H

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/SgImplementation.h>

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
using SgInputSet = std::set<SgInput*>;

/// An input on an SgNode
class SgInput
{
public:
    const TypeDesc* type;
    string name;
    SgNode* node;
    ValuePtr value;
    SgOutput* connection;

    void makeConnection(SgOutput* src);
    void breakConnection();
};

/// An output on an SgNode
class SgOutput
{
public:
    const TypeDesc* type;
    string name;
    SgNode* node;
    ValuePtr value;
    SgInputSet connections;

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
        static const unsigned int BSDF_R      = 1 << 7;  // A BDFS node for reflection
        static const unsigned int BSDF_T      = 1 << 8;  // A BDFS node for transmission
        static const unsigned int EDF         = 1 << 9;  // A EDF node
        static const unsigned int VDF         = 1 << 10; // A VDF node 
        // Specific shader types
        static const unsigned int SURFACE     = 1 << 11;  // A surface shader node
        static const unsigned int VOLUME      = 1 << 12; // A volume shader node
        static const unsigned int LIGHT       = 1 << 13; // A light shader node
        // Specific conditional types
        static const unsigned int IFELSE      = 1 << 14; // An if-else statement
        static const unsigned int SWITCH      = 1 << 15; // A switch statement
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

    static const SgNodePtr NONE;

    static const string SXCLASS_ATTRIBUTE;
    static const string CONSTANT;
    static const string IMAGE;
    static const string COMPARE;
    static const string SWITCH;

public:
    /// Constructor.
    SgNode(const string& name);

    /// Create a new node from a nodedef and an option node instance.
    static SgNodePtr create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance = nullptr);

    /// Return true if this node is a nodegraph.
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

    /// Add inputs/outputs
    SgInput* addInput(const string& name, const TypeDesc* type);
    SgOutput* addOutput(const string& name, const TypeDesc* type);

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

    /// Rename inputs/outputs
    void renameInput(const string& name, const string& newName);
    void renameOutput(const string& name, const string& newName);

    /// Add the given contex id to the set of contexts used for this node.
    void addContextID(int id) { _contextIDs.insert(id); }

    /// Return the set of contexts id's for the contexts used for this node.
    const std::set<int>& getContextIDs() const { return _contextIDs; }

protected:
    string _name;
    unsigned int _classification;

    std::unordered_map<string, SgInputPtr> _inputMap;
    vector<SgInput*> _inputOrder;

    std::unordered_map<string, SgOutputPtr> _outputMap;
    vector<SgOutput*> _outputOrder;

    SgImplementationPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const SgNode*> _usedClosures;
    std::set<int> _contextIDs;

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
    SgNodeGraph(const string& name, DocumentPtr document);

    /// Create a new shadergen graph from an element.
    /// Supported elements are outputs and shaderrefs.
    static SgNodeGraphPtr create(const string& name, ElementPtr element, ShaderGenerator& shadergen);

    /// Create a new shadergen graph from a nodegraph.
    static SgNodeGraphPtr create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen);

    /// Return true if this node is a nodegraph.
    bool isNodeGraph() const override { return true; }

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

    /// Add new node
    SgNode* addNode(const Node& node, ShaderGenerator& shadergen);

    /// Add input/output sockets
    SgInputSocket* addInputSocket(const string& name, const TypeDesc* type);
    SgOutputSocket* addOutputSocket(const string& name, const TypeDesc* type);

    /// Rename input/output sockets
    void renameInputSocket(const string& name, const string& newName);
    void renameOutputSocket(const string& name, const string& newName);

protected:
    /// Add input sockets from an interface element (nodedef, nodegraph or node)
    void addInputSockets(const InterfaceElement& elem);

    /// Add output sockets from an interface element (nodedef, nodegraph or node)
    void addOutputSockets(const InterfaceElement& elem);

    /// Traverse from the given root element and add all dependencies upstream.
    /// The traversal is done in the context of a material, if given, to include
    /// bind input elements in the traversal.
    void addUpstreamDependencies(const Element& root, ConstMaterialPtr material, ShaderGenerator& shadergen);

    /// Add a default geometric node and connect to the given input.
    void addDefaultGeomNode(SgInput* input, const GeomProp& geomprop, ShaderGenerator& shadergen);

    /// Add a color transform node and connect to the given output.
    void addColorTransformNode(SgOutput* output, const string& colorTransform, ShaderGenerator& shadergen);

    /// Perform all post-build operations on the graph.
    void finalize(ShaderGenerator& shadergen);

    /// Optimize the graph, removing redundant paths.
    void optimize();

    /// Bypass a node for a particular input and output,
    /// effectively connecting the input's upstream connection
    /// with the output's downstream connections.
    void bypass(SgNode* node, size_t inputIndex, size_t outputIndex = 0);

    /// Sort the nodes in topological order.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    void topologicalSort();

    /// Calculate scopes for all nodes in the graph
    void calculateScopes();

    /// Make sure inputs and outputs on the graph have
    /// valid and unique names to avoid name collisions
    /// during shader generation
    void validateNames(ShaderGenerator& shadergen);

    /// Break all connections on a node
    static void disconnect(SgNode* node);

    DocumentPtr _document;
    std::unordered_map<string, SgNodePtr> _nodeMap;
    std::vector<SgNode*> _nodeOrder;

    // Temporary storage for nodes that require color transformations
    std::unordered_map<SgNode*, string> _colorTransformMap;
};

/// A function argument for node implementation functions.
/// A argument is a pair of strings holding the 'type' and 'name' of the argument.
using Argument = std::pair<string, string>;
using Arguments = vector<Argument>;

using SgNodeContextPtr = std::shared_ptr<class SgNodeContext>;

/// Class representing an implementation context for a node.
///
/// For some shader generators a node might need customization to it's implementation 
/// depending on in which context the node is used. This class handles customizations
/// in the form of adding extra arguments to the node's implementation function as well
/// as a suffix to the function name to distinguish between the functions for different
/// contexts.
///
/// An example of where this is required if for BSDF and EDF nodes for HW targets 
/// where extra arguments are needed to give directions vectors for evaluation. 
/// For BSDF nodes another use-case is to distinguish between evaluation in a direct lighting
/// context and an indirect lighting context where different versions of the nodes' function
/// is required.
/// 
class SgNodeContext
{
public:
    /// Constructor, set the identifier for this context.
    SgNodeContext(int id) : _id(id) {}

    /// Return the identifier for this context.
    int id() const { return _id; }

    /// Add an extra argument to be used for the node function in this context.
    void addArgument(const Argument& arg) { _arguments.push_back(arg); }

    /// Return a list of extra argument to be used for the node function in this context.
    const Arguments& getArguments() const { return _arguments; }

    /// Set a function name suffix to be used for the node function in this context.
    void setFunctionSuffix(const string& suffix) { _functionSuffix = suffix; }

    /// Return the function name suffix to be used for the node function in this context.
    const string& getFunctionSuffix() const { return _functionSuffix; }

private:
    const int _id;
    Arguments _arguments;
    string _functionSuffix;
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
