#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"
#include "MayaUtil.h"

#include <MaterialXGenOgsXml/OgsFragment.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

const MTypeId MaterialXNode::MATERIALX_NODE_TYPEID = 0x00042402;
const MString MaterialXNode::MATERIALX_NODE_TYPENAME = "MaterialXNode";

const MString MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME = "documentFilePath";
const MString MaterialXNode::DOCUMENT_ATTRIBUTE_SHORT_NAME = "doc";
MObject MaterialXNode::DOCUMENT_ATTRIBUTE;

const MString MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME = "elementPath";
const MString MaterialXNode::ELEMENT_ATTRIBUTE_SHORT_NAME = "ele";
MObject MaterialXNode::ELEMENT_ATTRIBUTE;

const MString MaterialXNode::ENV_RADIANCE_ATTRIBUTE_LONG_NAME = "environmentRadianceMap";
const MString MaterialXNode::ENV_RADIANCE_ATTRIBUTE_SHORT_NAME = "envr";
MObject MaterialXNode::ENV_RADIANCE_ATTRIBUTE;

const MString MaterialXNode::ENV_IRRADIANCE_ATTRIBUTE_LONG_NAME = "environmentIrradianceMap";
const MString MaterialXNode::ENV_IRRADIANCE_ATTRIBUTE_SHORT_NAME = "envi";
MObject MaterialXNode::ENV_IRRADIANCE_ATTRIBUTE;

MObject MaterialXNode::OUT_ATTRIBUTE;

const MTypeId MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID = 0x00042403;
const MString MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPENAME = "MaterialXTexture";

const MTypeId MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID = 0x00042404;
const MString MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPENAME = "MaterialXSurface";

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

    MObject emptyStringObject = stringData.create();

    DOCUMENT_ATTRIBUTE = typedAttr.create(  DOCUMENT_ATTRIBUTE_LONG_NAME,
                                            DOCUMENT_ATTRIBUTE_SHORT_NAME,
                                            MFnData::kString,
                                            emptyStringObject );
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(typedAttr.setUsedAsFilename(true));
    CHECK_MSTATUS(addAttribute(DOCUMENT_ATTRIBUTE));

    ELEMENT_ATTRIBUTE = typedAttr.create(   ELEMENT_ATTRIBUTE_LONG_NAME,
                                            ELEMENT_ATTRIBUTE_SHORT_NAME,
                                            MFnData::kString,
                                            emptyStringObject );
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(addAttribute(ELEMENT_ATTRIBUTE));

    ENV_RADIANCE_ATTRIBUTE = typedAttr.create(  ENV_RADIANCE_ATTRIBUTE_LONG_NAME,
                                                ENV_RADIANCE_ATTRIBUTE_SHORT_NAME,
                                                MFnData::kString,
                                                emptyStringObject );
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(addAttribute(ENV_RADIANCE_ATTRIBUTE));

    ENV_IRRADIANCE_ATTRIBUTE = typedAttr.create(ENV_IRRADIANCE_ATTRIBUTE_LONG_NAME,
                                                ENV_IRRADIANCE_ATTRIBUTE_SHORT_NAME,
                                                MFnData::kString,
                                                emptyStringObject);
    CHECK_MSTATUS(typedAttr.setInternal(true));
    CHECK_MSTATUS(typedAttr.setKeyable(false));
    CHECK_MSTATUS(typedAttr.setAffectsAppearance(true));
    CHECK_MSTATUS(addAttribute(ENV_IRRADIANCE_ATTRIBUTE));

    static const MString outColorName(mx::OgsXmlGenerator::OUTPUT_NAME.c_str());

    MFnNumericAttribute nAttr;
    MObject outColorR = nAttr.create(outColorName + "R", "ocr", MFnNumericData::kFloat, 0.0);
    MObject outColorG = nAttr.create(outColorName + "G", "ocg", MFnNumericData::kFloat, 0.0);
    MObject outColorB = nAttr.create(outColorName + "B", "ocb", MFnNumericData::kFloat, 0.0);
    OUT_ATTRIBUTE = nAttr.create(outColorName, "oc", outColorR, outColorG, outColorB);

    CHECK_MSTATUS(nAttr.setKeyable(false))
    CHECK_MSTATUS(nAttr.setStorable(false))
    CHECK_MSTATUS(nAttr.setReadable(true))
    CHECK_MSTATUS(nAttr.setWritable(false))
    CHECK_MSTATUS(nAttr.setUsedAsColor(true))

    CHECK_MSTATUS(addAttribute(OUT_ATTRIBUTE))

    CHECK_MSTATUS(attributeAffects(DOCUMENT_ATTRIBUTE, OUT_ATTRIBUTE))
    CHECK_MSTATUS(attributeAffects(ELEMENT_ATTRIBUTE, OUT_ATTRIBUTE))
    CHECK_MSTATUS(attributeAffects(ENV_RADIANCE_ATTRIBUTE, OUT_ATTRIBUTE))
    CHECK_MSTATUS(attributeAffects(ENV_IRRADIANCE_ATTRIBUTE, OUT_ATTRIBUTE))

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

