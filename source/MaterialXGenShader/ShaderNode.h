//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERNODE_H
#define MATERIALX_SHADERNODE_H

/// @file
/// Classes for nodes created during shader generation

#include <MaterialXGenShader/Library.h>

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXCore/Node.h>

namespace MaterialX
{

class ShaderNode;
class ShaderPort;
class ShaderInput;
class ShaderOutput;
class ShaderGraph;

/// Shared pointer to a ShaderPort
using ShaderPortPtr = shared_ptr<class ShaderPort>;
/// Shared pointer to a ShaderInput
using ShaderInputPtr = shared_ptr<class ShaderInput>;
/// Shared pointer to a ShaderOutput
using ShaderOutputPtr = shared_ptr<class ShaderOutput>;
/// Shared pointer to a ShaderNode
using ShaderNodePtr = shared_ptr<class ShaderNode>;
/// Shared pointer to a ShaderInput
using ShaderInputSet = std::set<ShaderInput*>;

/// @class ShaderPort
/// An input or output port on a ShaderNode
class ShaderPort : public std::enable_shared_from_this<ShaderPort>
{
  public:
    /// Flags set on shader ports.
    static const unsigned int EMITTED = 1 << 0;
    static const unsigned int BIND_INPUT = 1 << 1;

    ShaderPort(ShaderNode* node, const TypeDesc* type, const string& name, ValuePtr value = nullptr);

    /// Return a shared pointer instance of this object.
    ShaderPortPtr getSelf()
    {
        return shared_from_this();
    }

    /// Return the node this port belongs to.
    ShaderNode* getNode() { return _node; }

    /// Return the node this port belongs to.
    const ShaderNode* getNode() const { return _node; }

    /// Set the data type for this port.
    void setType(const TypeDesc* type) { _type = type; }

    /// Return the data type for this port.
    const TypeDesc* getType() const { return _type; }

    /// Set the name of this port.
    void setName(const string& name) { _name = name; }

    /// Return the name of this port.
    const string& getName() const { return _name; }

    /// Set the variable name of this port.
    void setVariable(const string& name) { _variable = name; }

    /// Return the variable name of this port.
    const string& getVariable() const { return _variable; }

    /// Set the variable semantic of this port.
    void setSemantic(const string& semantic) { _semantic = semantic; }

    /// Return the variable semantic of this port.
    const string& getSemantic() const { return _semantic; }

    /// Set a value on this port.
    void setValue(ValuePtr value) { _value = value; }

    /// Return the value set on this port.
    ValuePtr getValue() const { return _value; }

    /// Set a unit type for the value on this port.
    void setUnit(const string& unit) { _unit = unit; }

    /// Return the unit type for the value on this port.
    const string& getUnit() const { return _unit; }

    /// Set the path to this port.
    void setPath(const string& path) { _path = path; }

    /// Return the path to this port.
    const string& getPath() const { return _path; }

    /// Set the emitted state on this port to true.
    void setEmitted() { _flags |= EMITTED; }

    /// Return the emitted state of this port.
    bool isEmitted() const { return (_flags & EMITTED) != 0; }

    /// Set the bind input state on this port to true.
    void setBindInput() { _flags |= BIND_INPUT; }

    /// Return the emitted state of this port.
    bool isBindInput() const { return (_flags & BIND_INPUT) != 0; }

    /// Set flags on this port.
    void setFlags(unsigned int flags) { _flags = flags; }

    /// Return flags set on this port.
    unsigned int getFlags() const { return _flags; }

  protected:
    ShaderNode* _node;
    const TypeDesc* _type;
    string _name;
    string _path;
    string _semantic;
    string _variable;
    ValuePtr _value;
    string _unit;
    unsigned int _flags;
};

/// @class ShaderInput
/// An input on a ShaderNode
class ShaderInput : public ShaderPort
{
  public:
    ShaderInput(ShaderNode* node, const TypeDesc* type, const string& name);

    /// Return a connection to an upstream node output,
    /// or nullptr if not connected.
    ShaderOutput* getConnection() { return _connection; }

    /// Return a connection to an upstream node output,
    /// or nullptr if not connected.
    const ShaderOutput* getConnection() const { return _connection; }

    /// Make a connection from the given source output to this input.
    void makeConnection(ShaderOutput* src);

    /// Break the connection to this input.
    void breakConnection();

    /// Set optional channels value
    void setChannels(const string& channels) { _channels = channels; }

    /// Get optional channels value
    const string& getChannels() const { return _channels; }

  protected:
    ShaderOutput* _connection;
    string _channels;
    friend class ShaderOutput;
};

/// @class ShaderOutput
/// An output on a ShaderNode
class ShaderOutput : public ShaderPort
{
  public:
    ShaderOutput(ShaderNode* node, const TypeDesc* type, const string& name);

    /// Return a set of connections to downstream node inputs,
    /// empty if not connected.
    ShaderInputSet& getConnections() { return _connections; }

