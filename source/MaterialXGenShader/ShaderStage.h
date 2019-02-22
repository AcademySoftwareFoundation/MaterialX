#ifndef MATERIALX_SHADERSTAGE_H
#define MATERIALX_SHADERSTAGE_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Node.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenOptions.h>

#include <queue>
#include <sstream>
#include <unordered_map>

/// Macro for being/end of statements to be picked up by a given shader stage.
/// For shaders that are multi-stage all code generation statements adding code 
/// to the shader should be wrapped inside such begin/end stating its target.
#define BEGIN_SHADER_STAGE(stage, name) if (stage.getName() == name) {
#define END_SHADER_STAGE(stage, name) }

namespace MaterialX
{

/// Identifier for main shader stage.
extern const string MAIN_STAGE;

class Variable;
using VariablePtr = std::shared_ptr<Variable>;

class VariableBlock;
using VariableBlockPtr = std::shared_ptr<VariableBlock>;
using VariableBlockMap = std::unordered_map<string, VariableBlockPtr>;

/// A variables in a shader stage
class Variable
{
  public:
    Variable(const VariableBlock* block, const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING,
        ValuePtr value = nullptr, const string& path = EMPTY_STRING);

    void setType(const TypeDesc* type) { _type = type; }
    const TypeDesc* getType() const { return _type; }

    void setName(const string& name) { _name = name; }
    const string& getName() const { return _name; }

    void setFullName(const string& fullName) { _fullName = fullName; }
    const string& getFullName() const { return _fullName; }

    void setSemantic(const string& semantic) { _semantic = semantic; }
    const string& getSemantic() const { return _semantic; }

    void setValue(ValuePtr value) { _value = value; }
    ValuePtr getValue() const { return _value; }

    void setPath(const string& path) { _path = path; }
    const string& getPath() const { return _path; }

    void setCalculated() { _calculated = true; }
    bool isCalculated() const { return _calculated; }

  private:
    const TypeDesc* _type;
    string _name;
    string _fullName;
    string _semantic;
    ValuePtr _value;
    string _path;
    bool _calculated;
};

/// A block of variables in a shader stage
class VariableBlock
{
  public:
    VariableBlock(const string& name, const string& instance) 
        : _name(name)
        , _instance(instance)
    {}

    const string& getName() const { return _name; }

    const string& getInstance() const { return _instance; }

    bool empty() const { return _variableOrder.empty(); }

    size_t size() const { return _variableOrder.size(); }

    Variable& operator[](size_t i) { return *_variableOrder[i]; }

    const Variable& operator[](size_t i) const { return *_variableOrder[i]; }

    Variable& operator[](const string& name);

    const Variable& operator[](const string& name) const;

    Variable* find(const string& name);

    const Variable* find(const string& name) const;

    void add(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING,
        ValuePtr value = nullptr, const string& path = EMPTY_STRING);

  private:
    string _name;
    string _instance;
    std::unordered_map<string, VariablePtr> _variableMap;
    vector<Variable*> _variableOrder;
};


/// A shader stage, containing the state and 
/// resulting source code for the stage.
class ShaderStage
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

    ShaderStage(const string& name, ConstSyntaxPtr syntax);

    /// Return the stage name.
    const string& getName() const { return _name; }

    /// Return the stage source code.
    const string& getSourceCode() const { return _code; }

    /// Create a new uniform variable block.
    VariableBlockPtr createUniformBlock(const string& name, const string& instance = EMPTY_STRING);

    /// Create a new input variable block.
    VariableBlockPtr createInputBlock(const string& name, const string& instance = EMPTY_STRING);

    /// Create a new output variable block.
    VariableBlockPtr createOutputBlock(const string& name, const string& instance = EMPTY_STRING);

    /// Return the uniform variable block with given name.
    VariableBlock& getUniformBlock(const string& name);
    const VariableBlock& getUniformBlock(const string& name) const;

    /// Return the input variable block with given name.
    VariableBlock& getInputBlock(const string& name);
    const VariableBlock& getInputBlock(const string& name) const;

    /// Return the output variable block with given name.
    VariableBlock& getOutputBlock(const string& name);
    const VariableBlock& getOutputBlock(const string& name) const;

