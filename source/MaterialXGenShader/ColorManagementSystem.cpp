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

bool ColorManagementSystem::supportsTransform(const ColorSpaceTransform& transform) const
{
    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr ColorManagementSystem::createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name, 
                                                const ShaderGenerator& shadergen, GenContext& context) const
{
    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceSpace + "', '" + transform.targetSpace + "').");
    }

    // Check if it's created and cached already.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(implName, shadergen.getTarget());
    if (!nodeImpl)
    {
        // If not, try creating a new implementation in the factory.
        nodeImpl = _implFactory.create(implName);
    }
    // Fall back to the default implementation
    if (!nodeImpl)
    {
        nodeImpl = SourceCodeNode::create();
    }
    nodeImpl->initialize(impl, shadergen, context);

    // Cache it.
    context.addNodeImplementation(implName, shadergen.getTarget(), nodeImpl);

    ShaderNodePtr shaderNode = ShaderNode::createColorTransformNode(parent, name, nodeImpl, transform.type);

    return shaderNode;
}

} // namespace MaterialX
