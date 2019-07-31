#include "Plugin.h"
#include "CreateMaterialXNodeCmd.h"
#include "ReloadMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "ShadingNodeOverrides.h"

#include <maya/MFnPlugin.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

#include <unordered_set>

namespace MaterialXMaya
{

Plugin& Plugin::instance()
{
    static Plugin s_instance;
    return s_instance;
}

void Plugin::initialize(const std::string& pluginLoadPath)
{
    // Always include plug-in load path
    _pluginLoadPath = pluginLoadPath;

    // Search in standard installed resources directories and plug-in relative resources
    _resourceSearchPath.append(_pluginLoadPath);
    _resourceSearchPath.append(_pluginLoadPath / mx::FilePath("../../resources"));
    _resourceSearchPath.append(_pluginLoadPath / mx::FilePath("../resources"));

    MHWRender::MRenderer* const theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* const fragmentManager = theRenderer ? theRenderer->getFragmentManager() : nullptr;
    if (!fragmentManager)
    {
        MGlobal::displayError("Failed to get the VP2 fragment manager");
    }
    else
    {
        // Set to resource path for now
        std::string shaderDebugPath = _resourceSearchPath[1].asString();

        // Add explicitly as VP2 does no prepend a folder separator, and thus fails silently to output anything.
        shaderDebugPath += "/";
        fragmentManager->setEffectOutputDirectory(shaderDebugPath.c_str());
        fragmentManager->setIntermediateGraphOutputDirectory(shaderDebugPath.c_str());
    }

    loadLibraries();
}

void Plugin::loadLibraries()
{
    _librarySearchPath = mx::FileSearchPath();
    _libraryDocument = mx::createDocument();

    {
        // Hash set to avoid duplicates.
        std::unordered_set<std::string> uniquePaths;

        auto appendFilePath = [this, &uniquePaths](const mx::FilePath& filePath)
        {
            if (uniquePaths.insert(filePath.asString()).second)
            {
                _librarySearchPath.append(filePath);
            }
        };

        appendFilePath(_pluginLoadPath);
        appendFilePath(_pluginLoadPath / mx::FilePath("../../libraries"));

        MStringArray extraSearchPaths;
        MGlobal::executeCommand("optionVar -q materialXLibrarySearchPaths", extraSearchPaths);

        for (const MString& mstrPath : extraSearchPaths)
        {
            const std::string strPath = mstrPath.asChar();
            if (uniquePaths.insert(strPath).second)
            {
                _librarySearchPath.append(mx::FilePath(strPath));
            }
        }
    }

    std::unordered_set<std::string> uniqueLibraryNames{
        "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl", "lights", "lights/genglsl"
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
        mx::StringVec(uniqueLibraryNames.begin(), uniqueLibraryNames.end()),
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
    Plugin::instance().initialize(plugin.loadPath().asChar());

    CHECK_MSTATUS(plugin.registerCommand(
        CreateMaterialXNodeCmd::NAME,
        CreateMaterialXNodeCmd::creator,
        CreateMaterialXNodeCmd::newSyntax));

    CHECK_MSTATUS(plugin.registerCommand(
        ReloadMaterialXNodeCmd::NAME,
        ReloadMaterialXNodeCmd::creator,
        ReloadMaterialXNodeCmd::newSyntax));

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

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
            TextureOverride::DRAW_CLASSIFICATION,
            TextureOverride::REGISTRANT_ID ));

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
            SurfaceOverride::DRAW_CLASSIFICATION,
            SurfaceOverride::REGISTRANT_ID ));

    return MS::kSuccess;
}
