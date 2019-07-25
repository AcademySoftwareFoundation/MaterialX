#include "MaterialXTextureOverride.h"

namespace MaterialXMaya
{

const MString
    MaterialXTextureOverride::REGISTRANT_ID = "materialXTexture",
    MaterialXTextureOverride::DRAW_CLASSIFICATION = "drawdb/shader/texture/2d/materialX";

MHWRender::MPxShadingNodeOverride* MaterialXTextureOverride::creator(const MObject& obj)
{
	std::cout.rdbuf(std::cerr.rdbuf());
	return new MaterialXTextureOverride(obj);
}

} // namespace MaterialXMaya
