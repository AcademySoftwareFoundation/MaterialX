#ifndef MATERIALX_MAYA_SHADINGNODEOVERRIDES_H
#define MATERIALX_MAYA_SHADINGNODEOVERRIDES_H

/// @file
/// Maya VP2 shading node overrides.

#include <maya/MViewport2Renderer.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>

namespace MaterialXMaya
{

/// Base class for surface and texture shading node overrides, templated since
/// they need to derive from different Maya API classes.
///
template <class BASE> class ShadingNodeOverride : public BASE
{
  public:
    MHWRender::DrawAPI supportedDrawAPIs() const override
    {
#ifdef MATERIALX_BUILD_CROSS
        return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile | MHWRender::kDirectX11;
#else
        return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
#endif
    }

    /// Return the name of the OGS fragment used for this shading node.
    MString fragmentName() const override;

    /// Set VP2 shader parameters based on MaterialX values and bind texture
    /// resources.
    void updateShader(MHWRender::MShaderInstance&, const MHWRender::MAttributeParameterMappingList&) override;

    /// Determine if changing the value of the specified plug should refresh
    /// the shader in the viewport.
    bool valueChangeRequiresFragmentRebuild(const MPlug*) const override;

  protected:
    ~ShadingNodeOverride() override;

    /// @param obj The corresponding MaterialX DG node.
    ShadingNodeOverride(const MObject& obj);

    MObject _object;
};

/// VP2 surface shading node override.
/// Implements shading for a MaterialXSurfaceNode.
//
class SurfaceOverride : public ShadingNodeOverride<MHWRender::MPxSurfaceShadingNodeOverride>
{
public:
    static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject&);

    /// Specifies the name of the MaterialXSurfaceNode input attribute used by
    /// Maya to determine if the surface is transparent.
    MString transparencyParameter() const override;

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

private:
    /// Inheriting the constructor from the base class.
    using ShadingNodeOverride<MHWRender::MPxSurfaceShadingNodeOverride>::ShadingNodeOverride;
};

/// VP2 texture shading node override.
/// Implements shading for a MaterialXTextureNode
///
class TextureOverride : public ShadingNodeOverride<MHWRender::MPxShadingNodeOverride>
{
public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

protected:
    /// Inheriting the constructor from the base class.
    using ShadingNodeOverride<MHWRender::MPxShadingNodeOverride>::ShadingNodeOverride;
};

} // namespace MaterialXMaya

#endif
