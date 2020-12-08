//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const string DefaultColorManagementSystem::CMS_NAME = "default_cms";

//
// DefaultColorManagementSystem methods
//

string DefaultColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform) const
{
    return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_" + _target;
}

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(const string& target)
{
    DefaultColorManagementSystemPtr result(new DefaultColorManagementSystem(target));
    return result;
}

DefaultColorManagementSystem::DefaultColorManagementSystem(const string& target)
{
    _target = createValidName(target);
}

} // namespace MaterialX
