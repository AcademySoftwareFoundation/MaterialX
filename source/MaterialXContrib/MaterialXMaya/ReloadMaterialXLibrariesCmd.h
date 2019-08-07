#ifndef MATERIALX_MAYA_RELOADLIBRARIESCMD_H
#define MATERIALX_MAYA_RELOADLIBRARIESCMD_H

#include <maya/MPxCommand.h>

namespace MaterialXMaya
{

///
///
///
class ReloadMaterialXLibrariesCmd : MPxCommand
{
public:
    ReloadMaterialXLibrariesCmd();
    ~ReloadMaterialXLibrariesCmd() override;

    MStatus doIt(const MArgList&) override;
    bool isUndoable() { return false; }

    static MSyntax newSyntax();
    static void* creator();

    static MString NAME;
};

} // namespace MaterialXMaya

#endif
