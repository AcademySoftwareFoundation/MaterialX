#ifndef MATERIALX_NODEIMPLEMENTATION_H
#define MATERIALX_NODEIMPLEMENTATION_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class Implementation;
class SgNode;
class Shader;
class ShaderGenerator;

using NodeImplementationPtr = shared_ptr<class NodeImplementation>;

class NodeImplementation
{
public:
    virtual ~NodeImplementation() {}

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

    /// Emit the function call, or other node implementation source code, for given node instance
    virtual void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...);

    /// Return true if this implementation for the given node instance is transparent.
    /// False is returned by default. Only override this if your node represents
    /// a surface shader with transparency.
    virtual bool isTransparent(const SgNode& node) const;

protected:
    /// Protected constructor
    NodeImplementation() {}
};

} // namespace MaterialX

#endif
