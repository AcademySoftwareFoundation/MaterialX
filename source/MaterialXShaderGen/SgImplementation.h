#ifndef MATERIALX_SGIMPLEMENTATION_H
#define MATERIALX_SGIMPLEMENTATION_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class Implementation;
class SgNode;
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

    /// Initialize with the given implementation element
    virtual void initialize(const Implementation& implementation);

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

    /// Emit function definition, if needed, for the given node instance
    virtual void emitFunction(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit the function call or inline source code for given node instance.
    /// The varying length arguments can, if needed, be used to emit extra inputs 
    /// to the function. These should be given as raw C strings, and can be variable 
    /// names created by the shader generator or numeric values. If used the arguments 
    /// are given to the function first, before the node's usual inputs and parameters.
    virtual void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...);

    /// Return true if this implementation for the given node instance is transparent.
    /// False is returned by default. Only override this if your node represents
    /// a surface shader with transparency.
    virtual bool isTransparent(const SgNode& node) const;

protected:
    /// Protected constructor
    SgImplementation() {}
};

} // namespace MaterialX

#endif
