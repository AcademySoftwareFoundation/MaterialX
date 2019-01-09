#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/GenContext.h>

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
    virtual ShaderPtr generate(const string& shaderName, ElementPtr element, const GenOptions& options) = 0;

    /// Emit type definitions for all data types that needs it.
    virtual void emitTypeDefinitions(Shader& shader);

    /// Emit function definitions for all nodes
    virtual void emitFunctionDefinitions(Shader& shader);

    /// Emit all functon calls constructing the shader body
    virtual void emitFunctionCalls(const GenContext& context, Shader& shader);

    /// Emit the final output expression
    virtual void emitFinalOutput(Shader& shader) const;

    /// Emit a shader constant input variable
    virtual void emitConstant(const Shader::Variable& uniform, Shader& shader);

    /// Emit a shader uniform input variable
    virtual void emitUniform(const Shader::Variable& uniform, Shader& shader);

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(const GenContext& context, const ShaderInput* input, Shader& shader) const;

    /// Get the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void getInput(const GenContext& context, const ShaderInput* input, string& result) const;

    /// Emit the output variable name for an output, optionally including it's type
    /// and default value assignment.
    virtual void emitOutput(const GenContext& context, const ShaderOutput* output, bool includeType, bool assignDefault, Shader& shader) const;

    /// Return the syntax object for the language used by the code generator
    const Syntax* getSyntax() const { return _syntax.get(); }

    /// Add node contexts id's to the given node to control
    /// in which contexts this node should be used.
    virtual void addNodeContextIDs(ShaderNode* node) const;

    /// Return the node context corresponding to the given id,
    /// or nullptr if no such context is found.
    const GenContext* getNodeContext(int id) const;

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

protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax);

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

    /// Utility to emit a block of either uniform or constant variables
    /// @param block Block to emit.
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// Qualifiers are specified by the syntax for the generator.
    /// @param shader Shader to emit to.
    virtual void emitVariableBlock(const Shader::VariableBlock& block, const string& qualifier, Shader& shader);

    /// Emit a shader input variable
    /// @param variable Variable to emit
    /// @param qualifier Optional qualifier to add before the variable declaration.
    /// Qualifiers are specified by the syntax for the generator.
    /// @param shader Shader source to emit output to
    virtual void emitVariable(const Shader::Variable& variable, const string& qualifier, Shader& shader);

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
