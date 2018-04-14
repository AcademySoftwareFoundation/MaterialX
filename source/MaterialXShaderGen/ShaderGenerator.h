#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Factory.h>

#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

namespace MaterialX
{

using ShaderGeneratorPtr = shared_ptr<class ShaderGenerator>;

/// An argument is a pair of strings holding the 'type' and 'name' of the argument.
using Argument = std::pair<string, string>;
using Arguments = vector<Argument>;

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

    /// Emit function definitions for all nodes
    virtual void emitFunctionDefinitions(Shader& shader);

    /// Emit all functon calls constructing the shader body
    virtual void emitFunctionCalls(Shader& shader);

    /// Emit the final output expression
    virtual void emitFinalOutput(Shader& shader) const;

    /// Emit a shader uniform input variable
    virtual void emitUniform(const Shader::Variable& uniform, Shader& shader);

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(const SgInput* input, Shader& shader) const;

    /// Emit the output variable name for an output, optionally including it's type
    virtual void emitOutput(const SgOutput* output, bool includeType, Shader& shader) const;

    /// Get the variable name to use for an input
    virtual string getVariableName(const SgInput* input) const;

    /// Get the variable name to use for an output
    virtual string getVariableName(const SgOutput* output) const;

    /// Query the shader generator if it wants any extra arguments added when 
    /// emiting the function for the given node.
    virtual const Arguments* getExtraArguments(const SgNode& node) const;

    /// Return the v-direction used by the target system
    virtual Shader::VDirection getTargetVDirection() const;

    /// Return the syntax object for the language used by the code generator
    SyntaxPtr getSyntax() const { return _syntax; }

    template<class T>
    using CreatorFunc = shared_ptr<T>(*)();

    /// Register a shader gen implementation for a given implementation element name
    void registerImplementation(const string& name, CreatorFunc<SgImplementation> creator);

    /// Determine if an implementation has been registered for a given implementation element name
    bool implementationRegistered(const string& name) const;

    /// Return a registered shader gen implementation given an implementation element.
    /// The element must be an Implementation or a NodeGraph acting as implementation.
    /// If no registered implementation is found a 'default' implementation instance 
    /// will be returned, as created by the 
    SgImplementationPtr getImplementation(InterfaceElementPtr element);

    /// Add to the search path used for finding source code.
    void registerSourceCodeSearchPath(const FilePath& path);

    /// Resolve a source code file using the registered search paths.
    FilePath findSourceCode(const FilePath& filename);

    /// Get the source code search path
    const FileSearchPath& sourceCodeSearchPath()
    {
        return _sourceCodeSearchPath;
    }

protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax);

    /// Create a default implementation which is the implementation class to use 
    /// for nodes that has no specific implementation registered for it.
    /// Derived classes can override this to use custom default implementations.
    virtual SgImplementationPtr createDefaultImplementation(ImplementationPtr impl);

    /// Create a compound implementation which is the implementation class to use
    /// for nodes using a nodegraph as their implementation.
    /// Derived classes can override this to use custom compound implementations.
    virtual SgImplementationPtr createCompoundImplementation(NodeGraphPtr impl);

    SyntaxPtr _syntax;
    Factory<SgImplementation> _implFactory;
    std::unordered_map<string, SgImplementationPtr> _cachedImpls;

    FileSearchPath _sourceCodeSearchPath;
};

} // namespace MaterialX

#endif
