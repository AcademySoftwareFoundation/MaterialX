#ifndef MATERIALX_MAYA_RELOADNODECMD_H
#define MATERIALX_MAYA_RELOADNODECMD_H

#include <maya/MPxCommand.h>

namespace MaterialXMaya
{

///
///
///
class ReloadMaterialXNodeCmd : MPxCommand
{
public:
	ReloadMaterialXNodeCmd();
	~ReloadMaterialXNodeCmd() override;

	MStatus doIt(const MArgList&) override;
	bool isUndoable() { return false; }

	static MSyntax newSyntax();
	static void* creator();

	static MString NAME;
};

} // namespace MaterialXMaya

#endif
