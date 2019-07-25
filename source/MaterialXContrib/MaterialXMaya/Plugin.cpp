#include "Plugin.h"
#include "CreateMaterialXNodeCmd.h"
#include "ReloadMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "MaterialXTextureOverride.h"
#include "MaterialXSurfaceOverride.h"

#include <maya/MFnPlugin.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

Plugin& Plugin::instance()
{
	static Plugin s_instance;
	return s_instance;
}

void Plugin::initialize(const std::string& loadPath)
{
    // Always include plug-in load path
	const mx::FilePath searchPath(loadPath);
    
    // Search in standard library directories
    _librarySearchPath.append(searchPath);
    _librarySearchPath.append(searchPath / mx::FilePath("../../libraries"));

    // Search in standard installed resources directories and plug-in relative resources
    _resourceSearchPath.append(searchPath);
    _resourceSearchPath.append(searchPath / mx::FilePath("../../resources"));
    _resourceSearchPath.append(searchPath / mx::FilePath("../resources"));

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
}

///////////////////////////////////////////////////////////////
static const MString sRegistrantId("testFileTexturePlugin");

// Plugin configuration
//
MStatus initializePlugin(MObject obj)
{
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
            MaterialXTextureOverride::DRAW_CLASSIFICATION,
            MaterialXTextureOverride::REGISTRANT_ID,
            MaterialXTextureOverride::creator));

        static const MString texture2dNodeClassification =
            MString("texture/2d:") + MaterialXTextureOverride::DRAW_CLASSIFICATION;

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
            MaterialXSurfaceOverride::DRAW_CLASSIFICATION,
            MaterialXSurfaceOverride::REGISTRANT_ID,
            MaterialXSurfaceOverride::creator));

        const MString& swatchName = MHWShaderSwatchGenerator::initialize();

        static const MString surfaceNodeClassification =
            MString("shader/surface:") + MaterialXSurfaceOverride::DRAW_CLASSIFICATION + ":swatch/" + swatchName;

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
	MFnPlugin plugin(obj);
	MStatus status;

	CHECK_MSTATUS(plugin.deregisterNode(MaterialXNode::MATERIALX_NODE_TYPEID));
    CHECK_MSTATUS(plugin.deregisterNode(MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID));
    CHECK_MSTATUS(plugin.deregisterNode(MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID));

	CHECK_MSTATUS(plugin.deregisterCommand(CreateMaterialXNodeCmd::NAME));
	CHECK_MSTATUS(plugin.deregisterCommand(ReloadMaterialXNodeCmd::NAME));

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
        MaterialXTextureOverride::DRAW_CLASSIFICATION,
		MaterialXTextureOverride::REGISTRANT_ID));

    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
        MaterialXSurfaceOverride::DRAW_CLASSIFICATION,
        MaterialXSurfaceOverride::REGISTRANT_ID));

	return MS::kSuccess;
}
