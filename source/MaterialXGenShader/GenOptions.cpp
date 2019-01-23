#include <MaterialXGenShader/GenOptions.h>

namespace MaterialX
{

GenOptions::GenOptions()
    : shaderInterfaceType(SHADER_INTERFACE_COMPLETE)
    , hwTransparency(false)
    , hwSpecularEnvironmentMethod(SPECULAR_ENVIRONMENT_PREFILTER)
{
}

}
