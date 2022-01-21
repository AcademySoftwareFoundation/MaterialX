//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace {

const string CMS_NAME = "default_cms";

// Remap from legacy color space names to their ACES 1.2 equivalents.
const StringMap COLOR_SPACE_REMAP =
{
    { "gamma18", "g18_rec709" },
    { "gamma22", "g22_rec709" },
    { "gamma24", "rec709_display" },
    { "lin_ap1", "acescg" }
};

} // anonymous namespace

//
// DefaultColorManagementSystem methods
//

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(const string& target)
{
    return DefaultColorManagementSystemPtr(new DefaultColorManagementSystem(target));
}

DefaultColorManagementSystem::DefaultColorManagementSystem(const string& target) :
    _target(target)
{
}

const string& DefaultColorManagementSystem::getName() const
{
    return CMS_NAME;
}

ImplementationPtr DefaultColorManagementSystem::getImplementation(const ColorSpaceTransform& transform) const
{
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }

    TargetDefPtr targetDef = _document->getTargetDef(_target);
    string sourceSpace = COLOR_SPACE_REMAP.count(transform.sourceSpace) ? COLOR_SPACE_REMAP.at(transform.sourceSpace) : transform.sourceSpace;
    string targetSpace = COLOR_SPACE_REMAP.count(transform.targetSpace) ? COLOR_SPACE_REMAP.at(transform.targetSpace) : transform.targetSpace;
    for (const string& target : targetDef->getMatchingTargets())
    {
        string implName = "IM_" + sourceSpace + "_to_" + targetSpace + "_" + transform.type->getName() + "_" + target;
        ImplementationPtr impl = _document->getImplementation(implName);
        if (impl)
        {
            return impl;
        }
    }

    return nullptr;
}

MATERIALX_NAMESPACE_END
