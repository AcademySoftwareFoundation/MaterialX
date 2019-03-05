#ifndef MATERIALX_SHADERNODEIMPL_H
#define MATERIALX_SHADERNODEIMPL_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class Shader;
class ShaderStage;
class ShaderGenerator;
class ShaderNode;
class ShaderGraph;
class GenContext;
class GenOptions;
class ShaderInput;
class ShaderOutput;
using ShaderGraphInputSocket = ShaderOutput;

using ShaderNodeImplPtr = shared_ptr<class ShaderNodeImpl>;

/// Class handling the shader generation implementation for a node.
/// Responsible for emitting the function definition and function call 
/// that is the node implementation.
class ShaderNodeImpl
{
  public:
    virtual ~ShaderNodeImpl() {}

    /// Return an identifyer for the language used by this implementation.
    /// By default an empty string is returned, representing any language.
    /// Only override this method if your derived node implementation class
    /// is for a specific language.
    virtual const string& getLanguage() const { return EMPTY_STRING; }

    /// Return an identifyer for the target used by this implementation.
    /// By default an empty string is returned, representing all targets.
    /// Only override this method if your derived node implementation class
    /// is for a specific target.
    virtual const string& getTarget() const { return EMPTY_STRING; }

    /// Initialize with the given implementation element.
    virtual void initialize(ElementPtr implementation, GenContext& context);

    /// Create shader variables needed for the implementation of this node (e.g. uniforms, inputs and outputs).
    /// Used if the node requires input data from the application.
    virtual void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const;

    /// Emit function definition for the given node instance.
    virtual void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    /// Emit the function call or inline source code for given node instance in the given context.
    virtual void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    /// Return a pointer to the graph if this implementation is using a graph,
    /// or returns nullptr otherwise.
    virtual ShaderGraph* getGraph() const;

    /// Return a unique hash for this implementation.
    virtual size_t getHash() const;

    /// Returns true if an input is editable by users.
    /// Editable inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    /// By default all inputs are considered to be editable.
    virtual bool isEditable(const ShaderInput& /*input*/) const
    {
        return true;
    }

    /// Returns true if a graph input is accessible by users.
    /// Accessible inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    /// By default all graph inputs are considered to be acessible.
    virtual bool isEditable(const ShaderGraphInputSocket& /*input*/) const
    {
        return true;
    }

  protected:
    /// Protected constructor
    ShaderNodeImpl() {}
};

} // namespace MaterialX

#endif
