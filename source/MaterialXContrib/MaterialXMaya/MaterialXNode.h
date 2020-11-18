#ifndef MATERIALX_MAYA_MATERIALXNODE_H
#define MATERIALX_MAYA_MATERIALXNODE_H

/// @file
/// Maya shading node classes.

#include <MaterialXCore/Document.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

class OgsFragment;

/// @class MaterialXNode
/// The base class for both surface and texture shading nodes.
class MaterialXNode : public MPxNode
{
  public:
    MaterialXNode();
    ~MaterialXNode() override;

    /// @name Maya API methods
    /// @{
    static void* creator();
    static MStatus initialize();

    MTypeId	typeId() const override;
    SchedulingType schedulingType() const override;

    bool getInternalValue(const MPlug&, MDataHandle&) override;
    bool setInternalValue(const MPlug&, const MDataHandle&) override;
    /// @}

    /// Set the attribute values and the OGS fragment owned by the node
    /// when creating the node with CreateMaterialXNodeCmd.
    /// @param documentFilePath The path to the MaterialX document file.
    /// @param elementPath The path to the MaterialX element within the document.
    /// @param envRadianceFileName The file name of the environment map to use for specular shading.
    /// @param envIrradianceFileName The file name of the environment map to use for diffuse shading.
    /// @param ogsFragment An object representing an OGS shade fragment created by the command for this node.
    ///     The ownership of the fragment is transfered to the node.
    ///
    void setData(   const MString& documentFilePath,
                    const MString& elementPath,
                    const MString& envRadianceFileName,
                    const MString& envIrradianceFileName,
                    std::unique_ptr<OgsFragment>&& ogsFragment );

    /// Reloads the document, regenerates the OGS fragment and refreshes the node in the viewport.
    void reloadDocument();

    /// Return the OGS fragment.
    const OgsFragment* getOgsFragment() const
    {
        return _ogsFragment.get();
    }

    /// Return the document file path.
    const MString& getDocumentFilePath() const
    {
        return _documentFilePath;
    }

    /// Return the file name of the environment map to use for specular shading.
    const MString& getEnvRadianceFileName() const
    {
        return _envRadianceFileName;
    }

    /// Return the file name of the environment map to use for diffuse shading.
    const MString& getEnvIrradianceFileName() const
    {
        return _envIrradianceFileName;
    }

    static const MTypeId MATERIALX_NODE_TYPEID;
    static const MString MATERIALX_NODE_TYPENAME;

    /// @name Attribute holding the path to the MaterialX document file.
    /// @{
    static const MString DOCUMENT_ATTRIBUTE_LONG_NAME;
    static const MString DOCUMENT_ATTRIBUTE_SHORT_NAME;
    static MObject DOCUMENT_ATTRIBUTE;
    /// @}

    /// @name Attribute holding the path to the MaterialX element within the document.
    /// @{
    static const MString ELEMENT_ATTRIBUTE_LONG_NAME;
    static const MString ELEMENT_ATTRIBUTE_SHORT_NAME;
    static MObject ELEMENT_ATTRIBUTE;
    /// @}

    /// @name Attribute holding the file name of the environment map to use for specular shading.
    /// @{
    static const MString ENV_RADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_RADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_RADIANCE_ATTRIBUTE;
    /// @}

    /// @name Attribute holding the file name of the environment map to use for diffuse shading.
    /// @{
    static const MString ENV_IRRADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_IRRADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_IRRADIANCE_ATTRIBUTE;
    /// @}

    /// The output color attribute, required to correctly connect the node to a shading group.
    /// Maps onto an output parameter with the same name in the generated OGS shade fragment.
    static MObject OUT_ATTRIBUTE;

  protected:
    /// An object representing an OGS shade fragment generated for this node.
    std::unique_ptr<OgsFragment> _ogsFragment;

  private:
    /// If the current document pointer is null, creates a new document based
    /// on the document file path attribute.
    /// (Re)generates the OGS fragment based on the element path attribute and
    /// registers it in VP2 under a unique name.
    void createAndRegisterFragment();

    /// @name Storage for internal Maya attributes.
    /// @{
    MString _documentFilePath, _elementPath;

    MString _envRadianceFileName = "piazza_bologni.hdr";
    MString _envIrradianceFileName = "piazza_bologni_diffuse.hdr";
    /// @}

    /// The OgsFragment keeps a shared pointer to the document it was created
    /// from but we also keep another shared pointer here to avoid reloading
    /// the document when the element path becomes invalid and the OgsFragment
    /// doesn't exist.
    mx::DocumentPtr _document;
};

/// @class MaterialXTextureNode
/// MaterialX texture shading node, accessible from the Hypershade under the
/// 2D Textures category. The output attribute can be connected to an input on
/// a surface node.
class MaterialXTextureNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    static const MTypeId MATERIALX_TEXTURE_NODE_TYPEID;
    static const MString MATERIALX_TEXTURE_NODE_TYPENAME;
};

/// @class MaterialXSurfaceNode
/// MaterialX surface shading node, accessible from the Hypershade under the
/// Surface category and available for assignment to a shape via the Marking
/// Menu.
class MaterialXSurfaceNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    bool getInternalValue(const MPlug&, MDataHandle&) override;

    static const MTypeId MATERIALX_SURFACE_NODE_TYPEID;
    static const MString MATERIALX_SURFACE_NODE_TYPENAME;

    /// The input transparency attribute, registered in VP2 as such by
    /// MaterialXMaya::SurfaceOverride::transparencyParameter().
    /// Stores a dummy value set above 0 if the surface is internally
    /// determined by MaterialX to be transparent. This makes VP2 render this
    /// surface in transparency passes.
    static MObject VP2_TRANSPARENCY_ATTRIBUTE;
};

} // namespace MaterialXMaya

#endif /* MATERIALX_NODE_H */
