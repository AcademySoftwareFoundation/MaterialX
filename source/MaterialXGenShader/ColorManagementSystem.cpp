#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

ColorSpaceTransform::ColorSpaceTransform(const string& ss, const string& ts, const TypeDesc* t)
    : sourceSpace(ss)
    , targetSpace(ts)
    , type(t)
{
    if (type != Type::COLOR3 && type != Type::COLOR4)
    {
        throw ExceptionShaderGenError("Color space transform can only be a color3 or color4.");
    }
}


ColorManagementSystem::ColorManagementSystem(const string& configFile)
    : _configFile(configFile)
{
}

void ColorManagementSystem::registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator)
{
    string implName = getImplementationName(transform);
    _implFactory.registerClass(implName, creator);
    _registeredImplNames.push_back(implName);
}

void  ColorManagementSystem::setConfigFile(const string& configFile)
{
    _configFile = configFile;
    _implFactory.unregisterClasses(_registeredImplNames);
    _registeredImplNames.clear();
    _cachedImpls.clear();
}

void ColorManagementSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
    _implFactory.unregisterClasses(_registeredImplNames);
    _registeredImplNames.clear();
    _cachedImpls.clear();
}

bool ColorManagementSystem::supportsTransform(const ColorSpaceTransform& transform)
{
    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr ColorManagementSystem::createNode(const ColorSpaceTransform& transform, const string& name, ShaderGenerator& shadergen, const GenOptions& options)
{
    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceSpace + "', '" + transform.targetSpace + "').");
    }

    // Check if the shader implementation has been created already
    ShaderNodeImplPtr shaderImpl;
    auto it = _cachedImpls.find(implName);
    if (it != _cachedImpls.end())
    {
        shaderImpl = it->second;
    }
    // If not, try creating a new shader implementation in the factory
    else
    {
        shaderImpl = _implFactory.create(implName);
    }
    // Fall back to the default implementation
    if (!shaderImpl)
    {
        shaderImpl = SourceCodeNode::create();
    }
    shaderImpl->initialize(impl, shadergen, options);

    _cachedImpls[implName] = shaderImpl;
    ShaderNodePtr shaderNode = ShaderNode::createColorTransformNode(name, shaderImpl, transform.type, shadergen);

    return shaderNode;
}

} // namespace MaterialX
