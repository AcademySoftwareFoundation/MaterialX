//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWIMPLEMENTATION_H
#define MATERIALX_HWIMPLEMENTATION_H

/// @file
/// Hardware shader node implementation base class

#include <MaterialXGenHw/Export.h>

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// @class HwImplementation
/// Base class for HW node implementations.
class MX_GENHW_API HwImplementation : public ShaderNodeImpl
{
  public:
    bool isEditable(const ShaderInput& input) const override;

  protected:
    HwImplementation() { }

    // Integer identifiers for coordinate spaces.
    // The order must match the order given for the space enum string in stdlib.
    enum Space
    {
        MODEL_SPACE = 0,
        OBJECT_SPACE = 1,
        WORLD_SPACE = 2
    };

    /// Internal string constants
    static const string SPACE;
    static const string INDEX;
    static const string GEOMPROP;
};

MATERIALX_NAMESPACE_END

#endif
