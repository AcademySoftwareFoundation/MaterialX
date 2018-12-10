#ifndef MATERIALX_SHADERNODE_H
#define MATERIALX_SHADERNODE_H

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>

#include <set>

namespace MaterialX
{

class ShaderNode;
class ShaderInput;
class ShaderOutput;

using ShaderInputPtr = shared_ptr<class ShaderInput>;
using ShaderOutputPtr = shared_ptr<class ShaderOutput>;
using ShaderNodePtr = shared_ptr<class ShaderNode>;
using ShaderInputSet = std::set<ShaderInput*>;

/// An input or output port on a ShaderNode
class ShaderPort
{
  public:
    static const unsigned int VARIABLE_NOT_RENAMABLE = 1 << 0; // Variable should not be automatically named

    /// Port type.
    const TypeDesc* type;

    /// Port name.
    string name;

    // Path to the origin (input/parameter element) for this shader port.
    // Can be used to map client side node inputs to uniforms on the generated shader,
    // if input values change during rendering.
    string path;

    /// Variable name as used in generated code.
    string variable;

    /// Parent node.
    ShaderNode* node;

    /// A value, or nullptr if not assigned.
    ValuePtr value;

    /// Property flags
    unsigned int flags;
};

/// An input on a ShaderNode
class ShaderInput : public ShaderPort
{
  public:  
    /// A connection to an upstream node output, or nullptr if not connected.
    ShaderOutput* connection;

    /// Make a connection from the given source output to this input.
    void makeConnection(ShaderOutput* src);

    /// Break the connection to this input.
    void breakConnection();
};

/// An output on a ShaderNode
class ShaderOutput : public ShaderPort
{
  public:
    /// A set of connections to downstream node inputs, empty if not connected.
    ShaderInputSet connections;

    /// Make a connection from this output to the given input
    void makeConnection(ShaderInput* dst);

    /// Break a connection from this output to the given input
    void breakConnection(ShaderInput* dst);

    /// Break all connections from this output
    void breakConnection();
};

/// Class representing a node in the shader generation DAG
class ShaderNode
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

        static const unsigned int COLOR_SPACE_TRANSFORM = 1 << 19; // Performs color space transformation

        static const unsigned int DO_NOT_OPTIMIZE = 1 << 20; // Flag that this should not be optimized
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
        void adjustAtConditionalInput(ShaderNode* condNode, int branch, const uint32_t condMask);
        bool usedByBranch(int branchIndex) const { return (conditionBitmask & (1 << branchIndex)) != 0; }

        Type type;
        ShaderNode* conditionalNode;
        uint32_t conditionBitmask;
        uint32_t fullConditionMask;
    };

    static const ShaderNodePtr NONE;

    static const string CONSTANT;
    static const string IMAGE;
    static const string COMPARE;
    static const string SWITCH;
    static const string BSDF_R;
    static const string BSDF_T;

  public:
    /// Constructor.
    ShaderNode(const string& name);

    /// Create a new node from a nodedef
    static ShaderNodePtr create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const GenOptions& options);

    /// Create a new color transform node from a ShaderNodeImpl and type.
    static ShaderNodePtr createColorTransformNode(const string& name, ShaderNodeImplPtr shaderImpl, const TypeDesc* type, ShaderGenerator& shadergen);

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
    ShaderNodeImpl* getImplementation()
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
    bool isUsedClosure(const ShaderNode* node) const
    {
        return _usedClosures.count(node) > 0;
    }

    /// Set input values from the given node and nodedef.
    void setValues(const Node& node, const NodeDef& nodeDef, ShaderGenerator& shadergen);

    /// Set input element paths for the given node and nodedef.
    void setPaths(const Node& node, const NodeDef& nodeDef, bool includeNodeDefInputs=true);

    /// Add inputs/outputs
    ShaderInput* addInput(const string& name, const TypeDesc* type);
    ShaderOutput* addOutput(const string& name, const TypeDesc* type);

    /// Get number of inputs/outputs
    size_t numInputs() const { return _inputOrder.size(); }
    size_t numOutputs() const { return _outputOrder.size(); }

    /// Get inputs/outputs by index
    ShaderInput* getInput(size_t index) { return _inputOrder[index]; }
    ShaderOutput* getOutput(size_t index = 0) { return _outputOrder[index]; }
    const ShaderInput* getInput(size_t index) const { return _inputOrder[index]; }
    const ShaderOutput* getOutput(size_t index = 0) const { return _outputOrder[index]; }

    /// Get inputs/outputs by name
    ShaderInput* getInput(const string& name);
    ShaderOutput* getOutput(const string& name);
    const ShaderInput* getInput(const string& name) const;
    const ShaderOutput* getOutput(const string& name) const;

    /// Get vector of inputs/outputs
    const vector<ShaderInput*>& getInputs() const { return _inputOrder; }
    const vector<ShaderOutput*>& getOutputs() const { return _outputOrder; }

    /// Get input which is used for sampling. If there is none
    /// then a null pointer is returned.
    ShaderInput* getSamplingInput() const
    {
        return _samplingInput;
    }

    /// Returns true if an input is editable by users.
    /// Editable inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    bool isEditable(const ShaderInput& input) const
    {
        return (!_impl || _impl->isEditable(input));
    }

    /// Returns true if a graph input is accessible by users.
    /// Editable inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    bool isEditable(const ShaderGraphInputSocket& input) const
    {
        return (!_impl || _impl->isEditable(input));
    }

    /// Add the given contex id to the set of contexts used for this node.
    void addContextID(int id) { _contextIDs.insert(id); }

    /// Return the set of contexts id's for the contexts used for this node.
    const std::set<int>& getContextIDs() const { return _contextIDs; }

  protected:
    string _name;
    unsigned int _classification;

    std::unordered_map<string, ShaderInputPtr> _inputMap;
    vector<ShaderInput*> _inputOrder;

    std::unordered_map<string, ShaderOutputPtr> _outputMap;
    vector<ShaderOutput*> _outputOrder;

    ShaderInput* _samplingInput;

    ShaderNodeImplPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const ShaderNode*> _usedClosures;
    std::set<int> _contextIDs;

    friend class ShaderGraph;
};

} // namespace MaterialX

#endif
