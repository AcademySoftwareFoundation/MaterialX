#ifndef MATERIALX_SHADERIMPLEMENTATION_H
#define MATERIALX_SHADERIMPLEMENTATION_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class Shader;
class ShaderGenerator;
class ShaderNode;
class ShaderGraph;
class GenContext;

using ShaderImplementationPtr = shared_ptr<class ShaderImplementation>;

/// Class handling the shader generation implementation for a node.
/// Responsible for emitting the function definition and function call 
/// that is the node implementation.
class ShaderImplementation
{
  public:
    virtual ~ShaderImplementation() {}

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
    virtual void initialize(ElementPtr implementation, ShaderGenerator& shadergen);

    /// Create shader variables needed for the implementation of this node (e.g. uniforms, inputs and outputs).
    /// Used if the node requires input data from the application.
    virtual void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit function definition for the given node instance.
    virtual void emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit the function call or inline source code for given node instance in the given context.
    virtual void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader);

    /// Return a pointer to the graph if this implementation is using a graph,
    /// or returns nullptr otherwise.
    virtual ShaderGraph* getGraph() const;

  protected:
    /// Protected constructor
    ShaderImplementation() {}
};

} // namespace MaterialX

#endif
