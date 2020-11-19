#include "Plugin.h"
#include "CreateMaterialXNodeCmd.h"
#include "ReloadMaterialXNodeCmd.h"
#include "ReloadMaterialXLibrariesCmd.h"
#include "MaterialXNode.h"
#include "ShadingNodeOverrides.h"

#include <MaterialXFormat/Util.h>
#ifdef MATERIALX_BUILD_CROSS
#include <MaterialXCross/Cross.h>
#endif

#include <maya/MFnPlugin.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MFragmentManager.h>

#include <unordered_set>

namespace MaterialXMaya
{

namespace
{
class SearchPathBuilder
{
public:
    explicit SearchPathBuilder(mx::FileSearchPath& searchPath) : _searchPath(searchPath)
    {
        _searchPath = mx::FileSearchPath();
    }

    void append(const mx::FilePath& filePath)
    {
        if (_uniquePaths.insert(filePath.asString()).second)
        {
            _searchPath.append(filePath);
        }
    }

    void appendFromOptionVar(const MString& optionVarName)
    {
        MStringArray mstrArray;
        if (MGlobal::executeCommand("optionVar -q " + optionVarName, mstrArray))
        {
            for (const MString& mstrPath : mstrArray)
            {
                append(mx::FilePath(mstrPath.asChar()));
            }
        }
    }

  private:
    mx::FileSearchPath& _searchPath;
    std::unordered_set<std::string> _uniquePaths;
};

void setIntermediateDumpPath()
{
    bool optionVarExists = false;
    MString path = MGlobal::optionVarStringValue("materialXIntermediateDumpPath", &optionVarExists);

    if (!optionVarExists || path.length() == 0)
    {
        return;
    }

    MHWRender::MRenderer* const theRenderer = MHWRender::MRenderer::theRenderer();
    if (!theRenderer)
    {
        MGlobal::displayError("Failed to get the VP2 renderer");
        return;
    }

    MHWRender::MFragmentManager* const fragmentManager = theRenderer->getFragmentManager();
    if (!fragmentManager)
    {
        MGlobal::displayError("Failed to get the VP2 fragment manager");
        return;
    }

    switch (path.asChar()[path.length() - 1])
    {
    case '/':
    case '\\':
        break;

    default:
        // Add explicitly as VP2 does not append a folder separator,
        // and thus fails silently to output anything.
        path += "/";
    }

    fragmentManager->setEffectOutputDirectory(path);
    fragmentManager->setIntermediateGraphOutputDirectory(path);
}

} // anonymous namespace

Plugin& Plugin::instance()
{
    static Plugin s_instance;
    return s_instance;
}

void Plugin::initialize(const std::string& pluginLoadPath)
{
    _pluginLoadPath = pluginLoadPath;
    setIntermediateDumpPath();
    loadLibraries();
}

mx::FileSearchPath Plugin::getResourceSearchPath() const
{
    mx::FileSearchPath searchPath;
    SearchPathBuilder builder(searchPath);

    // Search in standard installed resources directories and plug-in relative resources
    builder.append(_pluginLoadPath);
    builder.append(_pluginLoadPath / mx::FilePath("../../resources"));
    builder.append(_pluginLoadPath / mx::FilePath("../../libraries"));
    builder.append(_pluginLoadPath / mx::FilePath("../resources"));
    builder.append(_pluginLoadPath / mx::FilePath("../libraries"));
    builder.append(_pluginLoadPath / mx::FilePath(".."));

    builder.appendFromOptionVar("materialXResourceSearchPaths");
    return searchPath;
}

mx::FileSearchPath Plugin::getLightSearchPath() const
{
    mx::FileSearchPath searchPath;
    SearchPathBuilder builder(searchPath);

    // Search in standard installed resources directories and plug-in relative resources
    builder.append(_pluginLoadPath / mx::FilePath("Lights"));
    builder.append(_pluginLoadPath / mx::FilePath("../../resources/Lights"));
    builder.append(_pluginLoadPath / mx::FilePath("../resources/Lights"));
    builder.append(_pluginLoadPath / mx::FilePath("../Lights"));

    builder.appendFromOptionVar("materialXResourceSearchPaths");
    return searchPath;
}

void Plugin::loadLibraries()
{
    _libraryDocument = mx::createDocument();

    {
        SearchPathBuilder builder(_librarySearchPath);
        builder.append(_pluginLoadPath);
        builder.append(_pluginLoadPath / mx::FilePath("../libraries"));
        builder.append(_pluginLoadPath / mx::FilePath("../../libraries"));

        builder.appendFromOptionVar("materialXLibrarySearchPaths");
    }

    std::unordered_set<std::string> uniqueLibraryNames{
        "adsklib", "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl", "lights", "lights/genglsl"
    };

    {
        MStringArray extraLibraryNames;
        MGlobal::executeCommand("optionVar -q materialXLibraryNames", extraLibraryNames);

        for (const MString& mstrLibraryName : extraLibraryNames)
        {
            uniqueLibraryNames.insert(mstrLibraryName.asChar());
        }
    }

    mx::loadLibraries(
        mx::FilePathVec(uniqueLibraryNames.begin(), uniqueLibraryNames.end()),
        _librarySearchPath,
        _libraryDocument
    );
}

} // namespace MaterialXMaya

