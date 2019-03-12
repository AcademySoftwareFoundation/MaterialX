//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

//
// ColorSpaceTransform methods
//

ColorSpaceTransform::ColorSpaceTransform(const string& ss, const string& ts, const TypeDesc* t) :
    sourceSpace(ss),
    targetSpace(ts),
    type(t)
{
    if (type != Type::COLOR3 && type != Type::COLOR4)
    {
        throw ExceptionShaderGenError("Color space transform can only be a color3 or color4.");
    }
}


ColorManagementSystem::ColorManagementSystem()
{
}

void ColorManagementSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
}

bool ColorManagementSystem::supportsTransform(const ColorSpaceTransform& transform) const
{
    const string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr ColorManagementSystem::createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name, 
                                                GenContext& context) const
{
    const string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceSpace + "', '" + transform.targetSpace + "').");
    }

    // Check if it's created and cached already,
    // otherwise create and cache it.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(implName);
    if (!nodeImpl)
    {
        nodeImpl = SourceCodeNode::create();
        nodeImpl->initialize(*impl, context);
        context.addNodeImplementation(implName, nodeImpl);
    }

    // Create the node.
    ShaderNodePtr shaderNode = ShaderNode::create(parent, name, nodeImpl, 
        ShaderNode::Classification::TEXTURE | ShaderNode::Classification::COLOR_SPACE_TRANSFORM);

    // Create ports on the node.
    ShaderInput* input = shaderNode->addInput("in", transform.type);
    if (transform.type == Type::COLOR3)
    {
        input->setValue(Value::createValue(Color3(0.0f, 0.0f, 0.0f)));
    }
    else if (transform.type == Type::COLOR4)
    {
        input->setValue(Value::createValue(Color4(0.0f, 0.0f, 0.0f, 1.0)));
    }
    else
    {
        throw ExceptionShaderGenError("Invalid type specified to createColorTransform: '" + transform.type->getName() + "'");
    }
    shaderNode->addOutput("out", transform.type);

    return shaderNode;
}

} // namespace MaterialX
