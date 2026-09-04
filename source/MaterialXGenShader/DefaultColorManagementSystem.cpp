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
    {"g24_rec709_scene",    "rec709_display"},
    {"g22_ap1_scene",       "g22_ap1"},
    {"srgb_p3d65_scene",    "srgb_displayp3"},
    {"g22_adobergb_scene",  "adobergb"}
};

// User-facing display names for color space IDs, as tabulated in the color space table
// of MaterialX.Specification.md. Names handled by COLOR_SPACE_REMAP are looked up here
// by their remapped (base) name, so each display name is only listed once.
const StringMap COLOR_SPACE_USER_FACING_NAMES =
{
    // Base names that COLOR_SPACE_REMAP normalizes onto.
    { "g18_rec709",     "Gamma 1.8 Encoded Rec.709" },
    { "g22_rec709",     "Gamma 2.2 Encoded Rec.709" },
    { "rec709_display", "Gamma 2.4 Encoded Rec.709" },
    { "acescg",         "ACEScg" },
    { "lin_rec709",     "Linear Rec.709 (sRGB)" },
    { "lin_displayp3",  "Linear P3-D65" },
    { "lin_adobergb",   "Linear AdobeRGB" },
    { "srgb_texture",   "sRGB Encoded Rec.709 (sRGB)" },
    { "g22_ap1",        "Gamma 2.2 Encoded AP1" },
    { "srgb_displayp3", "sRGB Encoded P3-D65" },
    { "adobergb",       "Gamma 2.2 Encoded AdobeRGB" },

    // Color interop names with no legacy equivalent.
    { "lin_ap0_scene",       "ACES2065-1" },
    { "lin_rec2020_scene",   "Linear Rec.2020" },
    { "lin_ciexyzd65_scene", "CIE XYZ-D65 - Scene-referred" },
    { "srgb_ap1_scene",      "sRGB Encoded AP1" },

    // No-op color spaces.
    { "none", "Data" },  // prefer the new name
    { "data", "Data" }
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

string DefaultColorManagementSystem::getUserFacingName(const string& colorSpace) const
{
    string baseSpace = COLOR_SPACE_REMAP.count(colorSpace) ? COLOR_SPACE_REMAP.at(colorSpace) : colorSpace;
    return COLOR_SPACE_USER_FACING_NAMES.count(baseSpace) ? COLOR_SPACE_USER_FACING_NAMES.at(baseSpace) : colorSpace;
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
