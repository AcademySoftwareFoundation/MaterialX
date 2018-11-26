#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

const string DefaultColorManagementSystem::CMS_NAME = "default_cms";


string DefaultColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform)
{
    return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_" + _language;
}

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(ShaderGenerator& shadergen)
{
    DefaultColorManagementSystemPtr result(new DefaultColorManagementSystem(shadergen));
    return result;
}

DefaultColorManagementSystem::DefaultColorManagementSystem(ShaderGenerator& shadergen)
    : ColorManagementSystem(shadergen, MaterialX::EMPTY_STRING)
{
    _language = createValidName(_shadergen.getLanguage());
}

} // namespace MaterialX
