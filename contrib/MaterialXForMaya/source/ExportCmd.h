#ifndef MATERIALXFORMAYA_EXPORTCMD_H
#define MATERIALXFORMAYA_EXPORTCMD_H

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MMessage.h> 

class ExportCmd : public MPxCommand
{
public:
    static void* creator();
    static MSyntax newSyntax();

    MStatus doIt(const MArgList& argList);

    static const MString kCmdName;
};

#endif // MATERIALXFORMAYA_EXPORTCMD_H
