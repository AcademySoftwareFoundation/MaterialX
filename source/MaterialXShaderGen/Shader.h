#ifndef MATERIALX_SHADER_H
#define MATERIALX_SHADER_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Node.h>
#include <MaterialXShaderGen/SgNode.h>

#include <queue>
#include <sstream>

namespace MaterialX
{

using ShaderPtr = shared_ptr<class Shader>;

/// Class containing all data needed during shader generation.
/// After generation is completed it will contain the resulting source code
/// emitted by shader generators.
///
/// The class contains a default implementation using a single shader stage.
/// Derived shaders can override this, as well as overriding all methods 
/// that add code to the shader.
///
class Shader
{
public:
    /// Bracket types
    enum class Brackets
    {
        NONE,
        BRACES,
        PARENTHESES,
        SQUARES
    };

    /// 
    enum class VDirection
    {
        UP,
        DOWN
    };

    /// A uniform shader parameter
    using Uniform = pair<string, ParameterPtr>;

    /// A varying shader parameter
    using Varying = pair<string, InputPtr>;

public:
    /// Constructor
    Shader(const string& name);

    /// Destructor
    virtual ~Shader() {}

    /// Initialize the shader before shader generation.
    /// @param node The root element to generate the shader from. 
    /// @param language The shading language identifyer.
    /// @param target The target application identifyer.
    virtual void initialize(ElementPtr element, const string& language, const string& target);

    /// Must be called after shader generation is completed.
    /// Will release resources used during shader generation.
    virtual void finalize();

    /// Return the number of shader stanges for this shader
    /// Defaults to a single stage, derived classes can override this
    virtual size_t numStages() const { return 1; }

    /// Set the active stage that code will be added to
    virtual void setStage(size_t stage) { _activeStage = stage; }

    /// Start a new scope in the shader, using the given bracket type
    virtual void beginScope(Brackets brackets = Brackets::BRACES);

    /// End the current scope in the shader
    virtual void endScope(bool semicolon = false);

    /// Start a new line in the shader
    virtual void beginLine();

    /// End the current line in the shader
    virtual void endLine(bool semicolon = true);

    /// Add a newline character to the shader
    virtual void newLine();

    /// Add a string to the shader
    virtual void addStr(const string& str);

    /// Add a single line of code to the shader,
    /// optionally appening a semi-colon
    virtual void addLine(const string& str, bool semicolon = true);

    /// Add a block of code to the shader
    virtual void addBlock(const string& str);

    /// Add the contents of an include file
    /// Making sure it is only included once
    /// for a shader stage
    virtual void addInclude(const string& file);

    /// Add a value to the shader
    template<typename T>
    void addValue(const T& value)
    {
        std::stringstream str;
        str << value;
        stage().code += str.str();
    }

    /// Return the shader name
    const string& getName() const { return _name; }

    /// Return the optimized node graph created for shader generation.
    const NodeGraphPtr getNodeGraph() const { return _nodeGraph; }

    /// Return the output used for shader generation.
    const OutputPtr getOutput() const { return _output; }

    /// Return a vector of the nodes in the optimized node graph,
    /// given in topological order.
    const vector<SgNode>& getNodes() const { return _nodes;  }

    /// Return the SgNode for the given node pointer.
    SgNode& getNode(const NodePtr& nodePtr);
    const SgNode& getNode(const NodePtr& nodePtr) const { return const_cast<Shader*>(this)->getNode(nodePtr); }

    /// Return the vdirection requested in the current document.
    VDirection getRequestedVDirection() const { return _vdirection; }

    /// 
    void addUniform(const Uniform& u) { _uniforms.push_back(u); }

    /// 
    void addVarying(const Varying& v) { _varyings.push_back(v); }

    /// Return a vector of the final shader uniforms.
    const vector<Uniform>& getUniforms() const { return _uniforms; }

    /// Return a vector of the final shader varyings.
    const vector<Varying>& getVaryings() const { return _varyings; }

    /// Return the final shader source code for a given shader stage
    const string& getSourceCode(size_t stage = 0) const { return _stages[stage].code; }

protected:
    /// A shader stage, containing the state and 
    /// resulting source code for the stage
    struct Stage
    {
        int indentations;
        std::queue<Brackets> scopes;
        set<string> includes;
        string code;
        Stage() : indentations(0) {}
    };

    /// Return the currently active stage
    Stage& stage() { return _stages[_activeStage]; }

    /// Add indentation on current line
    virtual void indent();

    void addDefaultGeometricNodes(NodePtr node, NodeDefPtr nodeDef, NodeGraphPtr parent);

    string _name;
    NodeGraphPtr _nodeGraph;
    OutputPtr _output;
    vector<SgNode> _nodes;
    unordered_map<NodePtr, size_t> _nodeToSgNodeIndex;
    VDirection _vdirection;

    size_t _activeStage;
    vector<Stage> _stages;
    vector<Uniform> _uniforms;
    vector<Varying> _varyings;
};

/// @class @ExceptionShaderGenError
/// An exception that is thrown when shader generation fails.
class ExceptionShaderGenError : public Exception
{
public:
    ExceptionShaderGenError(const string& msg) :
        Exception(msg)
    {
    }

    ExceptionShaderGenError(const ExceptionShaderGenError& e) :
        Exception(e)
    {
    }

    virtual ~ExceptionShaderGenError() throw()
    {
    }
};

} // namespace MaterialX

#endif
