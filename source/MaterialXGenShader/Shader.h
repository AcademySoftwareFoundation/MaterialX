#ifndef MATERIALX_SHADER_H
#define MATERIALX_SHADER_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Node.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenOptions.h>

#include <queue>
#include <sstream>
#include <unordered_map>

/// Macro for being/end of statements to be picked up by a given shader stage.
/// For shaders that are multi-stage all code generation statements adding code 
/// to the shader should be wrapped inside such begin/end stating its target.
#define BEGIN_SHADER_STAGE(shader, stageId) if (shader.getActiveStage() == stageId) {
#define END_SHADER_STAGE(shader, stageId) }

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

    struct Variable
    {
        const TypeDesc* type;
        string name;
        string semantic;
        ValuePtr value;

        Variable(const TypeDesc* t = nullptr, const string& n = EMPTY_STRING, const string& s = EMPTY_STRING, ValuePtr v = nullptr)
            : type(t)
            , name(n)
            , semantic(s)
            , value(v)
        {}

        void getArraySuffix(string& result) const
        {
            result.clear();
            if (value && value->isA<vector<float>>())
            {
                vector<float> valueArray = value->asA<vector<float>>();
                result = "[" + std::to_string(valueArray.size()) + "]";
            }
            else if (value && value->isA<vector<int>>())
            {
                vector<int> valueArray = value->asA<vector<int>>();
                result = "[" + std::to_string(valueArray.size()) + "]";
            }
        }
    };

    using VariablePtr = std::shared_ptr<Variable>;

    /// A block of variables for a shader stage
    struct VariableBlock
    {
        string name;
        string instance;
        std::unordered_map<string, VariablePtr> variableMap;
        vector<Variable*> variableOrder;

        VariableBlock(const string& n, const string& i) : name(n), instance(i) {}    
        bool empty() const { return variableOrder.empty(); }
    };

    using VariableBlockPtr = std::shared_ptr<VariableBlock>;
    using VariableBlockMap = std::unordered_map<string, VariableBlockPtr>;

    /// Identifier for shader stages. The base class shader has only a single 
    /// pixel shader stage. Derived shader classes can define additional stages.
    static const size_t PIXEL_STAGE = 0;
    static const size_t NUM_STAGES  = 1;

    /// Identifiers for uniform variable blocks. 
    /// Derived classes can define additional blocks.
    ///
    /// Default uniform block for private variables.
    /// For uniforms not visible to user and only set by application.
    static const string PRIVATE_UNIFORMS;
    /// Default uniform block for public variables. 
    /// For uniforms visible in UI and set by users.
    static const string PUBLIC_UNIFORMS;

public:
    /// Constructor
    Shader(const string& name);

    /// Destructor
    virtual ~Shader() {}

    /// Initialize the shader before shader generation.
    /// @param element The root element to generate the shader from. 
    /// @param shadergen The shader generator instance.
    /// @param options Generation options
    virtual void initialize(ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options);

    /// Return the number of shader stages for this shader.
    /// Defaults to a single stage, derived classes can override this.
    virtual size_t numStages() const { return NUM_STAGES; }

    /// Set the active stage that code will be added to
    virtual void setActiveStage(size_t stage) { _activeStage = stage; }

    /// Return the active stage
    virtual size_t getActiveStage() const { return _activeStage; }

    /// Create a new constant variable for a stage.
    virtual void createConstant(size_t stage, const TypeDesc* type, const string& name,
        const string& semantic = EMPTY_STRING, ValuePtr value = nullptr);

    /// Create a new variable block for uniform inputs in a stage.
    virtual void createUniformBlock(size_t stage, const string& block, const string& instance = EMPTY_STRING);

    /// Create a new variable for uniform data in the given block for a stage.
    /// The block must be previously created with createUniformBlock.
    virtual void createUniform(size_t stage, const string& block, const TypeDesc* type, const string& name,
        const string& semantic = EMPTY_STRING, ValuePtr value = nullptr);

    /// Create a new variable for application/geometric data (primvars).
    virtual void createAppData(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING);

    /// Return the block of constant variables for a stage.
    const VariableBlock& getConstantBlock(size_t stage) const;

    /// Return all blocks of uniform variables for a stage.
    const VariableBlockMap& getUniformBlocks(size_t stage) const { return _stages[stage].uniforms; }

    /// Return a specific block of uniform variables for a stage.
    const VariableBlock& getUniformBlock(size_t stage, const string& block) const;

    /// Return the block of application data variables.
    const VariableBlock& getAppDataBlock() const { return _appData; }

    /// Start a new scope in the shader, using the given bracket type
    virtual void beginScope(Brackets brackets = Brackets::BRACES);

    /// End the current scope in the shader
    virtual void endScope(bool semicolon = false, bool newline = true);

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
    virtual void addBlock(const string& str, ShaderGenerator& shadergen);

    /// Add the function definition for a node
    virtual void addFunctionDefinition(ShaderNode* node, ShaderGenerator& shadergen);

    /// Add the function call for a node
    virtual void addFunctionCall(ShaderNode* node, const GenContext& context, ShaderGenerator& shadergen);

    /// Add the contents of an include file
    /// Making sure it is only included once
    /// for a shader stage
    virtual void addInclude(const string& file, ShaderGenerator& shadergen);

    /// Add a single line of code comment to the shader
    virtual void addComment(const string& str);

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

    /// Return the active shader graph.
    ShaderGraph* getGraph() const { return _graphStack.back(); }

    /// Push a new active shader graph.
    /// Used when emitting code for compounds / subgraphs.
    void pushActiveGraph(ShaderGraph* graph) { _graphStack.push_back(graph); }

    /// Reactivate the previously last used shader graph.
    void popActiveGraph() { _graphStack.pop_back(); }

    /// Return true if this shader matches the given classification.
    bool hasClassification(unsigned int c) const { return getGraph()->hasClassification(c); }

    /// Return the default vdirection which is up.
    static VDirection getDefaultVDirection() { return VDirection::UP; }

    /// Return the final shader source code for a given shader stage
    const string& getSourceCode(size_t stage = PIXEL_STAGE) const { return _stages[stage].code; }

protected:

    /// A shader stage, containing the state and 
    /// resulting source code for the stage
    struct Stage
    {
        string name;
        int indentations;
        std::queue<Brackets> scopes;
        std::set<string> includes;
        std::set<ShaderNodeImpl*> definedFunctions;

        // Block holding constant variables for this stage
        VariableBlock constants;

        // Blocks holding uniform variables for this stage
        VariableBlockMap uniforms;

        // Resulting source code for this stage
        string code;

        Stage(const string& n) : name(n), indentations(0), constants("Constants", "cn") {}
    };

    /// Return the currently active stage
    Stage& stage() { return _stages[_activeStage]; }

    /// Add indentation on current line
    virtual void indent();

    /// Return a container with all top level graphs use by this shader.
    virtual void getTopLevelShaderGraphs(ShaderGenerator& shadergen, std::deque<ShaderGraph*>& graphs) const;

    string _name;
    ShaderGraphPtr _rootGraph;
    vector<ShaderGraph*> _graphStack;
    VDirection _vdirection;

    size_t _activeStage;
    vector<Stage> _stages;

    // Block holding application/geometric input variables
    VariableBlock _appData;
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
