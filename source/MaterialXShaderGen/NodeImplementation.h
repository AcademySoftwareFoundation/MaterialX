#ifndef MATERIALX_NODEIMPLEMENTATION_H
#define MATERIALX_NODEIMPLEMENTATION_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

class SgNode;
class Shader;
class ShaderGenerator;

using NodePtr = shared_ptr<class Node>;
using NodeImplementationPtr = shared_ptr<class NodeImplementation>;

/// Base class for implementations that require custom code for a shader generator target.
/// This should only be used if using a data driven implementation by shading language code
/// is not possible AND a graph based implementation is not possible.
/// With this class you can emit the needed shading code that implements the node.
/// Derived classes should use the macros DECLARE_NODE_IMPLEMENTATION/DEFINE_NODE_IMPLEMENTATION
/// in it's declaration/definition, and register with the Registry class.
class NodeImplementation
{
public:
    virtual ~NodeImplementation() {}

    /// Return the name of the node the implementation is for
    virtual const string& getNode() const = 0;

    /// Return a unique identifyer for the language used for this implementation
    virtual const string& getLanguage() const = 0;

    /// Return a unique identifyer for the target this implementation is for
    virtual const string& getTarget() const = 0;

    /// Emit function definition, if needed, for the given node instance
    virtual void emitFunction(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Emit the function call, or other node implementation source code, for given node instance
    virtual void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader);

    /// Return true if this node for the given node instance is transparent.
    /// False is returned by default. Only override this if your node represents
    /// a surface shader with transparency.
    virtual bool isTransparent(const NodePtr& node) const;

    /// Get a unique id from the node/langunage/target combination
    static string id(const string& node, const string& language = EMPTY_STRING, const string& target = EMPTY_STRING);

protected:
    /// Protected constructor
    NodeImplementation() {}
};

} // namespace MaterialX

/// Macro declaring required members and methods for an implementation
#define DECLARE_NODE_IMPLEMENTATION(T)                                   \
    public:                                                              \
        static const string kNode;                                       \
        static const string kLanguage;                                   \
        static const string kTarget;                                     \
        static NodeImplementationPtr creator() { return std::make_shared<T>(); } \
        const string& getNode() const override { return kNode; }         \
        const string& getLanguage() const override { return kLanguage; } \
        const string& getTarget() const override { return kTarget; }     \

/// Macro defining required members and methods for an implementation
/// NODE sets the node name string identifier
/// LNG sets the language string identifier
/// TRG sets the target string identifier
#define DEFINE_NODE_IMPLEMENTATION(T, NODE, LNG, TRG) \
    const string T::kNode = NODE;                     \
    const string T::kLanguage = LNG;                  \
    const string T::kTarget = TRG;                    \

#endif
