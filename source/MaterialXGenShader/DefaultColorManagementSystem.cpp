//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXGenShader/Exception.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const string CMS_NAME = "default_cms";

// Remap from legacy color space names to their color interop ID equivalents.
// See https://github.com/AcademySoftwareFoundation/ColorInterop/tree/main/
//             Recommendations/01_TextureAssetColorSpaces
const StringMap COLOR_SPACE_REMAP =
{
    // Map 1.39.5 names to their ASWF Color Interop Forum equivalents.
    { "g18_rec709",     "g18_rec709_scene" },
    { "g22_rec709",     "g22_rec709_scene" },
    { "rec709_display", "g24_rec709_scene" },
    { "acescg",         "lin_ap1_scene" },
    { "lin_ap1",        "lin_ap1_scene" },
    { "g22_ap1",        "g22_ap1_scene" },
    { "srgb_texture",   "srgb_rec709_scene" },
    { "lin_adobergb",   "lin_adobergb_scene" },
    { "adobergb",       "g22_adobergb_scene" },
    { "srgb_displayp3", "srgb_p3d65_scene" },
    { "lin_displayp3",  "lin_p3d65_scene" },
    { "lin_rec709",     "lin_rec709_scene" },

    // These names are from a much earlier version.
    { "gamma18", "g18_rec709_scene" },
    { "gamma22", "g22_rec709_scene" },
    { "gamma24", "g24_rec709_scene" },
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

bool DefaultColorManagementSystem::isNoOpColorSpace(const string& colorSpace) const
{
    return colorSpace == "none" || colorSpace == "data";
}

NodeDefPtr DefaultColorManagementSystem::getNodeDef(const ColorSpaceTransform& transform) const
{
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }

    string sourceSpace = COLOR_SPACE_REMAP.count(transform.sourceSpace) ? COLOR_SPACE_REMAP.at(transform.sourceSpace) : transform.sourceSpace;
    string targetSpace = COLOR_SPACE_REMAP.count(transform.targetSpace) ? COLOR_SPACE_REMAP.at(transform.targetSpace) : transform.targetSpace;

    // After remapping, the transform may be a no-op (e.g. lin_rec709_scene -> lin_rec709).
    if (sourceSpace == targetSpace)
    {
        return _document->getNodeDef("ND_dot_" + transform.type.getName());
    }

    string nodeName = sourceSpace + "_to_" + targetSpace;

    for (NodeDefPtr nodeDef : _document->getMatchingNodeDefs(nodeName))
    {
        for (OutputPtr output : nodeDef->getOutputs())
        {
            if (output->getType() == transform.type.getName())
            {
                return nodeDef;
            }
        }
    }
    return nullptr;
}

MATERIALX_NAMESPACE_END
