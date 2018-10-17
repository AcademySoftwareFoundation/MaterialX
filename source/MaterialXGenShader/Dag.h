#ifndef MATERIALX_DAG_H
#define MATERIALX_DAG_H

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include <MaterialXGenShader/DagNode.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/GenImplementation.h>

namespace MaterialX
{

/// An internal input socket in a DAG,
/// used for connecting internal nodes to the outside
using DagInputSocket = DagOutput;

/// An internal input socket in a DAG,
/// used for connecting internal nodes to the outside
using DagOutputSocket = DagInput;

using DagPtr = shared_ptr<class Dag>;

/// Class representing a Directed Acyclic Graph for shader generation
class Dag : public DagNode
{
public:
    Dag(const string& name, DocumentPtr document);

    /// Create a new DAG from an element.
    /// Supported elements are outputs and shaderrefs.
    static DagPtr create(const string& name, ElementPtr element, ShaderGenerator& shadergen);

    /// Create a new DAG from a nodegraph.
    static DagPtr create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen);

    /// Return true if this node is a graph.
    bool isAGraph() const override { return true; }

    /// Get an internal node by name
    DagNode* getNode(const string& name);

    /// Get a vector of all nodes in order
    const vector<DagNode*>& getNodes() const { return _nodeOrder; }

    /// Get number of input sockets
    size_t numInputSockets() const { return numOutputs(); }

    /// Get number of output sockets
    size_t numOutputSockets() const { return numInputs(); }

    /// Get socket by index
    DagInputSocket* getInputSocket(size_t index) { return getOutput(index); }
    DagOutputSocket* getOutputSocket(size_t index = 0) { return getInput(index); }
    const DagInputSocket* getInputSocket(size_t index) const { return getOutput(index); }
    const DagOutputSocket* getOutputSocket(size_t index = 0) const { return getInput(index); }

    /// Get socket by name
    DagInputSocket* getInputSocket(const string& name) { return getOutput(name); }
    DagOutputSocket* getOutputSocket(const string& name) { return getInput(name); }
    const DagInputSocket* getInputSocket(const string& name) const { return getOutput(name); }
    const DagOutputSocket* getOutputSocket(const string& name) const { return getInput(name); }

    /// Get vector of sockets
    const vector<DagInputSocket*>& getInputSockets() const { return _outputOrder; }
    const vector<DagOutputSocket*>& getOutputSockets() const { return _inputOrder; }

    /// Add new node
    DagNode* addNode(const Node& node, ShaderGenerator& shadergen);

    /// Add input/output sockets
    DagInputSocket* addInputSocket(const string& name, const TypeDesc* type);
    DagOutputSocket* addOutputSocket(const string& name, const TypeDesc* type);

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
    void addDefaultGeomNode(DagInput* input, const GeomProp& geomprop, ShaderGenerator& shadergen);

    /// Add a color transform node and connect to the given output.
    void addColorTransformNode(DagOutput* output, const string& colorTransform, ShaderGenerator& shadergen);

    /// Perform all post-build operations on the graph.
    void finalize(ShaderGenerator& shadergen);

    /// Optimize the graph, removing redundant paths.
    void optimize();

    /// Bypass a node for a particular input and output,
    /// effectively connecting the input's upstream connection
    /// with the output's downstream connections.
    void bypass(DagNode* node, size_t inputIndex, size_t outputIndex = 0);

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
    static void disconnect(DagNode* node);

    DocumentPtr _document;
    std::unordered_map<string, DagNodePtr> _nodeMap;
    std::vector<DagNode*> _nodeOrder;

    // Temporary storage for nodes that require color transformations
    std::unordered_map<DagNode*, string> _colorTransformMap;
};

} // namespace MaterialX

#endif
