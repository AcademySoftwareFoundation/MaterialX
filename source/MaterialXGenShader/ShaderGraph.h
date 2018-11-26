#ifndef MATERIALX_SHADERGRAPH_H
#define MATERIALX_SHADERGRAPH_H

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

class ShaderGraphEdge;
class ShaderGraphEdgeIterator;

/// An internal input socket in a shader graph,
/// used for connecting internal nodes to the outside
using ShaderGraphInputSocket = ShaderOutput;

/// An internal output socket in a shader graph,
/// used for connecting internal nodes to the outside
using ShaderGraphOutputSocket = ShaderInput;

using ShaderGraphPtr = shared_ptr<class ShaderGraph>;

/// Class representing a graph (DAG) for shader generation
class ShaderGraph : public ShaderNode
{
  public:
    ShaderGraph(const string& name, DocumentPtr document);

    /// Create a new shader graph from an element.
    /// Supported elements are outputs and shaderrefs.
    static ShaderGraphPtr create(const string& name, ElementPtr element, ShaderGenerator& shadergen);

    /// Create a new shader graph from a nodegraph.
    static ShaderGraphPtr create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen);

    /// Return true if this node is a graph.
    bool isAGraph() const override { return true; }

    /// Get an internal node by name
    ShaderNode* getNode(const string& name);

    /// Get a vector of all nodes in order
    const vector<ShaderNode*>& getNodes() const { return _nodeOrder; }

    /// Get number of input sockets
    size_t numInputSockets() const { return numOutputs(); }

    /// Get number of output sockets
    size_t numOutputSockets() const { return numInputs(); }

    /// Get socket by index
    ShaderGraphInputSocket* getInputSocket(size_t index) { return getOutput(index); }
    ShaderGraphOutputSocket* getOutputSocket(size_t index = 0) { return getInput(index); }
    const ShaderGraphInputSocket* getInputSocket(size_t index) const { return getOutput(index); }
    const ShaderGraphOutputSocket* getOutputSocket(size_t index = 0) const { return getInput(index); }

    /// Get socket by name
    ShaderGraphInputSocket* getInputSocket(const string& name) { return getOutput(name); }
    ShaderGraphOutputSocket* getOutputSocket(const string& name) { return getInput(name); }
    const ShaderGraphInputSocket* getInputSocket(const string& name) const { return getOutput(name); }
    const ShaderGraphOutputSocket* getOutputSocket(const string& name) const { return getInput(name); }

    /// Get vector of sockets
    const vector<ShaderGraphInputSocket*>& getInputSockets() const { return _outputOrder; }
    const vector<ShaderGraphOutputSocket*>& getOutputSockets() const { return _inputOrder; }

    /// Add new node
    ShaderNode* addNode(const Node& node, ShaderGenerator& shadergen);

    /// Add input/output sockets
    ShaderGraphInputSocket* addInputSocket(const string& name, const TypeDesc* type);
    ShaderGraphOutputSocket* addOutputSocket(const string& name, const TypeDesc* type);

    /// Rename input/output sockets
    void renameInputSocket(const string& name, const string& newName);
    void renameOutputSocket(const string& name, const string& newName);

    /// Return an iterator for traversal upstream from the given output
    static ShaderGraphEdgeIterator traverseUpstream(ShaderOutput* output);

  protected:
    /// Add input sockets from an interface element (nodedef, nodegraph or node)
    void addInputSockets(const InterfaceElement& elem, ShaderGenerator& shadergen);

    /// Add output sockets from an interface element (nodedef, nodegraph or node)
    void addOutputSockets(const InterfaceElement& elem);

    /// Traverse from the given root element and add all dependencies upstream.
    /// The traversal is done in the context of a material, if given, to include
    /// bind input elements in the traversal.
    void addUpstreamDependencies(const Element& root, ConstMaterialPtr material, ShaderGenerator& shadergen);

    /// Add a default geometric node and connect to the given input.
    void addDefaultGeomNode(ShaderInput* input, const GeomProp& geomprop, ShaderGenerator& shadergen);

    /// Add a color transform node and connect to the given input.
    void addColorTransformNode(ShaderInput* input, const ColorSpaceTransform& transform, ShaderGenerator& shadergen);

    /// Add a color transform node and connect to the given output.
    void addColorTransformNode(ShaderOutput* input, const ColorSpaceTransform& transform, ShaderGenerator& shadergen);

    /// Perform all post-build operations on the graph.
    void finalize(ShaderGenerator& shadergen);

    /// Optimize the graph, removing redundant paths.
    void optimize();

    /// Bypass a node for a particular input and output,
    /// effectively connecting the input's upstream connection
    /// with the output's downstream connections.
    void bypass(ShaderNode* node, size_t inputIndex, size_t outputIndex = 0);

    /// Sort the nodes in topological order.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    void topologicalSort();

    /// Calculate scopes for all nodes in the graph
    void calculateScopes();

    /// Make sure inputs and outputs on the graph have
    /// valid and unique names to avoid name collisions
    /// during shader generation
    void validateNames(ShaderGenerator& shadergen);

    /// Populates the input color transform map if the provided input/parameter
    /// has a color space attribute and has a type of color3 or color4.
    void populateInputColorTransformMap(ColorManagementSystemPtr colorManagementSystem, const Node& node, ShaderNodePtr shaderNode, ValueElementPtr input, const string& targetColorSpace);

    /// Break all connections on a node
    static void disconnect(ShaderNode* node);

    DocumentPtr _document;
    std::unordered_map<string, ShaderNodePtr> _nodeMap;
    std::vector<ShaderNode*> _nodeOrder;

    // Temporary storage for inputs that require color transformations
    std::unordered_map<ShaderInput*, ColorSpaceTransform> _inputColorTransformMap;

    // Temporary storage for outputs that require color transformations
    std::unordered_map<ShaderOutput*, ColorSpaceTransform> _outputColorTransformMap;
};

/// An edge returned during shader graph traversal.
class ShaderGraphEdge
{
  public:
    ShaderGraphEdge(ShaderOutput* up, ShaderInput* down)
        : upstream(up)
        , downstream(down)
    {}
    ShaderOutput* upstream;
    ShaderInput* downstream;
};

/// Iterator class for traversing edges between nodes in a shader graph.
class ShaderGraphEdgeIterator
{
  public:
    ShaderGraphEdgeIterator(ShaderOutput* output);
    ~ShaderGraphEdgeIterator() { }

    bool operator==(const ShaderGraphEdgeIterator& rhs) const
    {
        return _upstream == rhs._upstream &&
            _downstream == rhs._downstream &&
            _stack == rhs._stack;
    }
    bool operator!=(const ShaderGraphEdgeIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current output in the traversal.
    ShaderGraphEdge operator*() const
    {
        return ShaderGraphEdge(_upstream, _downstream);
    }

    /// Iterate to the next edge in the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    ShaderGraphEdgeIterator& operator++();

    /// Return a reference to this iterator to begin traversal
    ShaderGraphEdgeIterator& begin()
    {
        return *this;
    }

    /// Return the end iterator.
    static const ShaderGraphEdgeIterator& end();

  private:
    void extendPathUpstream(ShaderOutput* upstream, ShaderInput* downstream);
    void returnPathDownstream(ShaderOutput* upstream);

    ShaderOutput* _upstream;
    ShaderInput* _downstream;
    using StackFrame = std::pair<ShaderOutput*, size_t>;
    std::vector<StackFrame> _stack;
    std::set<ShaderOutput*> _path;
};

} // namespace MaterialX

#endif
