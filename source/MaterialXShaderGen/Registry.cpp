#include <MaterialXShaderGen/Registry.h>

#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>

#include <MaterialXShaderGen/CustomImpls/VDirectionImpl.h>
#include <MaterialXShaderGen/CustomImpls/SwizzleImpl.h>

namespace MaterialX
{

Factory<ShaderGenerator> Registry::_shaderGeneratorFactory;
unordered_map<string, ShaderGeneratorPtr> Registry::_shaderGenerators;

Factory<CustomImpl> Registry::_implementationFactory;
unordered_map<string, CustomImplPtr> Registry::_implementations;

void Registry::registerShaderGenerator(const string& language, const string& target, CreatorFunc<ShaderGenerator> f)
{
    const string id = ShaderGenerator::id(language, target);
    _shaderGeneratorFactory.registerClass(id, f);
}

void Registry::unregisterShaderGenerator(const string& language, const string& target)
{
    const string id = ShaderGenerator::id(language, target);
    _shaderGeneratorFactory.unregisterClass(id);
    _shaderGenerators.erase(id);
}

void Registry::registerImplementation(const string& node, const string& language, const string& target, CreatorFunc<CustomImpl> f)
{
    const string id = CustomImpl::id(node, language, target);
    _implementationFactory.registerClass(id, f);
}

void Registry::unregisterImplementation(const string& node, const string& language, const string& target)
{
    const string id = CustomImpl::id(node, language, target);
    _implementationFactory.unregisterClass(id);
    _implementations.erase(id);
}

ShaderGeneratorPtr Registry::findShaderGenerator(const string& language, const string& target)
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

CustomImplPtr Registry::findImplementation(const string& node, const string& language, const string& target)
{
    // First, search only by node name
    CustomImplPtr ptr = findImplementationById(CustomImpl::id(node));
    if (ptr != nullptr)
    {
        return ptr;
    }

    // Second, search by node name and language
    ptr = findImplementationById(CustomImpl::id(node, language));
    if (ptr != nullptr)
    {
        return ptr;
    }

    // Third, search by node name, language and target
    return findImplementationById(CustomImpl::id(node, language, target));
}

ShaderGeneratorPtr Registry::findShaderGeneratorById(const string& id)
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

CustomImplPtr Registry::findImplementationById(const string& id)
{
    auto it = _implementations.find(id);
    if (it != _implementations.end())
    {
        return it->second;
    }

    CustomImplPtr nodePtr = _implementationFactory.create(id);
    if (nodePtr)
    {
        _implementations[id] = nodePtr;
    }

    return nodePtr;
}


#define REGISTER_SHADER_GENERATOR(T)   \
    registerShaderGenerator(T::kLanguage, T::kTarget, T::creator);
#define REGISTER_IMPLEMENTATION(T)        \
    registerImplementation(T::kNode, T::kLanguage, T::kTarget, T::creator);

#define UNREGISTER_SHADER_GENERATOR(T) \
    unregisterShaderGenerator(T::kLanguage, T::kTarget);
#define UNREGISTER_IMPLEMENTATION(T)      \
    unregisterImplementation(T::kNode, T::kLanguage, T::kTarget);

void Registry::registerBuiltIn()
{
    REGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);

    REGISTER_IMPLEMENTATION(VDirectionImplFlipOsl);
    REGISTER_IMPLEMENTATION(VDirectionImplNoOpOsl);
    REGISTER_IMPLEMENTATION(VDirectionImplFlipGlsl);
    REGISTER_IMPLEMENTATION(VDirectionImplNoOpGlsl);
    REGISTER_IMPLEMENTATION(SwizzleImpl);
}

void Registry::unregisterBuiltIn()
{
    UNREGISTER_SHADER_GENERATOR(ArnoldShaderGenerator);

    UNREGISTER_IMPLEMENTATION(VDirectionImplFlipOsl);
    UNREGISTER_IMPLEMENTATION(VDirectionImplNoOpOsl);
    UNREGISTER_IMPLEMENTATION(VDirectionImplFlipGlsl);
    UNREGISTER_IMPLEMENTATION(VDirectionImplNoOpGlsl);
    UNREGISTER_IMPLEMENTATION(SwizzleImpl);
}


} // namepsace MaterialX
