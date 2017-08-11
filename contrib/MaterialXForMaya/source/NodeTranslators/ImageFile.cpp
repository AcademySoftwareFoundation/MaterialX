#include <NodeTranslators/ImageFile.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

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
