#ifndef MATERIALX_TEXTURE_OVERRIDE_H
#define MATERIALX_TEXTURE_OVERRIDE_H

#include "MaterialXShadingNodeImpl.h"

#include <maya/MPxShadingNodeOverride.h>

/// VP2 texture shading node override
class MaterialXTextureOverride
    : public MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>
{
  public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

  protected:
    /// Inheriting the constructor from the base class.
    using MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>::MaterialXShadingNodeImpl;
};

#endif
