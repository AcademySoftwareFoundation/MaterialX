#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/Syntax.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

using ShaderGeneratorPtr = shared_ptr<class ShaderGenerator>;

/// Base class for shader generators
/// All 3rd party shader generators should derive from this class.
/// Derived classes should use DECLARE_SHADER_GENERATOR / DEFINE_SHADER_GENERATOR
/// in it's declaration / definition, and register with the Registry class.
class ShaderGenerator
{
public:
    /// Destructor
    virtual ~ShaderGenerator() {}

    /// Return a unique identifyer for the language used by this generator
    virtual const string& getLanguage() const = 0;

    /// Return a unique identifyer for the target this generator is for
    virtual const string& getTarget() const = 0;

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    virtual ShaderPtr generate(const string& shaderName, ElementPtr element) = 0;

    /// Emit typedefs for all data types that needs it
    virtual void emitTypeDefs(Shader& shader);

    /// Emit function source code for all nodes
    virtual void emitFunctions(Shader& shader);

    /// Emit function source code for the given node
    virtual void emitFunction(const SgNode& node, Shader& shader);

    /// Emit the function call (or inline expression) for a node.
    /// extraInputs will optionally hold a vector of named extra inputs
    /// to give in the function call. If used these are given before the 
    /// ordinary node inputs in the function call.
    virtual void emitFunctionCall(const SgNode& node, Shader& shader, vector<string>* extraInputs = nullptr);

    /// Emit the shader body
    virtual void emitShaderBody(Shader& shader);

    /// Emit the final output expression
    virtual void emitFinalOutput(Shader& shader) const;

    /// Emit a shader uniform input variable
    virtual void emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader);

    /// Emit the connected variable name for an input port
    /// or constant value if the port is not connected
    virtual void emitInput(const ValueElement& port, Shader& shader);

    /// Emit the output variable name for a node or node output, optionally including it's type
    virtual void emitOutput(const TypedElement& nodeOrOutput, bool includeType, Shader& shader);

    /// Return the v-direction used by the target system
    virtual Shader::VDirection getTargetVDirection() const;

    /// Return the syntax object for the language used by the code generator
    SyntaxPtr getSyntax() const { return _syntax; }

    /// Get a unique id from the langunage/target combination
    static string id(const string& language, const string& target = EMPTY_STRING);

protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax) : _syntax(syntax) {}

    SyntaxPtr _syntax;
};

} // namespace MaterialX

  /// Macro declaring required members and methods for a shader generator
#define DECLARE_SHADER_GENERATOR(T)                                           \
    public:                                                                   \
        static const string kLanguage;                                        \
        static const string kTarget;                                          \
        static ShaderGeneratorPtr creator() { return std::make_shared<T>(); } \
        const string& getLanguage() const override { return kLanguage; }      \
        const string& getTarget() const override { return kTarget; }          \

  /// Macro defining required members and methods for a shader generator
  /// LNG sets the language string identifier
  /// TRG sets the target string identifier
#define DEFINE_SHADER_GENERATOR(T, LNG, TRG) \
    const string T::kLanguage = LNG;         \
    const string T::kTarget = TRG;           \

#endif
