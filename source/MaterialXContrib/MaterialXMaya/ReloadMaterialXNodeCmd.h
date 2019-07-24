#ifndef RELOAD_MATERIALX_NODE_CMD_H
#define RELOAD_MATERIALX_NODE_CMD_H

#include <maya/MPxCommand.h>

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

#endif /* RELOAD_MATERIALX_NODE_CMD_H */
