// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_EXPORTCMD_H
#define MATERIALXFORMAYA_EXPORTCMD_H

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MMessage.h> 

namespace MaterialXForMaya
{

/// @class ExportCmd
/// Custom Maya command for MaterialX export
class ExportCmd : public MPxCommand
{
  public:
    /// Command creator
    static void* creator();
    /// Command syntax creation
    static MSyntax newSyntax();

    /// Command execution
    MStatus doIt(const MArgList& argList);

    /// Name of command
    static const MString kCmdName;
};

} // namespace MaterialXForMaya


#endif // MATERIALXFORMAYA_EXPORTCMD_H
