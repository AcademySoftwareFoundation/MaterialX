#include "ReloadMaterialXNodeCmd.h"

#include <maya/MSyntax.h>

MString ReloadMaterialXNodeCmd::NAME("reloadMaterialXNode");

ReloadMaterialXNodeCmd::ReloadMaterialXNodeCmd()
{
}

ReloadMaterialXNodeCmd::~ReloadMaterialXNodeCmd()
{
}

MStatus ReloadMaterialXNodeCmd::doIt(const MArgList &/*args*/)
{
	// TODO: Implement me!

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
