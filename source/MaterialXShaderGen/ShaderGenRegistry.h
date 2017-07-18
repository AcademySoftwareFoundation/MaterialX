#ifndef MATERIALX_SHADERGENREGISTRY_H
#define MATERIALX_SHADERGENREGISTRY_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

#include <MaterialXShaderGen/Factory.h>

namespace MaterialX
{

using ShaderGeneratorPtr = shared_ptr<class ShaderGenerator>;
using NodeImplementationPtr = shared_ptr<class NodeImplementation>;

/// Registry class for handling shader generators and node implementations.
/// Shader generators and implementations can be registered for a specific shading 
/// language and target application/renderer.
/// 3rd party shader generators and node implementations must register with this
/// class in order for the system to find them.
class ShaderGenRegistry
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

    /// Add to the search path used for finding source code.
    static void registerSourceCodeSearchPath(const FilePath& path);

    /// Resolve a source code file using the registered search paths.
    static FilePath findSourceCode(const FilePath& filename);

    /// Register all built-in shader generators and implementations.
    /// Should be called at application initialization.
    static void registerBuiltIn();

    /// Register all built-in shader generators and implementations.
    /// Should be called at application deinitialization.
    static void unregisterBuiltIn();

private:
    static ShaderGeneratorPtr findShaderGeneratorById(const string& id);
    static NodeImplementationPtr findNodeImplementationById(const string& id);
    
    static Factory<ShaderGenerator> _shaderGeneratorFactory;
    static unordered_map<string, ShaderGeneratorPtr> _shaderGenerators;

    static Factory<NodeImplementation> _implementationFactory;
    static unordered_map<string, NodeImplementationPtr> _implementations;

    static FileSearchPath _sourceCodeSearchPath;
};


/// @class @ScopedShaderGenInit
/// An RAII class for registration of built-in ShaderGen classes.
///
/// A ScopedShaderGenInit instance calls ShaderGenRegistry::registerBuiltIn when created,
/// and ShaderGenRegistry::unregisterBuiltIn when destroyed.
class ScopedShaderGenInit
{
public:
    ScopedShaderGenInit()
    {
        ShaderGenRegistry::registerBuiltIn();
    }
    ~ScopedShaderGenInit()
    {
        ShaderGenRegistry::unregisterBuiltIn();
    }
};

} // namespace MaterialX

#endif
