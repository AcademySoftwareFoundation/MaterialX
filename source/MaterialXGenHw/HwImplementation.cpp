//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/HwImplementation.h>
#include <MaterialXGenShader/ShaderNode.h>

MATERIALX_NAMESPACE_BEGIN

const string HwImplementation::SPACE = "space";
const string HwImplementation::INDEX = "index";
const string HwImplementation::GEOMPROP = "geomprop";

namespace
{

// When node inputs with these names are modified, we assume the
// associated HW shader must be recompiled.
const StringSet IMMUTABLE_INPUTS = {
    "index",
    "space",
    "attrname"
};

} // anonymous namespace

bool HwImplementation::isEditable(const ShaderInput& input) const
{
    return IMMUTABLE_INPUTS.count(input.getName()) == 0;
}

MATERIALX_NAMESPACE_END