void MaterialXNode::createAndRegisterFragment()
{
    try
    {
        if (_documentFilePath.length() == 0)
        {
            return;
        }

        if (!_document)
        {
            _document =
                MaterialXUtil::loadDocument(_documentFilePath.asChar(), Plugin::instance().getLibraryDocument());
        }

        if (_elementPath.length() == 0)
        {
            // If the element path attribute is empty, bail early and don't
            // attempt to create an OgsFragment.
            //
            return;
        }

        _ogsFragment.reset(new OgsFragment(_document->getDescendant(_elementPath.asChar()),
                                           Plugin::instance().getLibrarySearchPath()));

        MayaUtil::registerFragment(_ogsFragment->getFragmentName(), _ogsFragment->getFragmentSource(),
                                   _ogsFragment->getLightRigName(), _ogsFragment->getLightRigSource());
    }
    catch (std::exception& e)
    {
        MString msg("Failed to create shader for ");
        msg += typeName();
        msg += " '";
        msg += name();
        msg += "': ";
        msg += e.what();
        MGlobal::displayError(msg);
    }
}

void MaterialXNode::reloadDocument()
{
    if (_documentFilePath.length() == 0)
    {
        return;
    }

    MPlug documentPlug(thisMObject(), DOCUMENT_ATTRIBUTE);
    const MString documentFilePath = _documentFilePath;
    documentPlug.setValue("");
    documentPlug.setValue(documentFilePath);
}

bool MaterialXNode::setInternalValue(const MPlug& plug, const MDataHandle& dataHandle)
{
    if (plug == DOCUMENT_ATTRIBUTE)
    {
        const MString& value = dataHandle.asString();
        if (_documentFilePath == value)
        {
            return true;
        }

        _documentFilePath = value;
        _document.reset();
        _ogsFragment.reset();

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
        _ogsFragment.reset();

        createAndRegisterFragment();
    }
    else if (plug == ENV_RADIANCE_ATTRIBUTE)
    {
        _envRadianceFileName = dataHandle.asString();
    }
    else if (plug == ENV_IRRADIANCE_ATTRIBUTE)
    {
        _envIrradianceFileName = dataHandle.asString();
    }
    else
    {
        return MPxNode::setInternalValue(plug, dataHandle);
    }

    // Even if we fail to create a valid shader, we allow attribute values to be changed.
    return true;
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
    else if (plug == ENV_RADIANCE_ATTRIBUTE)
    {
        dataHandle.set(_envRadianceFileName);
    }
    else if (plug == ENV_IRRADIANCE_ATTRIBUTE)
    {
        dataHandle.set(_envIrradianceFileName);
    }
    else
    {
        return MPxNode::getInternalValue(plug, dataHandle);
    }

    return true;
}

void MaterialXNode::setData(const MString& documentFilePath,
                            const MString& elementPath,
                            const MString& envRadianceFileName,
                            const MString& envIrradianceFileName,
                            std::unique_ptr<OgsFragment>&& ogsFragment)
{
    _documentFilePath = documentFilePath;
    _elementPath = elementPath;

    // If the command doesn't provide environment map file names,
    // leave them set to the default values.
    if (envRadianceFileName.length() > 0)
    {
        _envRadianceFileName = envRadianceFileName;
    }

    if (envIrradianceFileName.length() > 0)
    {
        _envIrradianceFileName = envIrradianceFileName;
    }

    _ogsFragment = std::move(ogsFragment);
    if (_ogsFragment)
    {
        _document = _ogsFragment->getDocument();
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
    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME))
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
    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME))

    MFnNumericAttribute nAttr;
    VP2_TRANSPARENCY_ATTRIBUTE =
        nAttr.create(mx::OgsXmlGenerator::VP_TRANSPARENCY_NAME.c_str(), "vp2t", MFnNumericData::kFloat, 0.0);

    CHECK_MSTATUS(nAttr.setInternal(true))
    CHECK_MSTATUS(nAttr.setKeyable(false))

    // otherwise Maya refuses to map it to the eponymous fragment input
    CHECK_MSTATUS(nAttr.setWritable(true))

    CHECK_MSTATUS(nAttr.setHidden(true))
    CHECK_MSTATUS(nAttr.setAffectsAppearance(true))
    CHECK_MSTATUS(addAttribute(VP2_TRANSPARENCY_ATTRIBUTE))

    return MS::kSuccess;
}

bool MaterialXSurfaceNode::getInternalValue(const MPlug& plug, MDataHandle& dataHandle)
{
    if (plug == VP2_TRANSPARENCY_ATTRIBUTE)
    {
        dataHandle.set(_ogsFragment && _ogsFragment->isTransparent() ? 0.5f : 0.0f);
        return true;
    }
    else
    {
        return MaterialXNode::getInternalValue(plug, dataHandle);
    }
}

} // namespace MaterialXMaya
