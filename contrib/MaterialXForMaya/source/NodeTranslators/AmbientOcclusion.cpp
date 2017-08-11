#include <NodeTranslators/AmbientOcclusion.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

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
