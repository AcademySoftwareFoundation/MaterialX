#ifndef MATERIALX_SHADINGNODE_IMPL_H
#define MATERIALX_SHADINGNODE_IMPL_H

#include <maya/MViewport2Renderer.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>

namespace MaterialXMaya
{

template <class BASE>
class ShadingNodeOverride : public BASE
{
  public:
    MHWRender::DrawAPI supportedDrawAPIs() const override
    {
        return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
    }

    MString fragmentName() const override;

    void updateDG() override;

    void updateShader(
        MHWRender::MShaderInstance&,
        const MHWRender::MAttributeParameterMappingList&
    ) override;

    bool valueChangeRequiresFragmentRebuild(const MPlug*) const override;

  protected:
    ~ShadingNodeOverride() override;

    ShadingNodeOverride(const MObject&);

    MObject _object;
};

/// VP2 surface shading node override
//
class SurfaceOverride
    : public ShadingNodeOverride<MHWRender::MPxSurfaceShadingNodeOverride>
{
public:
    static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject&);

    MString transparencyParameter() const override;

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

private:
    /// Inheriting the constructor from the base class.
    using ShadingNodeOverride<MHWRender::MPxSurfaceShadingNodeOverride>::ShadingNodeOverride;
};

/// VP2 texture shading node override
//
class TextureOverride
    : public ShadingNodeOverride<MHWRender::MPxShadingNodeOverride>
{
public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

protected:
    /// Inheriting the constructor from the base class.
    using ShadingNodeOverride<MHWRender::MPxShadingNodeOverride>::ShadingNodeOverride;
};


} // namespace MaterialXMaya

#endif /* MATERIALX_SHADINGNODE_IMPL_H */
