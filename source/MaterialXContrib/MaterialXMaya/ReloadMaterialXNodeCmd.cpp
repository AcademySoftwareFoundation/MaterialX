#include "ReloadMaterialXNodeCmd.h"
#include "MaterialXNode.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgParser.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

MString ReloadMaterialXNodeCmd::NAME("reloadMaterialXNode");

ReloadMaterialXNodeCmd::ReloadMaterialXNodeCmd()
{
}

ReloadMaterialXNodeCmd::~ReloadMaterialXNodeCmd()
{
}

MStatus ReloadMaterialXNodeCmd::doIt(const MArgList& args)
{
    MArgParser parser(syntax(), args);

    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status)
    {
        return status;
    }

    try
    {
        MString materialXNodeName;
        CHECK_MSTATUS(argData.getCommandArgument(0, materialXNodeName))

        MSelectionList list;
        CHECK_MSTATUS(list.add(materialXNodeName))

        MObject node;
        CHECK_MSTATUS(list.getDependNode(0, node))

        MFnDependencyNode depNode(node);
        auto materialXNode = dynamic_cast<MaterialXNode*>(depNode.userNode());
        if (!materialXNode)
        {
            throw mx::Exception("MaterialXNode not found");
        }

        materialXNode->reloadDocument();
    }
    catch (std::exception& e)
    {
        MString message("Failed to reload MaterialX node: ");
        message += MString(e.what());
        MGlobal::displayError(message);
        return MS::kFailure;
    }

    return MS::kSuccess;
}

MSyntax ReloadMaterialXNodeCmd::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString); // Name of the node to reload
    return syntax;
}

void* ReloadMaterialXNodeCmd::creator()
{
    return new ReloadMaterialXNodeCmd();
}

} // namespace MaterialXMaya
