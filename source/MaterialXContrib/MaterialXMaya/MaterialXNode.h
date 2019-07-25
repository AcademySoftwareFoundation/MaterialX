#ifndef MATERIALXNODE_H
#define MATERIALXNODE_H

#include <MaterialXCore/Document.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

class MaterialXData;

class MaterialXNode : public MPxNode
{
  public:
    MaterialXNode();
    ~MaterialXNode() override;

    static void* creator();
    static MStatus initialize();

    MTypeId	typeId() const override;
    SchedulingType schedulingType() const override;

    bool getInternalValue(const MPlug&, MDataHandle&) override;
    bool setInternalValue(const MPlug&, const MDataHandle&) override;

    void setData(   const MString& documentFilePath,
                    const MString& elementPath,
                    const MString& envRadianceFileName,
                    const MString& envIrradianceFileName,
                    std::unique_ptr<MaterialXData>&& ); 

    void reloadDocument();

    const MaterialXData* getMaterialXData() const
    {
        return _materialXData.get();
    }

    const MString& getDocumentFilePath() const
    {
        return _documentFilePath;
    }

    const MString& getEnvRadianceFileName() const
    {
        return _envRadianceFileName;
    }

    const MString& getEnvIrradianceFileName() const
    {
        return _envIrradianceFileName;
    }

    static const MTypeId MATERIALX_NODE_TYPEID;
    static const MString MATERIALX_NODE_TYPENAME;

    /// Attribute holding a path to MaterialX document file
    static const MString DOCUMENT_ATTRIBUTE_LONG_NAME;
    static const MString DOCUMENT_ATTRIBUTE_SHORT_NAME;
    static MObject DOCUMENT_ATTRIBUTE;

    /// Attribute holding a MaterialX element name
    static const MString ELEMENT_ATTRIBUTE_LONG_NAME;
    static const MString ELEMENT_ATTRIBUTE_SHORT_NAME;
    static MObject ELEMENT_ATTRIBUTE;

    static const MString ENV_RADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_RADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_RADIANCE_ATTRIBUTE;

    static const MString ENV_IRRADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_IRRADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_IRRADIANCE_ATTRIBUTE;

    static MObject OUT_ATTRIBUTE;

  protected:
    std::unique_ptr<MaterialXData> _materialXData;

  private:
    void createAndRegisterFragment();

    MString _documentFilePath, _elementPath;

    MString _envRadianceFileName = "goegap_4k_dim.hdr";
    MString _envIrradianceFileName = "goegap_4k_dim.convolved.hdr";

    /// MaterialXData keeps a shared pointer to the document but we also keep
    /// another shared pointer here to avoid reloading the document when the
    /// element path becomes invalid and the MaterialXData doesn't exist.
    mx::DocumentPtr _document;
};

class MaterialXTextureNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    static const MTypeId MATERIALX_TEXTURE_NODE_TYPEID;
    static const MString MATERIALX_TEXTURE_NODE_TYPENAME;
};

class MaterialXSurfaceNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    bool getInternalValue(const MPlug&, MDataHandle&) override;

    static const MTypeId MATERIALX_SURFACE_NODE_TYPEID;
    static const MString MATERIALX_SURFACE_NODE_TYPENAME;

    static MObject VP2_TRANSPARENCY_ATTRIBUTE;
};

} // namespace MaterialXMaya

#endif /* MATERIALX_NODE_H */
