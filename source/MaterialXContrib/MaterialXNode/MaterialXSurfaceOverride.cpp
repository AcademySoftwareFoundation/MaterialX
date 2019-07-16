#include "MaterialXSurfaceOverride.h"
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

namespace mx = MaterialX;

const MString
    MaterialXSurfaceOverride::REGISTRANT_ID = "materialXSurface",
    MaterialXSurfaceOverride::DRAW_CLASSIFICATION = "drawdb/shader/surface/materialX";

MHWRender::MPxSurfaceShadingNodeOverride*
MaterialXSurfaceOverride::creator(const MObject& obj)
{
    std::cout.rdbuf(std::cerr.rdbuf());
    return new MaterialXSurfaceOverride(obj);
}

MString
MaterialXSurfaceOverride::transparencyParameter() const
{
    return mx::OgsXmlGenerator::VP_TRANSPARENCY_NAME.c_str();
}
