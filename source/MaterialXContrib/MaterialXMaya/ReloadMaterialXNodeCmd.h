#ifndef MATERIALX_MAYA_RELOADNODECMD_H
#define MATERIALX_MAYA_RELOADNODECMD_H

/// @file
/// Maya command for reloading MaterialX nodes.

#include <maya/MPxCommand.h>

namespace MaterialXMaya
{

/// @class ReloadMaterialXNodeCmd
/// Reloads the document and refreshes the viewport shader mapped to the
/// specified MaterialX node. This command is useful when the contents of the
/// document file have changed on disk, e.g. due to editing in an external XML
/// editor.
///
class ReloadMaterialXNodeCmd : MPxCommand
{
public:
    ReloadMaterialXNodeCmd();
    ~ReloadMaterialXNodeCmd() override;

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
