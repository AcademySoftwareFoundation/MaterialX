#ifndef MATERIALX_SURFACE_OVERRIDE_H
#define MATERIALX_SURFACE_OVERRIDE_H

#include "MaterialXShadingNodeImpl.h"

#include <maya/MPxSurfaceShadingNodeOverride.h>

/// VP2 surface shading node override
class MaterialXSurfaceOverride
    : public MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>
{
  public:
    static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject&);

    MString transparencyParameter() const override;

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

  private:
    /// Inheriting the constructor from the base class.
    using MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>::MaterialXShadingNodeImpl;
};

#endif
