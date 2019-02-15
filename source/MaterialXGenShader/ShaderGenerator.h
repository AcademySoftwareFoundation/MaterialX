#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

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
    virtual ShaderPtr generate(const string& name, ElementPtr element, const GenOptions& options) = 0;

    /// Start a new scope using the given bracket type.
    virtual void emitScopeBegin(ShaderStage& stage, ShaderStage::Brackets brackets = ShaderStage::Brackets::BRACES);

    /// End the current scope.
    virtual void emitScopeEnd(ShaderStage& stage, bool semicolon = false, bool newline = true);

    /// Start a new line.
    virtual void emitLineBegin(ShaderStage& stage);

    /// End the current line.
    virtual void emitLineEnd(ShaderStage& stage, bool semicolon = true);

    /// Add a line break.
    virtual void emitLineBreak(ShaderStage& stage);

    /// Add a string.
    virtual void emitString(ShaderStage& stage, const string& str);

    /// Add a single line of code, optionally appening a semi-colon.
    virtual void emitLine(ShaderStage& stage, const string& str, bool semicolon = true);

    /// Add a block of code.
    virtual void emitBlock(ShaderStage& stage, const string& str);

    /// Add the function definition for a node.
    virtual void emitFunctionDefinition(ShaderStage& stage, ShaderNode* node);

    /// Add the function call for a node.
    virtual void emitFunctionCall(ShaderStage& stage, ShaderNode* node, const GenContext& context, 
                                  bool checkScope = true);

    /// Add the contents of an include file. Making sure it is 
    /// only included once for the shader stage.
    virtual void emitInclude(ShaderStage& stage, const string& file);

    /// Add a single line code comment.
    virtual void emitComment(ShaderStage& stage, const string& str);

    /// Add a value.
    template<typename T>
    void emitValue(ShaderStage& stage, const T& value)
    {
        stage.addValue<T>(value);
    }

    /// Emit type definitions for all data types that needs it.
    virtual void emitTypeDefinitions(ShaderStage& stage);

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(ShaderStage& stage, const GenContext& context, const ShaderInput* input) const;

    /// Emit the output variable name for an output, optionally including it's type
    /// and default value assignment.
    virtual void emitOutput(ShaderStage& stage, const GenContext& context, const ShaderOutput* output, 
                            bool includeType, bool assignDefaultValue) const;

    /// Utility to emit a block of either uniform or constant variables
    /// @param block Block to emit.
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// Qualifiers are specified by the syntax for the generator.
    /// @param shader Shader to emit to.
    virtual void emitVariableBlock(ShaderStage& stage, const VariableBlock& block,
        const string& qualifier, const string& separator);

    /// Emit a shader variable
    /// @param variable Variable to emit
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// Qualifiers are specified by the syntax for the generator.
    /// @param shader Shader source to emit output to
    virtual void emitVariable(ShaderStage& stage, const Variable& variable, const string& qualifier);

    /// Utility to emit a shader constant variable
    virtual void emitConstant(ShaderStage& stage, const Variable& variable);

    /// Utility to emit a shader uniform input variable
    virtual void emitUniform(ShaderStage& stage, const Variable& variable);

    /// Push a new active shader graph.
    /// Used when emitting code for compounds / subgraphs.
    void pushActiveGraph(ShaderStage& stage, ShaderGraph* graph) const;

    /// Reactivate the previously last used shader graph.
    void popActiveGraph(ShaderStage& stage) const;

    /// Get the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void getInput(const GenContext& context, const ShaderInput* input, string& result) const;

    /// Return the syntax object for the language used by the code generator
    const Syntax* getSyntax() const { return _syntax.get(); }

    /// Add context id's to the given node to control
    /// in which contexts this node should be used.
    virtual void addContextIDs(ShaderNode* node) const;

    /// Return the context corresponding to the given id,
    /// or nullptr if no such context is found.
    const GenContext* getContext(int id) const;

    template<class T>
    using CreatorFunction = shared_ptr<T>(*)();

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
    ColorManagementSystemPtr getColorManagementSystem()
    {
        return _colorManagementSystem;
    }

    /// Return a registered shader node implementation given an implementation element.
    /// The element must be an Implementation or a NodeGraph acting as implementation.
    /// If no registered implementation is found a 'default' implementation instance
    /// will be returned, as defined by the createDefaultImplementation method.
    ShaderNodeImplPtr getImplementation(InterfaceElementPtr element, const GenOptions& options);

    /// Add to the search path used for finding source code.
    void registerSourceCodeSearchPath(const FilePath& path);

    /// Resolve a source code file using the registered search paths.
    FilePath findSourceCode(const FilePath& filename);

    /// Get the source code search path
    const FileSearchPath& sourceCodeSearchPath()
    {
        return _sourceCodeSearchPath;
    }

    /// Given a input element attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be of a different type than the input value type.
    /// @param input Input value element to test.
    /// @param mappingElement Element which provides enumeration information for mapping.
    /// @param enumerationType Enumeration type description (returned).
    /// @return Enumeration value. Null if no remapping is performed.
    virtual ValuePtr remapEnumeration(const ValueElementPtr& /*input*/, const InterfaceElement& /*mappingElement*/, const TypeDesc*& /*enumerationType*/)
    {
        return nullptr;
    }

    /// Given a input specification (name, value, type) attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be of a different type than the input value type.
    /// which is accepted by the shader generator.
    /// @param inputName Name of input parameter.
    /// @param inputValue Input value to test.
    /// @param inputType Input type.
    /// @param mappingElement Element which provides enumeration information for mapping.
    /// @param enumerationType Enumeration type description (returned).
    /// @return Enumeration value. Null if no remapping is performed.
    virtual ValuePtr remapEnumeration(const string& /*inputName*/, const string& /*inputValue*/, const string& /*inputType*/,
                                      const InterfaceElement& /*mappingElement*/, const TypeDesc*& /*enumerationType*/)
    {
        return nullptr;
    }

    /// Return a cached implementation if used during shader generation
    const ShaderNodeImplPtr getCachedImplementation(const string& name) const
    {
        auto it = _cachedImpls.find(name);
        if (it != _cachedImpls.end())
        {
            return it->second;
        }
        return nullptr;
    }

public:
    /// Identifiers for contexts
    enum Context
    {
        CONTEXT_DEFAULT = 0
    };

    static string SEMICOLON_NEWLINE;
    static string COMMA;

protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax);

    /// Create a new stage in a shader.
    virtual ShaderStagePtr createStage(Shader& shader, const string& name);

    /// Create a default implementation which is the implementation class to use
    /// for nodes that has no specific implementation registered for it.
    /// Derived classes can override this to use custom default implementations.
    virtual ShaderNodeImplPtr createDefaultImplementation(ImplementationPtr impl);

    /// Create a compound implementation which is the implementation class to use
    /// for nodes using a nodegraph as their implementation.
    /// Derived classes can override this to use custom compound implementations.
    virtual ShaderNodeImplPtr createCompoundImplementation(NodeGraphPtr impl);

    /// Create a new node context with the given id. The context is added to the
    /// shader generators node context storage and returned.
    GenContextPtr createContext(int id);

    SyntaxPtr _syntax;
    Factory<ShaderNodeImpl> _implFactory;
    std::unordered_map<string, ShaderNodeImplPtr> _cachedImpls;

    FileSearchPath _sourceCodeSearchPath;

    std::unordered_map<int, GenContextPtr> _contexts;
    GenContextPtr _defaultContext;

    ColorManagementSystemPtr _colorManagementSystem;
};

} // namespace MaterialX

#endif
