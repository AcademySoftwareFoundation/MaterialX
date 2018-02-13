#ifndef MATERIALX_SGIMPLEMENTATION_H
#define MATERIALX_SGIMPLEMENTATION_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class Implementation;
class SgNode;
class SgNodeGraph;
class Shader;
class ShaderGenerator;

using SgImplementationPtr = shared_ptr<class SgImplementation>;

/// Class handling the shader generation implementation for a node.
/// Responsible for emitting the function definition and function call 
/// that is the node implementation.
class SgImplementation
{
public:
    virtual ~SgImplementation() {}

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
    virtual void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit function definition for the given node instance.
    virtual void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit the function call or inline source code for given node instance.
    virtual void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Return true if this implementation for the given node instance is transparent.
    /// False is returned by default. Only override this if your node represents
    /// a surface shader with transparency.
    virtual bool isTransparent(const SgNode& node) const;

    /// Return a pointer to the node graph if this implementation is using a graph,
    /// or returns nullptr otherwise.
    virtual SgNodeGraph* getNodeGraph() const;

protected:
    /// Protected constructor
    SgImplementation() {}
};

} // namespace MaterialX

#endif
