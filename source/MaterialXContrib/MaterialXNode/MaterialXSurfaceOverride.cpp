#include "MaterialXSurfaceOverride.h"

const MString
    MaterialXSurfaceOverride::REGISTRANT_ID = "materialXSurface",
    MaterialXSurfaceOverride::DRAW_CLASSIFICATION = "drawdb/shader/surface/materialX";

MHWRender::MPxSurfaceShadingNodeOverride*
MaterialXSurfaceOverride::creator(const MObject& obj)
{
    std::cout.rdbuf(std::cerr.rdbuf());
    return new MaterialXSurfaceOverride(obj);
}
