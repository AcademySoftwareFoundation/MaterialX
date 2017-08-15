// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <NodeTranslators/Place2dTexture.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

namespace MaterialXForMaya
{

DEFINE_NODE_TRANSLATOR(Place2dTexture, "place2dTexture")

mx::NodePtr Place2dTexture::exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& /*context*/)
{
    const std::string name = getNodeName(mayaNode, outputType);

    // Check if this node instance exists already
    mx::NodePtr node = parent->getNode(name);
    if (node)
    {
        return node;
    }

    MFnDependencyNode fnNode(mayaNode);

    mx::NodePtr texcoord = parent->addNode("texcoord", name, outputType);
    node = texcoord;

    MPlug repeatU = fnNode.findPlug("repeatU", false);
    MPlug repeatV = fnNode.findPlug("repeatV", false);
    if (repeatU.asFloat() != 1.0 || repeatV.asFloat() != 1.0)
    {
        mx::NodePtr repeat = parent->addNode("multiply", name + "_repeat", outputType);
        repeat->setConnectedNode("in1", node);
        if (outputType == "vector3")
        {
            repeat->setInputValue("in2", mx::Vector3(repeatU.asFloat(), repeatV.asFloat(), 0));
        }
        else
        {
            repeat->setInputValue("in2", mx::Vector2(repeatU.asFloat(), repeatV.asFloat()));
        }
        node = repeat;
    }

    MPlug offsetU = fnNode.findPlug("offsetU", false);
    MPlug offsetV = fnNode.findPlug("offsetV", false);
    if (offsetU.asFloat() != 0 || offsetV.asFloat() != 0)
    {
        mx::NodePtr offset = parent->addNode("add", name + "_offset", outputType);
        offset->setConnectedNode("in1", node);
        if (outputType == "vector3")
        {
            offset->setInputValue("in2", mx::Vector3(offsetU.asFloat(), offsetV.asFloat(), 0));
        }
        else
        {
            offset->setInputValue("in2", mx::Vector2(offsetU.asFloat(), offsetV.asFloat()));
        }
        node = offset;
    }

    return node;
}

} // namespace MaterialXForMaya

