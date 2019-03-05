#include <MaterialXGenShader/GenOptions.h>

namespace MaterialX
{

GenOptions::GenOptions()
    : shaderInterfaceType(SHADER_INTERFACE_COMPLETE)
    , fileTextureVerticalFlip(false)
    , hwTransparency(false)
    , hwSpecularEnvironmentMethod(SPECULAR_ENVIRONMENT_PREFILTER)
    , hwMaxActiveLightSources(3)
{
}

}
