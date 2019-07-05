#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"
#include "MayaUtil.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

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

MObject MaterialXNode::OUT_ATTRIBUTE;

const MTypeId MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID(0x00042403);
const MString MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPENAME("MaterialXTexture");

const MTypeId MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID(0x00042404);
const MString MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPENAME("MaterialXSurface");

MaterialXNode::MaterialXNode()
{
}

MaterialXNode::~MaterialXNode()
{
}

void* MaterialXNode::creator()
{
    return new MaterialXNode();
}

MStatus MaterialXNode::initialize()
{
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

    static const MString outColorName(MaterialX::OgsXmlGenerator::OUTPUT_NAME.c_str());

    MFnNumericAttribute nAttr;
    MObject outColorR = nAttr.create(outColorName + "R", "ocr", MFnNumericData::kFloat, 0.0);
    MObject outColorG = nAttr.create(outColorName + "G", "ocg", MFnNumericData::kFloat, 0.0);
    MObject outColorB = nAttr.create(outColorName + "B", "ocb", MFnNumericData::kFloat, 0.0);
    OUT_ATTRIBUTE = nAttr.create(outColorName, "oc", outColorR, outColorG, outColorB);
    MAKE_OUTPUT(nAttr);
    CHECK_MSTATUS(nAttr.setUsedAsColor(true));

    CHECK_MSTATUS(addAttribute(OUT_ATTRIBUTE));

    CHECK_MSTATUS(attributeAffects(DOCUMENT_ATTRIBUTE, OUT_ATTRIBUTE));
    CHECK_MSTATUS(attributeAffects(ELEMENT_ATTRIBUTE, OUT_ATTRIBUTE));

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
        message += " '";
        message += name();
        message += "': ";
        message += error;
        MGlobal::displayError(message);
    };

    auto loadDocument = [this]()
    {
        return MaterialXMaya::loadDocument(
            _documentFilePath.asChar(), Plugin::instance().getLibrarySearchPath()
        );
    };

    auto createAndRegister = [this](mx::DocumentPtr document)
    {
        _materialXData.reset(new MaterialXData(
            document,
            _elementPath.asChar(),
            Plugin::instance().getLibrarySearchPath()
        ));

        MaterialXMaya::registerFragment(
            _materialXData->getFragmentName(), _materialXData->getFragmentSource()
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
            createAndRegister(loadDocument());
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

            createAndRegister(document);
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
    return new MaterialXTextureNode();
}

MStatus MaterialXTextureNode::initialize()
{
    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));
    return MS::kSuccess;
}

MTypeId MaterialXSurfaceNode::typeId() const
{
    return MATERIALX_SURFACE_NODE_TYPEID;
}

void* MaterialXSurfaceNode::creator()
{
    return new MaterialXSurfaceNode();
}

MStatus MaterialXSurfaceNode::initialize()
{
    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));

    return MS::kSuccess;
}