// Plugin configuration
//
MStatus initializePlugin(MObject obj)
{
    using namespace MaterialXMaya;

    MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

    try
    {
        Plugin::instance().initialize(plugin.loadPath().asChar());
    }
    catch (std::exception& e)
    {
        MString message("Failed to initialize MaterialXMaya plugin: ");
        message += MString(e.what());
        MGlobal::displayError(message);
        return MS::kFailure;
    }

    CHECK_MSTATUS(plugin.registerCommand(
        CreateMaterialXNodeCmd::NAME,
        CreateMaterialXNodeCmd::creator,
        CreateMaterialXNodeCmd::newSyntax));

    CHECK_MSTATUS(plugin.registerCommand(
        ReloadMaterialXNodeCmd::NAME,
        ReloadMaterialXNodeCmd::creator,
        ReloadMaterialXNodeCmd::newSyntax));

    CHECK_MSTATUS(plugin.registerCommand(
        ReloadMaterialXLibrariesCmd::NAME,
        ReloadMaterialXLibrariesCmd::creator,
        ReloadMaterialXLibrariesCmd::newSyntax));

    CHECK_MSTATUS(plugin.registerNode(
        MaterialXNode::MATERIALX_NODE_TYPENAME,
        MaterialXNode::MATERIALX_NODE_TYPEID,
        MaterialXNode::creator,
        MaterialXNode::initialize,
        MPxNode::kDependNode,
        nullptr));

    {
        CHECK_MSTATUS(MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
            TextureOverride::DRAW_CLASSIFICATION,
            TextureOverride::REGISTRANT_ID,
            TextureOverride::creator));

        static const MString texture2dNodeClassification =
            MString("texture/2d:") + TextureOverride::DRAW_CLASSIFICATION;

        CHECK_MSTATUS(plugin.registerNode(
            MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPENAME,
            MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID,
            MaterialXTextureNode::creator,
            MaterialXTextureNode::initialize,
            MPxNode::kDependNode,
            &texture2dNodeClassification));
    }

    {
        CHECK_MSTATUS(MHWRender::MDrawRegistry::registerSurfaceShadingNodeOverrideCreator(
            SurfaceOverride::DRAW_CLASSIFICATION,
            SurfaceOverride::REGISTRANT_ID,
            SurfaceOverride::creator));

        static const MString surfaceNodeClassification =
            MString("shader/surface:") + SurfaceOverride::DRAW_CLASSIFICATION;

        CHECK_MSTATUS(plugin.registerNode(
            MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPENAME,
            MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID,
            MaterialXSurfaceNode::creator,
            MaterialXSurfaceNode::initialize,
            MPxNode::kDependNode,
            &surfaceNodeClassification));
    }

#ifdef MATERIALX_BUILD_CROSS
    mx::Cross::initialize();
#endif
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
    using namespace MaterialXMaya;

    MFnPlugin plugin(obj);
    MStatus status;

    CHECK_MSTATUS(plugin.deregisterNode(MaterialXNode::MATERIALX_NODE_TYPEID));
    CHECK_MSTATUS(plugin.deregisterNode(MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID));
    CHECK_MSTATUS(plugin.deregisterNode(MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID));

    CHECK_MSTATUS(plugin.deregisterCommand(CreateMaterialXNodeCmd::NAME));
    CHECK_MSTATUS(plugin.deregisterCommand(ReloadMaterialXNodeCmd::NAME));
    CHECK_MSTATUS(plugin.deregisterCommand(ReloadMaterialXLibrariesCmd::NAME));

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
            TextureOverride::DRAW_CLASSIFICATION,
            TextureOverride::REGISTRANT_ID ));

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
            SurfaceOverride::DRAW_CLASSIFICATION,
            SurfaceOverride::REGISTRANT_ID ));

#ifdef MATERIALX_BUILD_CROSS
    mx::Cross::finalize();
#endif
    return MS::kSuccess;
}
