#ifndef MATERIALX_MAYA_RELOADLIBRARIESCMD_H
#define MATERIALX_MAYA_RELOADLIBRARIESCMD_H

/// @file
/// Maya command for reloading MaterialX libraries.

#include <maya/MPxCommand.h>

namespace MaterialXMaya
{

/// @class ReloadMaterialXLibrariesCmd
/// Reloads all MaterialX libraries used by the plug-in.
///
class ReloadMaterialXLibrariesCmd : MPxCommand
{
public:
    ReloadMaterialXLibrariesCmd();
    ~ReloadMaterialXLibrariesCmd() override;

    MStatus doIt(const MArgList&) override;
    bool isUndoable() const override
    {
        return false;
    }

    static MSyntax newSyntax();
    static void* creator();

    static MString NAME;
};

} // namespace MaterialXMaya

#endif
