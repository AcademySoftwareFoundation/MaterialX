#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "MaterialXUtil.h"
#include "Plugin.h"

#include <MaterialXFormat/XmlIo.h>

#include <maya/MArgParser.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

#include <algorithm>
#include <sstream>

namespace mx = MaterialX;

namespace
{
const char* const kDocumentFlag     = "d";
const char* const kElementFlag      = "e";

const char* const kTextureFlag      = "t";
const char* const kTextureFlagLong  = "asTexture";

const char* const kOgsXmlFlag       = "x";
const char* const kOgsXmlFlagLong   = "ogsXml";

void registerFragment(const MaterialXData& materialData, const std::string& ogsXmlFileName)
{
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* fragmentManager = theRenderer ? theRenderer->getFragmentManager() : nullptr;
    if (!fragmentManager)
    {
        throw mx::Exception("Failed to find fragment manager");
    }

    // Name of fragment created or reused
    MString fragmentNameM;

    // Register fragments with the manager if needed
    const std::string& fragmentString = materialData.getFragmentSource();
    const std::string& fragmentName = materialData.getFragmentName();
    if (!fragmentName.empty() && !fragmentString.empty())
    {
        mx::FilePath dumpPath(Plugin::instance().getShaderDebugPath());
        if (!dumpPath.isEmpty())
        {
            std::string dumpPathString(dumpPath.asString());
            // Add explicitly as VP2 does no prepend a folder separator, and thus fails silently to output anything.
            dumpPathString += "/"; 
            fragmentManager->setEffectOutputDirectory(dumpPathString.c_str());
            fragmentManager->setIntermediateGraphOutputDirectory(dumpPathString.c_str());
        }

        const bool fragmentExists = fragmentManager->hasFragment(fragmentName.c_str());
        if (!fragmentExists)
        {
            // If explicit XML file specified use it.
            mx::FilePath ogsXmlPath;
            if (!ogsXmlFileName.empty())
            {
                ogsXmlPath = Plugin::instance().getResourceSearchPath().find(ogsXmlFileName);
                if (!ogsXmlPath.isEmpty())
                {
                    fragmentNameM = fragmentManager->addShadeFragmentFromFile(ogsXmlPath.asString().c_str(), false);
                }
            }

            // Otherwise use the generated XML
            else
            {
                fragmentNameM = fragmentManager->addShadeFragmentFromBuffer(fragmentString.c_str(), false);
            }
        }
        else
        {
            fragmentNameM.set(fragmentName.c_str());
        }
    }

    // TODO: On failure a fallback shader should be provided.
    if (fragmentNameM.length() == 0)
    {
        throw mx::Exception("Failed to add shader fragment: (" + fragmentName + ")");
    }
}

}

MString CreateMaterialXNodeCmd::NAME("createMaterialXNode");

CreateMaterialXNodeCmd::CreateMaterialXNodeCmd()
{
}

CreateMaterialXNodeCmd::~CreateMaterialXNodeCmd()
{
}

MStatus CreateMaterialXNodeCmd::doIt( const MArgList &args )
{
    // Parse the shader node
    //
    MArgParser parser(syntax(), args);

    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status)
        return status;

    MString elementPath;
    try
    {
        MString documentFilePath;
        if (parser.isFlagSet(kDocumentFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kDocumentFlag, 0, documentFilePath));
        }

        if (documentFilePath.length() == 0)
        {
            throw mx::Exception("MaterialX document file path is empty.");
        }

	    if (parser.isFlagSet(kElementFlag))
	    {
            CHECK_MSTATUS(argData.getFlagArgument(kElementFlag, 0, elementPath));
	    }

        MString ogsXmlFileName;
        if (parser.isFlagSet(kOgsXmlFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kOgsXmlFlag, 0, ogsXmlFileName));
        }

        mx::DocumentPtr document = MaterialXMaya::loadDocument(
            documentFilePath.asChar(), Plugin::instance().getLibrarySearchPath()
        );

        std::unique_ptr<MaterialXData> materialXData{
            new MaterialXData(document,
                              elementPath.asChar(),
                              Plugin::instance().getLibrarySearchPath())
        };

        elementPath.set(materialXData->getElementPath().c_str());
        if (elementPath.length() == 0)
        {
            throw mx::Exception("The element specified is not renderable.");
        }

        bool asTexture = false;
        if (parser.isFlagSet(kTextureFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kTextureFlag, 0, asTexture));
        }
        else
        {
            asTexture = !materialXData->elementIsAShader();
        }

        // Create the MaterialX node
        MObject node = _dgModifier.createNode(
            asTexture ? MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID
            : MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID
        );

        // Generate a valid Maya node name from the path string
        {
            const std::string nodeName = mx::createValidName(elementPath.asChar());
            _dgModifier.renameNode(node, nodeName.c_str());
        }

        ::registerFragment(*materialXData, ogsXmlFileName.asChar());

        MFnDependencyNode depNode(node);
        auto materialXNode = dynamic_cast<MaterialXNode*>(depNode.userNode());
        if (!materialXNode)
        {
            throw mx::Exception("Unexpected DG node type.");
        }

        materialXNode->setData(documentFilePath, elementPath, std::move(materialXData));
        materialXNode->createOutputAttr(_dgModifier);

        _dgModifier.doIt();

        MString message("Created ");
        message += materialXNode->typeName();
        message += " node: ";
        message += materialXNode->name();
        MGlobal::displayInfo(message);
        return MS::kSuccess;
    }
    catch (std::exception& e)
    {
        MString message("Failed to create MaterialX node: ");
        message += MString(e.what());
        MGlobal::displayError(message);
        return MS::kFailure;
    }
 }

MSyntax CreateMaterialXNodeCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kDocumentFlag, MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME.asChar(), MSyntax::kString);
	syntax.addFlag(kElementFlag, MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME.asChar(), MSyntax::kString);
    syntax.addFlag(kOgsXmlFlag, kOgsXmlFlagLong, MSyntax::kString);
    syntax.addFlag(kTextureFlag, kTextureFlagLong, MSyntax::kBoolean);
	return syntax;
}

void* CreateMaterialXNodeCmd::creator()
{
	return new CreateMaterialXNodeCmd();
}
