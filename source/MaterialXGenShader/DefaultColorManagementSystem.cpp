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

ImplementationPtr DefaultColorManagementSystem::getImplementation(const ColorSpaceTransform& transform) const
{
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }

    TargetDefPtr targetDef = _document->getTargetDef(_target);
    const StringVec targets = targetDef->getMatchingTargets();
    for (const string& target : targets)
    {
        const string implName = "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_" + target;
        ImplementationPtr impl = _document->getImplementation(implName);
        if (impl)
        {
            return impl;
        }
    }

    return nullptr;
}

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(const string& target)
{
    return DefaultColorManagementSystemPtr(new DefaultColorManagementSystem(target));
}

DefaultColorManagementSystem::DefaultColorManagementSystem(const string& target) :
    _target(target)
{
}

} // namespace MaterialX
