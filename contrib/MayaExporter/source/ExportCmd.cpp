// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <ExportCmd.h>
#include <SceneTranslator.h>

#include <MaterialXFormat/XmlIo.h>

#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>

#include <sstream>

namespace MaterialXForMaya
{
const char* fileFlag = "file";
const char* fileFlagShort = "f";

const char* selectionFlag = "selection";
const char* selectionFlagShort = "sel";

const char* includeStdLibFlag = "includeStdLib";
const char* includeStdLibFlagShort = "std";

const char* materialAssignmentsFlag = "materialAssignments";
const char* materialAssignmentsFlagShort = "ma";

const char* surfaceShadersFlag = "surfaceShaders";
const char* surfaceShadersFlagShort = "ss";

const char* displacementShadersFlag = "displacementShaders";
const char* displacementShadersFlagShort = "ds";

const char* nodeDefinitionsFlag = "nodeDefinitions";
const char* nodeDefinitionsFlagShort = "nd";

const char* lightsFlag = "lights";
const char* lightsFlagShort = "li";

const char* lightAssignmentsFlag = "lightAssignments";
const char* lightAssignmentsFlagShort = "lia";

const char* lightShadersFlag = "lightShaders";
const char* lightShadersFlagShort = "lis";

const MString ExportCmd::kCmdName = "mxExport";

void* ExportCmd::creator()
{
    return new ExportCmd();
}

MSyntax ExportCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(fileFlagShort, fileFlag, MSyntax::kString);
    syntax.addFlag(selectionFlagShort, selectionFlag, MSyntax::kNoArg);
    syntax.addFlag(includeStdLibFlagShort, includeStdLibFlag, MSyntax::kBoolean);
    syntax.addFlag(materialAssignmentsFlagShort, materialAssignmentsFlag, MSyntax::kBoolean);
    syntax.addFlag(surfaceShadersFlagShort, surfaceShadersFlag, MSyntax::kBoolean);
    syntax.addFlag(displacementShadersFlagShort, displacementShadersFlag, MSyntax::kBoolean);
    syntax.addFlag(nodeDefinitionsFlagShort, nodeDefinitionsFlag, MSyntax::kBoolean);
    syntax.addFlag(lightsFlagShort, lightsFlag, MSyntax::kBoolean);
    syntax.addFlag(lightAssignmentsFlagShort, lightAssignmentsFlag, MSyntax::kBoolean);
    syntax.addFlag(lightShadersFlagShort, lightShadersFlag, MSyntax::kBoolean);

    return syntax;
}

MStatus ExportCmd::doIt(const MArgList& argList)
{
    MStatus status;
    MArgDatabase args(syntax(), argList, &status);

    if (!status)
    {
       MGlobal::displayError("Error parsing the arg list.");
        return MS::kFailure;
    }

    const MString file = args.isFlagSet(fileFlag) ? args.flagArgumentString(fileFlag, 0) : "";
    const bool selectionOnly = args.isFlagSet(selectionFlag);

    SceneTranslator::Options options;
    options.includeStdLib = args.isFlagSet(includeStdLibFlag) ? args.flagArgumentBool(includeStdLibFlag, 0) : true;
    options.materialAssignments = args.isFlagSet(materialAssignmentsFlag) ? args.flagArgumentBool(materialAssignmentsFlag, 0) : true;
    options.surfaceShaders = args.isFlagSet(surfaceShadersFlag) ? args.flagArgumentBool(surfaceShadersFlag, 0) : true;
    options.displacementShaders = args.isFlagSet(displacementShadersFlag) ? args.flagArgumentBool(displacementShadersFlag, 0) : true;
    options.nodeDefinitions = args.isFlagSet(nodeDefinitionsFlag) ? args.flagArgumentBool(nodeDefinitionsFlag, 0) : true;
    options.lights = args.isFlagSet(lightsFlag) ? args.flagArgumentBool(lightsFlag, 0) : true;
    options.lightAssignments = args.isFlagSet(lightAssignmentsFlag) ? args.flagArgumentBool(lightAssignmentsFlag, 0) : true;
    options.lightShaders = args.isFlagSet(lightShadersFlag) ? args.flagArgumentBool(lightShadersFlag, 0) : true;

    try
    {
       SceneTranslator::MObjectHandleSet mayaMaterials, mayaLights;
       SceneTranslator::findMaterials(selectionOnly, mayaMaterials);
       if (options.lights)
       {
          SceneTranslator::findLights(selectionOnly, mayaLights);
       }

       SceneTranslator translator(options);
       translator.exportLook(mayaMaterials, mayaLights);

       if (file.length() > 0)
       {
           translator.writeToFile(file.asChar());
           setResult(file);
       }
       else
       {
           std::stringstream stream;
           translator.writeToStream(stream);
           setResult(MString(stream.str().c_str()));
       }
    }
    catch (std::exception& e)
    {
       MGlobal::displayError(e.what());
       return MS::kFailure;
    }

    return MS::kSuccess;
}

} // namespace MaterialXForMaya
