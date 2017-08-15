// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <Plugin.h>
#include <Factory.h>
#include <FileTranslator.h>
#include <ExportCmd.h>
#include <NodeTranslator.h>
#include <NodeTranslators/DefaultTranslator.h>
#include <NodeTranslators/ImageFile.h>
#include <NodeTranslators/AmbientOcclusion.h>
#include <NodeTranslators/Place2dTexture.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>

namespace MaterialXForMaya
{

Plugin& Plugin::instance()
{
    static Plugin s_instance;
    return s_instance;
}

Plugin::Plugin()
{
}

bool Plugin::initialize(const string& loadPath)
{
    mx::FilePath searchPath(loadPath);
    searchPath = searchPath / mx::FilePath("../data");
    _librarySearchPath = searchPath;

    // Load the translation data
    try
    {
        _translators.clear();
        _translatorData = mx::createDocument();
        mx::readFromXmlFile(_translatorData, "maya_translation.mtlx", _librarySearchPath);
    }
    catch (mx::Exception&)
    {
        return false;
    }

    // Setup the attribute ignore list
    mx::NodeDefPtr ignoreList = _translatorData->getNodeDef("mayaExportAttributeIgnoreList");
    if (ignoreList)
    {
        for (mx::ElementPtr child : ignoreList->getChildren())
        {
            NodeTranslator::_attributeIgnoreList.insert(child->getName());
        }
    }

    return true;
}

void Plugin::registerTranslator(const string& typeName, CreatorFunction f)
{
    _factory.registerClass(typeName, f);
}

void Plugin::setDefaultTranslator(const string& typeName)
{
    _defaultTranslator = typeName;
}

NodeTranslatorPtr Plugin::getTranslator(const MObject& mayaNode)
{
    MFnDependencyNode fnNode(mayaNode);

    const string typeName = fnNode.typeName().asChar();

    auto it = _translators.find(typeName);
    if (it != _translators.end())
    {
        return it->second;
    }

    NodeTranslatorPtr translator = _factory.create(typeName);
    if (!translator)
    {
        translator = _factory.create(_defaultTranslator);
    }

    if (translator)
    {
        translator->initialize(mayaNode, _translatorData);
        _translators[typeName] = translator;
    }

    return translator;
}

} // namespace MaterialXForMaya

MStatus initializePlugin(MObject obj)
{
    MFnPlugin fnPlugin(obj, PLUGIN_COMPANY, "0.1", "Any");

    if (!MaterialXForMaya::Plugin::instance().initialize(fnPlugin.loadPath().asChar()))
    {
        return MS::kFailure;
    }

    // Register all translator classes
    MaterialXForMaya::Plugin::instance().registerTranslator(MaterialXForMaya::DefaultTranslator::typeName(), MaterialXForMaya::DefaultTranslator::creator);
    MaterialXForMaya::Plugin::instance().registerTranslator(MaterialXForMaya::ImageFile::typeName(), MaterialXForMaya::ImageFile::creator);
    MaterialXForMaya::Plugin::instance().registerTranslator(MaterialXForMaya::AmbientOcclusion::typeName(), MaterialXForMaya::AmbientOcclusion::creator);
    MaterialXForMaya::Plugin::instance().registerTranslator(MaterialXForMaya::Place2dTexture::typeName(), MaterialXForMaya::Place2dTexture::creator);
    MaterialXForMaya::Plugin::instance().setDefaultTranslator(MaterialXForMaya::DefaultTranslator::typeName());

    MStatus status;
    status = fnPlugin.registerFileTranslator(MaterialXForMaya::FileTranslator::kTranslatorName, "none", MaterialXForMaya::FileTranslator::creator, 
        MaterialXForMaya::FileTranslator::kOptionScript.asChar(), MaterialXForMaya::FileTranslator::kDefaultOptions.asChar());
    if (!status)
    {
        return status;
    }

    status = fnPlugin.registerCommand(MaterialXForMaya::ExportCmd::kCmdName, MaterialXForMaya::ExportCmd::creator, 
                                      MaterialXForMaya::ExportCmd::newSyntax);
    if (!status)
    {
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin fnPlugin(obj);

    MStatus status;

    status = fnPlugin.deregisterFileTranslator(MaterialXForMaya::FileTranslator::kTranslatorName);
    if (!status)
    {
        return status;
    }
    status = fnPlugin.deregisterCommand(MaterialXForMaya::ExportCmd::kCmdName);
    if (!status)
    {
        return status;
    }

    return status;
}

