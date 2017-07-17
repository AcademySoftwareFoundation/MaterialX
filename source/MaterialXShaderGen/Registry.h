#ifndef MATERIALX_REGISTRY_H
#define MATERIALX_REGISTRY_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

#include <MaterialXShaderGen/Factory.h>

namespace MaterialX
{

using ShaderGeneratorPtr = shared_ptr<class ShaderGenerator>;
using NodeImplementationPtr = shared_ptr<class NodeImplementation>;

/// Registry class for handling shader generators and custom implementations.
/// Shader generators and implementations can be registered for a specific shading 
/// language and target application/renderer.
/// 3rd party shader generators and custom implementations must register with this
/// class in order for the system to find them.
class Registry
{
public:
    template<class T>
    using CreatorFunc = shared_ptr<T>(*)();

    /// Register a shader generator for the given language and target.
    static void registerShaderGenerator(const string& language, const string& target, CreatorFunc<ShaderGenerator> creator);

    /// Unregister a shader generator for the given language and target.
    static void unregisterShaderGenerator(const string& language, const string& target);

    /// Register an implementation for a node for the given language and target.
    static void registerNodeImplementation(const string& node, const string& language, const string& target, CreatorFunc<NodeImplementation> creator);

    /// Unregister an implementation for a node for the given language and target.
    static void unregisterNodeImplementation(const string& node, const string& language, const string& target);

    /// Find a shader generator for the given language and target. Creating a new instance 
    /// if it's not been created already. Return nullptr if no generator has been registered 
    /// for the given language and target.
    static ShaderGeneratorPtr findShaderGenerator(const string& language, const string& target = EMPTY_STRING);

    /// Find an implementation for a node for the given language and target. Creating a new 
    /// instance if it's not been created already. Return nullptr if no implementation has been 
    /// registered for the given node, language and target.
    static NodeImplementationPtr findNodeImplementation(const string& node, const string& language = EMPTY_STRING, const string& target = EMPTY_STRING);

    /// Register all built-in shader generators and implementations.
    /// Should be called at application initializetion.
    static void registerBuiltIn();

    /// Register all built-in shader generators and implementations.
    /// Should be called at application deinitializetion.
    static void unregisterBuiltIn();

private:
    static ShaderGeneratorPtr findShaderGeneratorById(const string& id);
    static NodeImplementationPtr findNodeImplementationById(const string& id);
    
    static Factory<ShaderGenerator> _shaderGeneratorFactory;
    static unordered_map<string, ShaderGeneratorPtr> _shaderGenerators;

    static Factory<NodeImplementation> _implementationFactory;
    static unordered_map<string, NodeImplementationPtr> _implementations;
};


/// @class @ScopedRegistryInit
/// An RAII class for Registry registration.
///
/// A ScopedRegistryInit instance calls Registry registration when created,
/// and Registry unregistration when destroyed.
class ScopedRegistryInit
{
public:
    ScopedRegistryInit()
    {
        Registry::registerBuiltIn();
    }
    ~ScopedRegistryInit()
    {
        Registry::unregisterBuiltIn();
    }
};

} // namespace MaterialX

#endif
