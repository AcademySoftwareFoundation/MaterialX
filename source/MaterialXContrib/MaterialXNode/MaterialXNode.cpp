#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MPlugArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <maya/MRenderUtil.h>
#include <maya/MFloatVector.h>

#define MAKE_INPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(true)); \
	CHECK_MSTATUS(attr.setStorable(true)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(false)); \
	CHECK_MSTATUS(attr.setStorable(false)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(false));

const MTypeId MaterialXNode::MATERIALX_NODE_TYPEID(0x00042402);
const MString MaterialXNode::MATERIALX_NODE_TYPENAME("MaterialXNode");

MString MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME("documentFilePath");
MString MaterialXNode::DOCUMENT_ATTRIBUTE_SHORT_NAME("doc");
MObject MaterialXNode::DOCUMENT_ATTRIBUTE;

MString MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME("elementPath");
MString MaterialXNode::ELEMENT_ATTRIBUTE_SHORT_NAME("ele");
MObject MaterialXNode::ELEMENT_ATTRIBUTE;

const MTypeId MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID(0x00042403);
const MString MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPENAME("MaterialXTexture");

const MTypeId MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID(0x00042404);
const MString MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPENAME("MaterialXSurface");

MaterialXNode::MaterialXNode()
{
	std::cout << "MaterialXNode::MaterialXNode" << std::endl;
}

MaterialXNode::~MaterialXNode()
{
	std::cout << "MaterialXNode::~MaterialXNode" << std::endl;
}

void* MaterialXNode::creator()
{
	std::cout.rdbuf(std::cerr.rdbuf());
	std::cout << "MaterialXNode::creator" << std::endl;
	return new MaterialXNode();
}

MStatus MaterialXNode::initialize()
{
	std::cout << "MaterialXNode::initialize" << std::endl;

	MFnTypedAttribute typedAttr;
	MFnStringData stringData;

	MObject theString = stringData.create();

	DOCUMENT_ATTRIBUTE = typedAttr.create(DOCUMENT_ATTRIBUTE_LONG_NAME, DOCUMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(typedAttr.setUsedAsFilename(true));
	CHECK_MSTATUS(addAttribute(DOCUMENT_ATTRIBUTE));

	ELEMENT_ATTRIBUTE = typedAttr.create(ELEMENT_ATTRIBUTE_LONG_NAME, ELEMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
	CHECK_MSTATUS(addAttribute(ELEMENT_ATTRIBUTE));

	return MS::kSuccess;
}

void MaterialXNode::createOutputAttr(MDGModifier& mdgModifier)
{
    if (_materialXData && !_materialXData->getElementPath().empty())
    {
        const MString outputName(MaterialX::OgsXmlGenerator::OUTPUT_NAME.c_str());
        MFnNumericAttribute nAttr;
        _outAttr = nAttr.createColor(outputName, outputName);
        CHECK_MSTATUS(nAttr.setStorable(false));
        CHECK_MSTATUS(nAttr.setInternal(false));
        CHECK_MSTATUS(nAttr.setReadable(true));
        CHECK_MSTATUS(nAttr.setWritable(false));
        CHECK_MSTATUS(nAttr.setCached(true));
        CHECK_MSTATUS(nAttr.setHidden(false));

        mdgModifier.addAttribute(thisMObject(), _outAttr);
    }
}

MStatus MaterialXNode::setDependentsDirty(const MPlug &/*plugBeingDirtied*/, MPlugArray & affectedPlugs)
{
	if (!_outAttr.isNull())
	{
		MPlug outPlug(thisMObject(), _outAttr);
		affectedPlugs.append(outPlug);
	}

	return MS::kSuccess;
}

MTypeId MaterialXNode::typeId() const
{
    return MATERIALX_NODE_TYPEID;
}

MPxNode::SchedulingType MaterialXNode::schedulingType() const
{
	return MPxNode::SchedulingType::kParallel;
}

bool MaterialXNode::getInternalValue(const MPlug& plug, MDataHandle& dataHandle)
{
    if (plug == DOCUMENT_ATTRIBUTE)
    {
        dataHandle.set(_documentFilePath);
    }
    else if (plug == ELEMENT_ATTRIBUTE)
    {
        dataHandle.set(_elementPath);
    }
    else
    {
        return MPxNode::getInternalValue(plug, dataHandle);
    }

    return true;
}

bool MaterialXNode::setInternalValue(const MPlug& plug, const MDataHandle& dataHandle)
{
    auto displayError = [this](const char* error)
    {
        MString message("Failed to create shader for ");
        message += typeName();
        message += ' ';
        message += name();
        message += ": ";
        message += error;
        MGlobal::displayError(message);
    };

    auto loadDocument = [this]()
    {
        return MaterialXMaya::loadDocument(
            _documentFilePath.asChar(), Plugin::instance().getLibrarySearchPath()
        );
    };

    if (plug == DOCUMENT_ATTRIBUTE)
    {
        const MString& value = dataHandle.asString();
        if (_documentFilePath == value)
        {
            return true;
        }

        _documentFilePath = value;
        _materialXData.reset();

        try
        {
            _materialXData.reset( new MaterialXData(
                loadDocument(),
                _elementPath.asChar(),
                Plugin::instance().getLibrarySearchPath()
            ));
        }
        catch (std::exception& e)
        {
            displayError(e.what());
        }
    }
    else if (plug == ELEMENT_ATTRIBUTE)
    {
        const MString& value = dataHandle.asString();
        if (_elementPath == value)
        {
            return true;
        }

        _elementPath = value;
        mx::DocumentPtr document;

        if (_materialXData)
        {
            document = _materialXData->getDocument();
            _materialXData.reset();
        }

        try
        {
            if (!document)
            {
                document = loadDocument();
            }

            _materialXData.reset(new MaterialXData(
                document,
                _elementPath.asChar(),
                Plugin::instance().getLibrarySearchPath()
            ));
        }
        catch (std::exception& e)
        {
            displayError(e.what());
        }
    }
    else
    {
        return MPxNode::setInternalValue(plug, dataHandle);
    }

    // Even if we fail to create a valid shader, we allow attribute values to be changed.
    return true;
}

void MaterialXNode::setData(const MString& documentFilePath,
                            const MString& elementPath,
                            std::unique_ptr<MaterialXData>&& materialXData)
{
    _documentFilePath = documentFilePath;
    _elementPath = elementPath;
    _materialXData = std::move(materialXData);
}

MTypeId MaterialXTextureNode::typeId() const
{
    return MATERIALX_TEXTURE_NODE_TYPEID;
}

void* MaterialXTextureNode::creator()
{
    std::cout.rdbuf(std::cerr.rdbuf());
    std::cout << "MaterialXTextureNode::creator" << std::endl;
    return new MaterialXTextureNode();
}

MStatus MaterialXTextureNode::initialize()
{
    std::cout << "MaterialXTextureNode::initialize" << std::endl;

    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));

    return MS::kSuccess;
}

MTypeId MaterialXSurfaceNode::typeId() const
{
    return MATERIALX_SURFACE_NODE_TYPEID;
}

void* MaterialXSurfaceNode::creator()
{
    std::cout.rdbuf(std::cerr.rdbuf());
    std::cout << "MaterialXSurfaceNode::creator" << std::endl;
    return new MaterialXSurfaceNode();
}

MStatus MaterialXSurfaceNode::initialize()
{
    std::cout << "MaterialXSurfaceNode::initialize" << std::endl;

    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));

    return MS::kSuccess;
}