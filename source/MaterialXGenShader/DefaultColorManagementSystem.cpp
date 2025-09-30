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

// Remap from legacy color space names to their ACES 1.2 equivalents.
const StringMap COLOR_SPACE_REMAP =
{
    { "gamma18", "g18_rec709" },
    { "gamma22", "g22_rec709" },
    { "gamma24", "rec709_display" },
    { "lin_ap1", "acescg" },

    // In 1.39 we remap namespaces from ASWF recommended color interop spaces
    //  See https://github.com/AcademySoftwareFoundation/ColorInterop
    // This should improve interop with the nanocolor spaces found in USD.
    {"lin_ap1_scene",       "acescg"},
    {"lin_rec709_scene",    "lin_rec709"},
    {"lin_p3d65_scene",     "lin_displayp3"},
    {"lin_adobergb_scene",  "lin_adobergb"},
    {"srgb_rec709_scene",   "srgb_texture"},
    {"g22_rec709_scene",    "g22_rec709"},
    {"g18_rec709_scene",    "g18_rec709"},
    {"g22_ap1_scene",       "g22_ap1"},
    {"srgb_p3d65_scene",    "srgb_displayp3"},
    {"g22_adobergb_scene",  "adobergb"}
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

NodeDefPtr DefaultColorManagementSystem::getNodeDef(const ColorSpaceTransform& transform) const
{
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }

    string sourceSpace = COLOR_SPACE_REMAP.count(transform.sourceSpace) ? COLOR_SPACE_REMAP.at(transform.sourceSpace) : transform.sourceSpace;
    string targetSpace = COLOR_SPACE_REMAP.count(transform.targetSpace) ? COLOR_SPACE_REMAP.at(transform.targetSpace) : transform.targetSpace;
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
