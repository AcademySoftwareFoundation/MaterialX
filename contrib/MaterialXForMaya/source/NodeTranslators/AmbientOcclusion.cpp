// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <NodeTranslators/AmbientOcclusion.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

namespace MaterialXForMaya
{

DEFINE_NODE_TRANSLATOR(AmbientOcclusion, "aiAmbientOcclusion")

mx::NodePtr AmbientOcclusion::exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context)
{
    mx::NodePtr node = NodeTranslator::exportNode(mayaNode, outputType, parent, context);
    if (node)
    {
        MFnDependencyNode fnNode(mayaNode);
        MPlug plug = fnNode.findPlug("spread", false);
        mx::ParameterPtr coneangle = node->getParameter("coneangle");
        if (coneangle && !plug.isNull())
        {
            coneangle->setValue(plug.asFloat() * 90.0f);
        }
    }
    return node;
}

} // namespace MaterialXForMaya