    /// Return a set of connections to downstream node inputs,
    /// empty if not connected.
    const ShaderInputSet& getConnections() const { return _connections; }

    /// Make a connection from this output to the given input
    void makeConnection(ShaderInput* dst);

    /// Break a connection from this output to the given input
    void breakConnection(ShaderInput* dst);

    /// Break all connections from this output
    void breakConnections();

  protected:
    ShaderInputSet _connections;
    friend class ShaderInput;
};

/// @class ShaderNode
/// Class representing a node in the shader generation DAG
class ShaderNode
{
  public:
    virtual ~ShaderNode() { }

    /// Flags for classifying nodes into different categories.
    class Classification
    {
    public:
        // Node classes
        static const unsigned int TEXTURE     = 1 << 0;  /// Any node that outputs floats, colors, vectors, etc.
        static const unsigned int CLOSURE     = 1 << 1;  /// Any node that represents light integration
        static const unsigned int SHADER      = 1 << 2;  /// Any node that outputs a shader
        // Specific texture node types
        static const unsigned int FILETEXTURE = 1 << 3;  /// A file texture node
        static const unsigned int CONDITIONAL = 1 << 4;  /// A conditional node
        static const unsigned int CONSTANT    = 1 << 5;  /// A constant node
        // Specific closure types
        static const unsigned int BSDF        = 1 << 6;  /// A BDFS node
        static const unsigned int BSDF_R      = 1 << 7;  /// A BDFS node only for reflection
        static const unsigned int BSDF_T      = 1 << 8;  /// A BDFS node only for transmission
        static const unsigned int EDF         = 1 << 9;  /// A EDF node
        static const unsigned int VDF         = 1 << 10; /// A VDF node
        // Specific shader types
        static const unsigned int SURFACE     = 1 << 11; /// A surface shader node
        static const unsigned int VOLUME      = 1 << 12; /// A volume shader node
        static const unsigned int LIGHT       = 1 << 13; /// A light shader node
        // Specific conditional types
        static const unsigned int IFELSE      = 1 << 14; /// An if-else statement
        static const unsigned int SWITCH      = 1 << 15; /// A switch statement
        // Types based on nodegroup
        static const unsigned int SAMPLE2D    = 1 << 16; /// Can be sampled in 2D (uv space)
        static const unsigned int SAMPLE3D    = 1 << 17; /// Can be sampled in 3D (position)
        static const unsigned int CONVOLUTION2D = 1 << 18; /// Performs a convolution in 2D (uv space)
    };

    /// @struct ScopeInfo
    /// Information on source code scope for the node.
    ///
    /// @todo: Refactor the scope handling, using scope id's instead
    ///
    struct ScopeInfo
    {
        enum Type
        {
            UNKNOWN,
            GLOBAL,
            SINGLE,
            MULTIPLE
        };

        ScopeInfo() : type(UNKNOWN), conditionalNode(nullptr), conditionBitmask(0), fullConditionMask(0) {}

        void merge(const ScopeInfo& fromScope);
        void adjustAtConditionalInput(ShaderNode* condNode, int branch, uint32_t fullMask);
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
    static const string TRANSFORM_POINT;
    static const string TRANSFORM_VECTOR;
    static const string TRANSFORM_NORMAL;
    static const string TEXTURE2D_GROUPNAME;
    static const string TEXTURE3D_GROUPNAME;
    static const string PROCEDURAL2D_GROUPNAME;
    static const string PROCEDURAL3D_GROUPNAME;
    static const string CONVOLUTION2D_GROUPNAME;

  public:
    /// Constructor.
    ShaderNode(const ShaderGraph* parent, const string& name);

    /// Create a new node from a nodedef.
    static ShaderNodePtr create(const ShaderGraph* parent, const string& name, const NodeDef& nodeDef, 
                                GenContext& context);

    /// Create a new node from a node implementation.
    static ShaderNodePtr create(const ShaderGraph* parent, const string& name, ShaderNodeImplPtr impl,
                                unsigned int classification = Classification::TEXTURE);

    /// Return true if this node is a graph.
    virtual bool isAGraph() const { return false; }

    /// Return the parent graph that owns this node.
    /// If this node is a root graph it has no parent
    /// and nullptr will be returned.
    const ShaderGraph* getParent() const
    {
        return _parent;
    }

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
    const ShaderNodeImpl& getImplementation() const
    {
        return *_impl;
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
    void setValues(const Node& node, const NodeDef& nodeDef, GenContext& context);

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

  protected:
    const ShaderGraph* _parent;
    string _name;
    unsigned int _classification;

    std::unordered_map<string, ShaderInputPtr> _inputMap;
    vector<ShaderInput*> _inputOrder;

    std::unordered_map<string, ShaderOutputPtr> _outputMap;
    vector<ShaderOutput*> _outputOrder;

    ShaderNodeImplPtr _impl;
    ScopeInfo _scopeInfo;
    std::set<const ShaderNode*> _usedClosures;

    friend class ShaderGraph;
};

} // namespace MaterialX

#endif
