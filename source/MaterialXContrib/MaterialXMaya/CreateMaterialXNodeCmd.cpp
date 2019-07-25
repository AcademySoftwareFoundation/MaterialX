#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "MaterialXData.h"
#include "MaterialXUtil.h"
#include "MayaUtil.h"
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

namespace MaterialXMaya
{

namespace
{
const char* const kDocumentFlag     = "d";
const char* const kElementFlag      = "e";

const char* const kAsTextureFlag      = "t";
const char* const kAsTextureFlagLong  = "asTexture";

const char* const kOgsXmlFlag       = "x";
const char* const kOgsXmlFlagLong   = "ogsXml";

const char* const kEnvRadianceFlag = "er";
const char* const kEnvRadianceFlagLong = "envRadiance";

const char* const kEnvIrradianceFlag = "ei";
const char* const kEnvIrradianceFlagLong = "envIrradiance";

void registerDebugFragment(const std::string& ogsXmlFileName)
{
    // Use the fragment from an explicitly provided file instead of
    // the one generated in OgsFragment - this can be used for debugging.
    //
    MHWRender::MRenderer* const theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* const fragmentManager =
        theRenderer ? theRenderer->getFragmentManager() : nullptr;

    if (!fragmentManager)
    {
        throw std::runtime_error("Failed to get the VP2 fragment manager");
    }

    // If explicit XML file specified use it.
    const mx::FilePath ogsXmlPath =
        Plugin::instance().getResourceSearchPath().find(ogsXmlFileName);

    if (ogsXmlPath.isEmpty())
    {
        throw mx::Exception("Failed to find the explicitly provided fragment file.");
    }

    constexpr bool hidden = false;
    const MString registeredFragment =
        fragmentManager->addShadeFragmentFromFile(ogsXmlPath.asString().c_str(), hidden);

    if (registeredFragment.length() == 0)
    {
        throw mx::Exception("Failed to register debug fragment from file.");
    }
}
} // anonymous namespace

MString CreateMaterialXNodeCmd::NAME("createMaterialXNode");

CreateMaterialXNodeCmd::CreateMaterialXNodeCmd()
{
}

CreateMaterialXNodeCmd::~CreateMaterialXNodeCmd()
{
}

std::string CreateMaterialXNodeCmd::createNode( mx::DocumentPtr document,
                                                mx::TypedElementPtr renderableElement,
                                                bool createAsTexture,
                                                const MString& documentFilePath,
                                                const mx::FileSearchPath& searchPath,
                                                const MString& envRadianceFileName,
                                                const MString& envIrradianceFileName )
{
    std::unique_ptr<OgsFragment> ogsFragment{ new OgsFragment(document,
                                                              renderableElement,
                                                              searchPath) };

    MayaUtil::registerFragment(
        ogsFragment->getFragmentName(), ogsFragment->getFragmentSource()
    );

    // Decide what kind of node we want to create
    if (!createAsTexture)
    {
        createAsTexture = !ogsFragment->elementIsAShader();
    }

    // Create the MaterialX node
    MObject node = _dgModifier.createNode(createAsTexture ? 
                                          MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID :
                                          MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID);

    // Generate a valid Maya node name from the path string
    std::string renderableElementPath = renderableElement->getNamePath();
    const std::string nodeName = mx::createValidName(renderableElementPath);
    _dgModifier.renameNode(node, nodeName.c_str());

    MFnDependencyNode depNode(node);
    auto materialXNode = dynamic_cast<MaterialXNode*>(depNode.userNode());
    if (!materialXNode)
    {
        throw mx::Exception("Unexpected DG node type.");
    }

    materialXNode->setData( documentFilePath, renderableElementPath.c_str(),
                            envRadianceFileName, envIrradianceFileName, std::move(ogsFragment) );

    return nodeName;
}

MStatus CreateMaterialXNodeCmd::doIt( const MArgList &args )
{
    // Parse the shader node
    //
    MArgParser parser(syntax(), args);

    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status)
    {
        return status;
    }

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

        mx::DocumentPtr document = MaterialXUtil::loadDocument(
            documentFilePath.asChar(), Plugin::instance().getLibrarySearchPath()
        );

        // Find renderables in the document
        std::vector<mx::TypedElementPtr> renderableElements;
        mx::findRenderableElements(document, renderableElements);
        if (renderableElements.empty())
        {
            throw mx::Exception("There are no renderable elements in the document.");
        }

        // If there is a specific element specified set the renderables list to
        // be just the single element if it's considered renderable.
        MString elementPath;
        if (parser.isFlagSet(kElementFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kElementFlag, 0, elementPath));
            if (elementPath.length())
            {
                mx::TypedElementPtr desiredElement =
                    MaterialXUtil::getRenderableElement(document, renderableElements, elementPath.asChar());

                if (desiredElement)
                {
                    renderableElements.clear();
                    renderableElements.push_back(desiredElement);
                }
            }
        }

        MString ogsXmlFileName;
        if (parser.isFlagSet(kOgsXmlFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kOgsXmlFlag, 0, ogsXmlFileName));
        }

        if (ogsXmlFileName.length() > 0)
        {
            registerDebugFragment(ogsXmlFileName.asChar());
        }

        bool createAsTexture = false;
        if (parser.isFlagSet(kAsTextureFlag))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kAsTextureFlag, 0, createAsTexture));
        }

        MString envRadianceFileName;
        if (parser.isFlagSet(kEnvRadianceFlagLong))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kEnvRadianceFlagLong, 0, envRadianceFileName));
        }

        MString envIrradianceFileName;
        if (parser.isFlagSet(kEnvIrradianceFlagLong))
        {
            CHECK_MSTATUS(argData.getFlagArgument(kEnvIrradianceFlagLong, 0, envIrradianceFileName));
        }

        const mx::FileSearchPath& searchPath = Plugin::instance().getLibrarySearchPath();
        MStringArray nodeNames;
        for (auto renderableElement : renderableElements)
        {
            std::string nodeName = createNode(  document,
                                                renderableElement,
                                                createAsTexture,
                                                documentFilePath,
                                                searchPath,
                                                envRadianceFileName,
                                                envIrradianceFileName );
            nodeNames.append(nodeName.c_str());
        }

        status = _dgModifier.doIt();
        if (!status)
        {
            unsigned int nodeNameCount = nodeNames.length();
            std::string nodeString;
            if (nodeNameCount)
            {
                nodeString = nodeNames[0].asChar();
                for (unsigned int i = 1; i < nodeNameCount; i++)
                {
                    nodeString += std::string(",") + nodeNames[i].asChar();
                }
            }
            throw mx::Exception(nodeString);
        }
        setResult(nodeNames);
        return status;
    }
    catch (std::exception& e)
    {
        MString message("Failed to create MaterialX nodes: ");
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
    syntax.addFlag(kAsTextureFlag, kAsTextureFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kEnvRadianceFlag, kEnvRadianceFlagLong, MSyntax::kString);
    syntax.addFlag(kEnvIrradianceFlag, kEnvIrradianceFlagLong, MSyntax::kString);
    return syntax;
}

void* CreateMaterialXNodeCmd::creator()
{
    return new CreateMaterialXNodeCmd();
}

} // namespace MaterialXMaya
