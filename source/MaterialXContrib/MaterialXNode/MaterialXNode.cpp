#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"
#include "MayaUtil.h"
#include "MaterialXData.h"

#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

namespace mx = MaterialX;

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

MObject MaterialXSurfaceNode::VP2_TRANSPARENCY_ATTRIBUTE;

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

    static const MString outColorName(mx::OgsXmlGenerator::OUTPUT_NAME.c_str());

    MFnNumericAttribute nAttr;
    MObject outColorR = nAttr.create(outColorName + "R", "ocr", MFnNumericData::kFloat, 0.0);
    MObject outColorG = nAttr.create(outColorName + "G", "ocg", MFnNumericData::kFloat, 0.0);
    MObject outColorB = nAttr.create(outColorName + "B", "ocb", MFnNumericData::kFloat, 0.0);
    OUT_ATTRIBUTE = nAttr.create(outColorName, "oc", outColorR, outColorG, outColorB);

    CHECK_MSTATUS(nAttr.setKeyable(false));
    CHECK_MSTATUS(nAttr.setStorable(false));
    CHECK_MSTATUS(nAttr.setReadable(true));
    CHECK_MSTATUS(nAttr.setWritable(false));
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
    auto createAndRegisterFragment = [this]()
    {
        try
        {
            if (_documentFilePath.length() == 0)
            {
                return;
            }

            if (!_document)
            {
                _document = MaterialXMaya::loadDocument(
                    _documentFilePath.asChar(), Plugin::instance().getLibrarySearchPath()
                );
            };

            if (_elementPath.length() == 0)
            {
                // When an empty element path is passed to MaterialXData's
                // constructor, the first renderable element is selected, which is
                // a handy feature when creating the node with the command.
                // However this automatic behavior would complicate state
                // transitions on attribute edits after the node has been created.
                // So if the element path attribute is empty, bail early and don't
                // attempt to create a MaterialXData.
                //
                return;
            }

            _materialXData.reset(new MaterialXData(
                _document,
                _elementPath.asChar(),
                Plugin::instance().getLibrarySearchPath()
            ));

            MaterialXMaya::registerFragment(
                _materialXData->getFragmentName(), _materialXData->getFragmentSource()
            );
        }
        catch (std::exception& e)
        {
            MString message("Failed to create shader for ");
            message += typeName();
            message += " '";
            message += name();
            message += "': ";
            message += e.what();
            MGlobal::displayError(message);
        }
    };

    if (plug == DOCUMENT_ATTRIBUTE)
    {
        const MString& value = dataHandle.asString();
        if (_documentFilePath == value)
        {
            return true;
        }

        _documentFilePath = value;
        _document.reset();
        _materialXData.reset();
        
        createAndRegisterFragment();
    }
    else if (plug == ELEMENT_ATTRIBUTE)
    {
        const MString& value = dataHandle.asString();
        if (_elementPath == value)
        {
            return true;
        }

        _elementPath = value;
        _materialXData.reset();

        createAndRegisterFragment();
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

    if (_materialXData)
    {
        _document = _materialXData->getDocument();
    }
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

    MFnNumericAttribute nAttr;
    VP2_TRANSPARENCY_ATTRIBUTE = nAttr.create(
        mx::OgsXmlGenerator::VP_TRANSPARENCY_NAME.c_str(),
        "vp2t",
        MFnNumericData::kFloat,
        0.0
    );

    CHECK_MSTATUS(nAttr.setInternal(true));
    CHECK_MSTATUS(nAttr.setKeyable(false));

    // otherwise Maya refuses to map it to the eponymous fragment input
    CHECK_MSTATUS(nAttr.setWritable(true));

    CHECK_MSTATUS(nAttr.setHidden(true));
    CHECK_MSTATUS(nAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(addAttribute(VP2_TRANSPARENCY_ATTRIBUTE));

    return MS::kSuccess;
}

bool
MaterialXSurfaceNode::getInternalValue(const MPlug& plug, MDataHandle& dataHandle)
{
    if (plug == VP2_TRANSPARENCY_ATTRIBUTE)
    {
        dataHandle.set(_materialXData && _materialXData->isTransparent() ? 0.5f : 0.0f);
        return true;
    }
    else
    {
        return MaterialXNode::getInternalValue(plug, dataHandle);
    }
}