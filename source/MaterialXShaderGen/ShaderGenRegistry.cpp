#include <MaterialXShaderGen/ShaderGenRegistry.h>

#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>

#include <MaterialXShaderGen/NodeImplementations/VDirection.h>
#include <MaterialXShaderGen/NodeImplementations/Swizzle.h>
#include <MaterialXShaderGen/NodeImplementations/Switch.h>
#include <MaterialXShaderGen/NodeImplementations/Compare.h>
#include <MaterialXShaderGen/NodeImplementations/Surface.h>

namespace MaterialX
{

Factory<ShaderGenerator> ShaderGenRegistry::_shaderGeneratorFactory;
unordered_map<string, ShaderGeneratorPtr> ShaderGenRegistry::_shaderGenerators;

Factory<NodeImplementation> ShaderGenRegistry::_implementationFactory;
unordered_map<string, NodeImplementationPtr> ShaderGenRegistry::_implementations;

FileSearchPath ShaderGenRegistry::_sourceCodeSearchPath;

void ShaderGenRegistry::registerShaderGenerator(const string& language, const string& target, CreatorFunc<ShaderGenerator> f)
{
    const string id = ShaderGenerator::id(language, target);
    _shaderGeneratorFactory.registerClass(id, f);
}

void ShaderGenRegistry::unregisterShaderGenerator(const string& language, const string& target)
{
    const string id = ShaderGenerator::id(language, target);
    _shaderGeneratorFactory.unregisterClass(id);
    _shaderGenerators.erase(id);
}

void ShaderGenRegistry::registerNodeImplementation(const string& node, const string& language, const string& target, CreatorFunc<NodeImplementation> f)
{
    const string id = NodeImplementation::id(node, language, target);
    _implementationFactory.registerClass(id, f);
}

void ShaderGenRegistry::unregisterNodeImplementation(const string& node, const string& language, const string& target)
{
    const string id = NodeImplementation::id(node, language, target);
    _implementationFactory.unregisterClass(id);
    _implementations.erase(id);
}

ShaderGeneratorPtr ShaderGenRegistry::findShaderGenerator(const string& language, const string& target)
{
    // First, search only by language
    ShaderGeneratorPtr ptr = findShaderGeneratorById(ShaderGenerator::id(language));
    if (ptr != nullptr)
    {
        return ptr;
    }

    // Second, search by language and target
    return findShaderGeneratorById(ShaderGenerator::id(language, target));
}

NodeImplementationPtr ShaderGenRegistry::findNodeImplementation(const string& node, const string& language, const string& target)
{
    // First, search only by node name
    NodeImplementationPtr ptr = findNodeImplementationById(NodeImplementation::id(node));
    if (ptr != nullptr)
    {
        return ptr;
    }

    // Second, search by node name and language
    ptr = findNodeImplementationById(NodeImplementation::id(node, language));
    if (ptr != nullptr)
    {
        return ptr;
    }

    // Third, search by node name, language and target
    return findNodeImplementationById(NodeImplementation::id(node, language, target));
}

void ShaderGenRegistry::registerSourceCodeSearchPath(const FilePath& path)
{
    _sourceCodeSearchPath.append(path);
}

/// Resolve a file using the registered search paths.
FilePath ShaderGenRegistry::findSourceCode(const FilePath& filename)
{
    return _sourceCodeSearchPath.find(filename);
}

const FileSearchPath& ShaderGenRegistry::sourceCodeSearchPath()
{
    return _sourceCodeSearchPath;
}

ShaderGeneratorPtr ShaderGenRegistry::findShaderGeneratorById(const string& id)
{
    auto it = _shaderGenerators.find(id);
    if (it != _shaderGenerators.end())
    {
        return it->second;
    }

    ShaderGeneratorPtr generatorPtr = _shaderGeneratorFactory.create(id);
    if (generatorPtr)
    {
        _shaderGenerators[id] = generatorPtr;
    }

    return generatorPtr;
}

NodeImplementationPtr ShaderGenRegistry::findNodeImplementationById(const string& id)
{
    auto it = _implementations.find(id);
    if (it != _implementations.end())
    {
        return it->second;
    }

    NodeImplementationPtr nodePtr = _implementationFactory.create(id);
    if (nodePtr)
    {
        _implementations[id] = nodePtr;
    }

    return nodePtr;
}


#define REGISTER_SHADER_GENERATOR(T)   \
    registerShaderGenerator(T::kLanguage, T::kTarget, T::creator);
#define REGISTER_NODE_IMPLEMENTATION(T)        \
    registerNodeImplementation(T::kNode, T::kLanguage, T::kTarget, T::creator);

#define UNREGISTER_SHADER_GENERATOR(T) \
    unregisterShaderGenerator(T::kLanguage, T::kTarget);
#define UNREGISTER_NODE_IMPLEMENTATION(T)      \
    unregisterNodeImplementation(T::kNode, T::kLanguage, T::kTarget);

void ShaderGenRegistry::registerBuiltIn()
{
    REGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);
    REGISTER_SHADER_GENERATOR(OgsFxShaderGenerator);

    REGISTER_NODE_IMPLEMENTATION(VDirectionFlipOsl);
    REGISTER_NODE_IMPLEMENTATION(VDirectionNoOpOsl);
    REGISTER_NODE_IMPLEMENTATION(VDirectionFlipGlsl);
    REGISTER_NODE_IMPLEMENTATION(VDirectionNoOpGlsl);
    REGISTER_NODE_IMPLEMENTATION(Swizzle);
    REGISTER_NODE_IMPLEMENTATION(Switch);
    REGISTER_NODE_IMPLEMENTATION(Compare);
    REGISTER_NODE_IMPLEMENTATION(SurfaceOgsFx);
}

void ShaderGenRegistry::unregisterBuiltIn()
{
    UNREGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);
    UNREGISTER_SHADER_GENERATOR(OgsFxShaderGenerator);

    UNREGISTER_NODE_IMPLEMENTATION(VDirectionFlipOsl);
    UNREGISTER_NODE_IMPLEMENTATION(VDirectionNoOpOsl);
    UNREGISTER_NODE_IMPLEMENTATION(VDirectionFlipGlsl);
    UNREGISTER_NODE_IMPLEMENTATION(VDirectionNoOpGlsl);
    UNREGISTER_NODE_IMPLEMENTATION(Swizzle);
    UNREGISTER_NODE_IMPLEMENTATION(Switch);
    UNREGISTER_NODE_IMPLEMENTATION(Compare);
    UNREGISTER_NODE_IMPLEMENTATION(SurfaceOgsFx);
}

} // namepsace MaterialX
