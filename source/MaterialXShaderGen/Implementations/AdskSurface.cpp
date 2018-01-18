#include <MaterialXShaderGen/Implementations/AdskSurface.h>
#include <MaterialXShaderGen/SgNode.h>

namespace MaterialX
{

namespace {

    static const string kLanguage = "glsl";
    static const string kTarget = "ogsfx";

}

SgImplementationPtr AdskSurfaceOgsFx::creator()
{
    return std::make_shared<AdskSurfaceOgsFx>();
}

const string& AdskSurfaceOgsFx::getLanguage() const
{
    return kLanguage;
}

const string& AdskSurfaceOgsFx::getTarget() const
{
    return kTarget;
}

bool AdskSurfaceOgsFx::isTransparent(const SgNode& node) const
{
    if (node.getInput("opacity"))
    {
        MaterialX::ValuePtr value = node.getInput("opacity")->value;
        if (value)
        {
            try
            {
                MaterialX::Color3 color3Value = value->asA<MaterialX::Color3>();
                return color3Value[0] < 1.0 || color3Value[1] < 1.0 || color3Value[2] < 1.0;
            }
            catch(Exception)
            {
                return false;
            }
        }
    }
    return false;
}

} // namespace MaterialX
