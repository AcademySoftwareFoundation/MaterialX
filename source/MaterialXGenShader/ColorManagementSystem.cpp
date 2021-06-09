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
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }
    ImplementationPtr impl = getImplementation(transform);
    return impl != nullptr;
}

ShaderNodePtr ColorManagementSystem::createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name, 
                                                GenContext& context) const
{
    ImplementationPtr impl = getImplementation(transform);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceSpace + "', '" + transform.targetSpace + "').");
    }

    // Check if it's created and cached already,
    // otherwise create and cache it.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(impl->getName());
    if (!nodeImpl)
    {
        nodeImpl = SourceCodeNode::create();
        nodeImpl->initialize(*impl, context);
        context.addNodeImplementation(impl->getName(), nodeImpl);
    }

    // Create the node.
    ShaderNodePtr shaderNode = ShaderNode::create(parent, name, nodeImpl, ShaderNode::Classification::TEXTURE);

    // Create ports on the node.
    ShaderInput* input = shaderNode->addInput("in", transform.type);
    if (transform.type->getName() == Type::COLOR3->getName())
    {
        input->setValue(Value::createValue(Color3(0.0f, 0.0f, 0.0f)));
    }
    else if (transform.type->getName() == Type::COLOR4->getName())
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


void ColorManagementSystem::getPortConnections(ShaderGraph* graph, ShaderNode* colorTransformNode,
                                               const TypeDesc* targetType, GenContext& context,
                                               ShaderInput*& inputToConnect, ShaderOutput*& outputToConnect)
{
    const std::string colorTransformNodeName = colorTransformNode->getName();
    ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);
    ShaderInput* colorTransformNodeInput = colorTransformNode->getInput(0);

    outputToConnect = colorTransformNodeOutput;
    inputToConnect = colorTransformNodeInput;
    if ((colorTransformNodeInput->getType() == Type::COLOR4 && targetType == Type::COLOR3) ||
        (colorTransformNodeInput->getType() == Type::COLOR3 && targetType == Type::COLOR4))
    {
        const string convertToColor4String = "c3_to_" + colorTransformNodeName;
        ShaderNode* convertToColor4Ptr = graph->getNode(convertToColor4String);
        if (!convertToColor4Ptr)
        {
            NodeDefPtr toColor4NodeDef = _document->getNodeDef("ND_convert_color3_color4");
            ShaderNodePtr convertToColor4 = ShaderNode::create(graph, convertToColor4String, *toColor4NodeDef, context);
            graph->addNode(convertToColor4);
            convertToColor4Ptr = convertToColor4.get();
        }
        const string convertToColor3String = colorTransformNodeName + "_to_c3";
        ShaderNode* convertToColor3Ptr = graph->getNode(convertToColor3String);
        if (!convertToColor3Ptr)
        {
            NodeDefPtr toColor3NodeDef = _document->getNodeDef("ND_convert_color4_color3");
            ShaderNodePtr convertToColor3 = ShaderNode::create(graph, convertToColor3String, *toColor3NodeDef, context);
            graph->addNode(convertToColor3);
            convertToColor3Ptr = convertToColor3.get();
        }

        if (targetType == Type::COLOR3)
        {
            colorTransformNodeInput->makeConnection(convertToColor4Ptr->getOutput(0));
            convertToColor3Ptr->getInput(0)->makeConnection(colorTransformNodeOutput);
            inputToConnect = convertToColor4Ptr->getInput(0);
            outputToConnect = convertToColor3Ptr->getOutput(0);
        }
        else
        {
            colorTransformNodeInput->makeConnection(convertToColor3Ptr->getOutput(0));
            convertToColor4Ptr->getInput(0)->makeConnection(colorTransformNodeOutput);
            inputToConnect = convertToColor3Ptr->getInput(0);
            outputToConnect = convertToColor4Ptr->getOutput(0);
        }
    }
}

void ColorManagementSystem::connectNodeToShaderInput(ShaderGraph* graph, ShaderNode* colorTransformNode, ShaderInput* shaderInput, GenContext& context)
{
    if (!graph || !colorTransformNode|| !shaderInput)
    {
        throw ExceptionShaderGenError("Cannot connect color management node to shader input");
    }

    ShaderInput* inputToConnect = nullptr;
    ShaderOutput* outputToConnect = nullptr;

    getPortConnections(graph, colorTransformNode, shaderInput->getType(), context, inputToConnect, outputToConnect);
    if (inputToConnect && outputToConnect)
    {
        inputToConnect->setValue(shaderInput->getValue());

        if (shaderInput->isBindInput())
        {
            ShaderOutput* oldConnection = shaderInput->getConnection();
            inputToConnect->makeConnection(oldConnection);
        }

        shaderInput->makeConnection(outputToConnect);
    }
}


void ColorManagementSystem::connectNodeToShaderOutput(ShaderGraph* graph, ShaderNode* colorTransformNode, ShaderOutput* shaderOutput, GenContext& context)
{
    if (!graph || !colorTransformNode || !shaderOutput)
    {
        throw ExceptionShaderGenError("Cannot connect color management node to shader output");
    }

    ShaderInput* inputToConnect = nullptr;
    ShaderOutput* outputToConnect = nullptr;

    getPortConnections(graph, colorTransformNode, shaderOutput->getType(), context, inputToConnect, outputToConnect);
    if (inputToConnect && outputToConnect)
    {
        ShaderInputSet downStreamInputs = shaderOutput->getConnections();
        for (ShaderInput* downStreamInput : downStreamInputs)
        {
            downStreamInput->breakConnection();
            downStreamInput->makeConnection(outputToConnect);
        }

        // Connect the node to the upstream output.
        inputToConnect->makeConnection(shaderOutput);
    }
}

} // namespace MaterialX
