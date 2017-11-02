#include <MaterialXShaderGen/ShaderGenRegistry.h>

#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>

namespace MaterialX
{

Factory<ShaderGenerator> ShaderGenRegistry::_shaderGeneratorFactory;
unordered_map<string, ShaderGeneratorPtr> ShaderGenRegistry::_shaderGenerators;

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

#define REGISTER_SHADER_GENERATOR(T)   \
    registerShaderGenerator(T::kLanguage, T::kTarget, T::creator);

#define UNREGISTER_SHADER_GENERATOR(T) \
    unregisterShaderGenerator(T::kLanguage, T::kTarget);

void ShaderGenRegistry::registerBuiltIn()
{
    REGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);
    REGISTER_SHADER_GENERATOR(OgsFxShaderGenerator);
}

void ShaderGenRegistry::unregisterBuiltIn()
{
    UNREGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);
    UNREGISTER_SHADER_GENERATOR(OgsFxShaderGenerator);
}

} // namepsace MaterialX
