//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

/// @file
/// Base shader generator class

#include <MaterialXGenShader/Library.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Syntax.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

/// @class ShaderGenerator
/// Base class for shader generators
/// All third-party shader generators should derive from this class.
/// Derived classes should use DECLARE_SHADER_GENERATOR / DEFINE_SHADER_GENERATOR
/// in their declaration / definition, and register with the Registry class.
class ShaderGenerator
{
  public:
    /// Destructor
    virtual ~ShaderGenerator() { }

    /// Return a unique identifier for the language used by this generator
    virtual const string& getLanguage() const = 0;

    /// Return a unique identifier for the target this generator is for
    virtual const string& getTarget() const = 0;

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    virtual ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const = 0;

    /// Start a new scope using the given bracket type.
    virtual void emitScopeBegin(ShaderStage& stage, Syntax::Punctuation punc = Syntax::CURLY_BRACKETS) const;

    /// End the current scope.
    virtual void emitScopeEnd(ShaderStage& stage, bool semicolon = false, bool newline = true) const;

    /// Start a new line.
    virtual void emitLineBegin(ShaderStage& stage) const;

    /// End the current line.
    virtual void emitLineEnd(ShaderStage& stage, bool semicolon = true) const;

    /// Add a line break.
    virtual void emitLineBreak(ShaderStage& stage) const;

    /// Add a string.
    virtual void emitString(const string& str, ShaderStage& stage) const;

    /// Add a single line of code, optionally appending a semicolon.
    virtual void emitLine(const string& str, ShaderStage& stage, bool semicolon = true) const;

    /// Add a single line code comment.
    virtual void emitComment(const string& str, ShaderStage& stage) const;

    /// Add a block of code.
    virtual void emitBlock(const string& str, GenContext& context, ShaderStage& stage) const;

    /// Add the contents of an include file. Making sure it is 
    /// only included once for the shader stage.
    virtual void emitInclude(const string& file, GenContext& context, ShaderStage& stage) const;

    /// Add a value.
    template<typename T>
    void emitValue(const T& value, ShaderStage& stage) const
    {
        stage.addValue<T>(value);
    }

    /// Add the function definition for a single node.
    virtual void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    /// Add the function call for a single node.
    virtual void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage,
                                  bool checkScope = true) const;

    /// Add all function definitions for a graph.
    virtual void emitFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Add all function calls for a graph.
    virtual void emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Emit type definitions for all data types that needs it.
    virtual void emitTypeDefinitions(GenContext& context, ShaderStage& stage) const;

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const;

    /// Emit the output variable name for an output, optionally including it's type
    /// and default value assignment.
    virtual void emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const;

    /// Emit definitions for all shader variables in a block.
    /// @param block Block to emit.
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// @param separator Separator to use between the declarations.
    /// @param context Context for generation.
    /// @param stage The stage to emit code into.
    /// @param assignValue If true the variables are initialized with their value.
    virtual void emitVariableDeclarations(const VariableBlock& block, const string& qualifier, const string& separator, GenContext& context, ShaderStage& stage,
                                          bool assignValue = true) const;

    /// Emit definition of a single shader variable.
    /// @param variable Shader port representing the variable.
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// @param context Context for generation.
    /// @param stage The stage to emit code into.
    /// @param assignValue If true the variable is initialized with its value.
    virtual void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                         bool assignValue = true, unsigned int index = 0) const;

    /// Return the result of an upstream connection or value for an input.
    virtual string getUpstreamResult(const ShaderInput* input, GenContext& context) const;

    /// Reset identifiers in use for the given context.
    virtual void resetIdentifiers(GenContext& context) const;

    /// Return the syntax object for the language used by the code generator
    const Syntax& getSyntax() const { return *_syntax; }

    /// Register a shader node implementation for a given implementation element name
    void registerImplementation(const string& name, CreatorFunction<ShaderNodeImpl> creator);

    /// Determine if a shader node implementation has been registered for a given implementation element name
    bool implementationRegistered(const string& name) const;

    /// Sets the color management system
    void setColorManagementSystem(ColorManagementSystemPtr colorManagementSystem)
    {
        _colorManagementSystem = colorManagementSystem;
    }

    /// Returns the color management system
    ColorManagementSystemPtr getColorManagementSystem() const
    {
        return _colorManagementSystem;
    }

    /// Return a registered shader node implementation given an implementation element.
    /// The element must be an Implementation or a NodeGraph acting as implementation.
    /// If no registered implementation is found a 'default' implementation instance
    /// will be returned, as defined by the createDefaultImplementation method.
    ShaderNodeImplPtr getImplementation(const InterfaceElement& element, GenContext& context) const;

    /// Given an input specification attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be converted to a different type than the input.
    /// @param input Nodedef input potentially holding an enum definition.
    /// @param value The value string to remap.
    /// @param result Enumeration type and value (returned).
    /// @return Return true if the remapping was successful.
    virtual bool remapEnumeration(const ValueElement& input, const string& value,
                                  std::pair<const TypeDesc*, ValuePtr>& result) const;

  protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax);

    /// Create a new stage in a shader.
    virtual ShaderStagePtr createStage(const string& name, Shader& shader) const;

    /// Create a source code implementation which is the implementation class to use
    /// for nodes that has no specific C++ implementation registered for it.
    /// Derived classes can override this to use custom source code implementations.
    virtual ShaderNodeImplPtr createSourceCodeImplementation(const Implementation& impl) const;

    /// Create a compound implementation which is the implementation class to use
    /// for nodes using a nodegraph as their implementation.
    /// Derived classes can override this to use custom compound implementations.
    virtual ShaderNodeImplPtr createCompoundImplementation(const NodeGraph& impl) const;

    /// Set function name for a stage.
    void setFunctionName(const string& functionName, ShaderStage& stage) const
    {
        stage.setFunctionName(functionName);
    }

    /// Replace tokens with identifiers according to the given substitutions map.
    void replaceTokens(const StringMap& substitutions, ShaderStage& stage) const;

  protected:
    static const string SEMICOLON;
    static const string COMMA;

    SyntaxPtr _syntax;
    Factory<ShaderNodeImpl> _implFactory;
    ColorManagementSystemPtr _colorManagementSystem;
    StringMap _tokenSubstitutions;
};

} // namespace MaterialX

#endif // MATERIALX_SHADERGENERATOR_H
