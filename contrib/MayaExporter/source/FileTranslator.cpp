// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <FileTranslator.h>
#include <SceneTranslator.h>
#include <MaterialXFormat/XmlIo.h>
#include <maya/MGlobal.h>

namespace mx = MaterialX;

namespace MaterialXForMaya
{
    MStatus parseOptions(const MString& optionsString, SceneTranslator::Options& options)
    {
        if (optionsString.length() > 0)
        {
            MStringArray optionList;
            optionsString.split(';', optionList);

            for (unsigned int i = 0; i < optionList.length(); ++i)
            {
                MStringArray option;
                optionList[i].split('=', option);
                if (option.length() < 2)
                {
                    MGlobal::displayError("Invalid options string: " + optionsString);
                    return MS::kFailure;
                }
                if (option[0] == MString("includeStdLib"))
                {
                   options.includeStdLib = option[1].asInt() > 0;
                }
                else if (option[0] == MString("materialAssignments"))
                {
                    options.materialAssignments = option[1].asInt() > 0;
                }
                else if (option[0] == MString("surfaceShaders"))
                {
                    options.surfaceShaders = option[1].asInt() > 0;
                }
                else if (option[0] == MString("displacementShaders"))
                {
                    options.displacementShaders = option[1].asInt() > 0;
                }
                else if (option[0] == MString("nodeDefinitions"))
                {
                    options.nodeDefinitions = option[1].asInt() > 0;
                }
                else if (option[0] == MString("lights"))
                {
                    options.lights = option[1].asInt() > 0;
                }
                else if (option[0] == MString("lightAssignments"))
                {
                    options.lightAssignments = option[1].asInt() > 0;
                }
                else if (option[0] == MString("lightShaders"))
                {
                    options.lightShaders = option[1].asInt() > 0;
                }
            }
        }
        return MS::kSuccess;
    }

const MString FileTranslator::kTranslatorName = "MaterialX";
const MString FileTranslator::kOptionScript = "mxFileTranslatorOptions";
const MString FileTranslator::kDefaultOptions =
    "includeStdLib=1;"
    "materialAssignments=1;"
    "surfaceShaders=1;"
    "displacementShaders=1;"
    "nodeDefinitions=0;"
    "lights=0;"
    "lightAssignments=0"
    "lightShaders=0";

FileTranslator::FileTranslator()
{
}

FileTranslator::~FileTranslator()
{
}

void* FileTranslator::creator()
{
    return new FileTranslator();
}

MStatus FileTranslator::reader(const MFileObject&, const MString& /*optionsString*/, FileAccessMode /*mode*/)
{
    MGlobal::displayError("MaterialX file import not supported!");
    return MS::kFailure;
}

MStatus FileTranslator::writer(const MFileObject& file, const MString& optionsString, FileAccessMode mode)
{
   SceneTranslator::Options options;
    if (!parseOptions(optionsString, options))
    {
        return MS::kFailure;
    }

    const bool selectionOnly = (mode == MPxFileTranslator::kExportActiveAccessMode);
    const std::string filename = file.resolvedFullName().asChar();

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

        translator.writeToFile(filename);
    }
    catch (std::exception& e)
    {
        MGlobal::displayError(e.what());
        return MS::kFailure;
    }

    return MS::kSuccess;
}

bool FileTranslator::haveReadMethod () const
{
    return false;
}

bool FileTranslator::haveWriteMethod () const
{
    return true;
}

MString FileTranslator::defaultExtension () const
{
    return "mtlx";
}

MPxFileTranslator::MFileKind FileTranslator::identifyFile(const MFileObject& file, const char* /*buffer*/, short /*size*/) const
{
    const std::string name = file.resolvedName().asChar();
    const size_t length = name.length();
    if (length > 5 && name.substr(length - 4) == ".mtlx")
    {
        return kCouldBeMyFileType;
    }
    return kNotMyFileType;
}

} // namespace MaterialXForMaya
