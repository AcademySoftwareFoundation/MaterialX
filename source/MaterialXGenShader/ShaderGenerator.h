#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/SgNode.h>
#include <MaterialXGenShader/SgOptions.h>

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
    virtual ShaderPtr generate(const string& shaderName, ElementPtr element, const SgOptions& options) = 0;

    /// Emit typedefs for all data types that needs it
    virtual void emitTypeDefs(Shader& shader);

    /// Emit function definitions for all nodes
    virtual void emitFunctionDefinitions(Shader& shader);

    /// Emit all functon calls constructing the shader body
    virtual void emitFunctionCalls(const SgNodeContext& context, Shader& shader);

    /// Emit the final output expression
    virtual void emitFinalOutput(Shader& shader) const;

    /// Emit a shader uniform input variable
    virtual void emitUniform(const Shader::Variable& uniform, Shader& shader);

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(const SgNodeContext& context, const SgInput* input, Shader& shader) const;

    /// Emit the output variable name for an output, optionally including it's type
    /// and default value assignment.
    virtual void emitOutput(const SgNodeContext& context, const SgOutput* output, bool includeType, bool assignDefault, Shader& shader) const;

    /// Return the v-direction used by the target system
    virtual Shader::VDirection getTargetVDirection() const;

    /// Return the syntax object for the language used by the code generator
    const Syntax* getSyntax() const { return _syntax.get(); }

    /// Add node contexts id's to the given node to control 
    /// in which contexts this node should be used.
    virtual void addNodeContextIDs(SgNode* node) const;

    /// Return the node context corresponding to the given id,
    /// or nullptr if no such context is found.
    const SgNodeContext* getNodeContext(int id) const;

    template<class T>
    using CreatorFunction = shared_ptr<T>(*)();

    /// Register a shader gen implementation for a given implementation element name
    void registerImplementation(const string& name, CreatorFunction<SgImplementation> creator);

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

public:
    enum NodeContext
    {
        NODE_CONTEXT_DEFAULT = 0
    };

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

    /// Create a new node context with the given id. The context is added to the 
    /// shader generators node context storage and returned.
    SgNodeContextPtr createNodeContext(int id);

    SyntaxPtr _syntax;
    Factory<SgImplementation> _implFactory;
    std::unordered_map<string, SgImplementationPtr> _cachedImpls;

    FileSearchPath _sourceCodeSearchPath;

    std::unordered_map<int, SgNodeContextPtr> _nodeContexts;
    SgNodeContextPtr _defaultNodeContext;
};

} // namespace MaterialX

#endif