    /// Return the constant variable block.
    VariableBlock& getConstantBlock();
    const VariableBlock& getConstantBlock() const;

    /// Return a map of all uniform blocks.
    const VariableBlockMap& getUniformBlocks() const
    {
        return _uniforms;
    }

    /// Return a map of all input blocks.
    const VariableBlockMap& getInputBlocks() const
    {
        return _inputs;
    }

    /// Return a map of all output blocks.
    const VariableBlockMap& getOutputBlocks() const
    {
        return _outputs;
    }
 
protected:
    /// Start a new scope using the given bracket type.
    void beginScope(Brackets brackets = Brackets::BRACES);

    /// End the current scope.
    void endScope(bool semicolon = false, bool newline = true);

    /// Start a new line.
    void beginLine();

    /// End the current line.
    void endLine(bool semicolon = true);

    /// Add a newline character.
    void newLine();

    /// Add a string.
    void addString(const string& str);

    /// Add a single line of code, optionally appening a semi-colon.
    void addLine(const string& str, bool semicolon = true);

    /// Add a single line code comment.
    void addComment(const string& str);

    /// Add a block of code.
    void addBlock(const string& str, GenContext& context);

    /// Add the contents of an include file. Making sure it is 
    /// only included once for the shader stage.
    void addInclude(const string& file, GenContext& context);

    /// Add a value.
    template<typename T>
    void addValue(const T& value)
    {
        std::stringstream str;
        str << value;
        _code += str.str();
    }

    /// Add the function definition for a node.
    void addFunctionDefinition(const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context);

    friend class ShaderGenerator;

private:
    /// Name of the stage
    const string _name;

    /// Syntax for the type of shader to generate.
    ConstSyntaxPtr _syntax;

    /// Current indentation level.
    int _indentations;

    /// Current scope.
    std::queue<Brackets> _scopes;

    /// Set of include files that has been included.
    std::set<string> _includes;

    /// Set of hash ID's for functions that has been defined.
    std::set<size_t> _definedFunctions;

    /// Block holding constant variables for this stage.
    VariableBlock _constants;

    /// Map of blocks holding uniform variables for this stage.
    VariableBlockMap _uniforms;

    /// Map of blocks holding input variables for this stage.
    VariableBlockMap _inputs;

    /// Map of blocks holding output variables for this stage.
    VariableBlockMap _outputs;

    /// Resulting source code for this stage.
    string _code;
};

using ShaderStagePtr = std::shared_ptr<ShaderStage>;

/// Utility function for adding a variable to a uniform block.
inline void addStageUniform(ShaderStage& stage, const string& block, const TypeDesc* type, const string& name,
    const string& semantic = EMPTY_STRING, ValuePtr value = nullptr, const string& path = EMPTY_STRING)
{
    VariableBlock& uniforms = stage.getUniformBlock(block);
    uniforms.add(type, name, semantic, value, path);
}

/// Utility function for adding a variable to an input block.
inline void addStageInput(ShaderStage& stage, const string& block, const TypeDesc* type, const string& name,
    const string& semantic = EMPTY_STRING, ValuePtr value = nullptr, const string& path = EMPTY_STRING)
{
    VariableBlock& inputs = stage.getInputBlock(block);
    inputs.add(type, name, semantic, value, path);
}

/// Utility function for adding a variable to an output block.
inline void addStageOutput(ShaderStage& stage, const string& block, const TypeDesc* type, const string& name)
{
    VariableBlock& outputs = stage.getOutputBlock(block);
    outputs.add(type, name);
}

/// Utility function for adding a connector block between stages.
inline void addStageConnectorBlock(ShaderStage& from, ShaderStage& to, const string& block, const string& instance)
{
    from.createOutputBlock(block, instance);
    to.createInputBlock(block, instance);
}

/// Utility function for adding a variable to a stage connector block.
inline void addStageConnector(ShaderStage& from, ShaderStage& to, const string& block, const TypeDesc* type, const string& name)
{
    addStageOutput(from, block, type, name);
    addStageInput(to, block, type, name);
}

} // namespace MaterialX

#endif
