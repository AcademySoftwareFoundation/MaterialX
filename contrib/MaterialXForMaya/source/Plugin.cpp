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


MStatus initializePlugin(MObject obj)
{
    MFnPlugin fnPlugin(obj, PLUGIN_COMPANY, "0.1", "Any");

    if (!Plugin::instance().initialize(fnPlugin.loadPath().asChar()))
    {
        return MS::kFailure;
    }

    // Register all translator classes
    Plugin::instance().registerTranslator(DefaultTranslator::typeName(), DefaultTranslator::creator);
    Plugin::instance().registerTranslator(ImageFile::typeName(), ImageFile::creator);
    Plugin::instance().registerTranslator(AmbientOcclusion::typeName(), AmbientOcclusion::creator);
    Plugin::instance().registerTranslator(Place2dTexture::typeName(), Place2dTexture::creator);
    Plugin::instance().setDefaultTranslator(DefaultTranslator::typeName());

    MStatus status;
    status = fnPlugin.registerFileTranslator(FileTranslator::kTranslatorName, "none", FileTranslator::creator, FileTranslator::kOptionScript.asChar(), FileTranslator::kDefaultOptions.asChar());
    if (!status)
    {
        return status;
    }

    status = fnPlugin.registerCommand(ExportCmd::kCmdName, ExportCmd::creator, ExportCmd::newSyntax);
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

    status = fnPlugin.deregisterFileTranslator(FileTranslator::kTranslatorName);
    if (!status)
    {
        return status;
    }
    status = fnPlugin.deregisterCommand(ExportCmd::kCmdName);
    if (!status)
    {
        return status;
    }

    return status;
}
