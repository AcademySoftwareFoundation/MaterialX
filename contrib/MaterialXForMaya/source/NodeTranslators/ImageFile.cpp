// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <NodeTranslators/ImageFile.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

namespace MaterialXForMaya
{

DEFINE_NODE_TRANSLATOR(ImageFile, "file")

mx::NodePtr ImageFile::exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context)
{
    mx::NodePtr node = NodeTranslator::exportNode(mayaNode, outputType, parent, context);
    if (node)
    {
        MFnDependencyNode fnNode(mayaNode);

        MPlug colorSpacePlug = fnNode.findPlug("colorSpace", false);
        mx::ParameterPtr fileParam = node->getParameter("file");
        if (fileParam && !colorSpacePlug.isNull())
        {
            fileParam->setAttribute("colorspace", colorSpacePlug.asString().asChar());
        }

        node = exportColorBalance(mayaNode, node);
    }
    return node;
}

} //namespace MaterialXForMaya

