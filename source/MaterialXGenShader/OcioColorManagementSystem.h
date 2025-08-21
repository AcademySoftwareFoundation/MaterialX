//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OCIO_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_OCIO_COLOR_MANAGEMENT_SYSTEM_H

#ifdef MATERIALX_BUILD_OCIO

/// @file
/// OCIO color management system implementation

#include <MaterialXCore/Definition.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <map>

MATERIALX_NAMESPACE_BEGIN

// Opaque class to insulate from the full OCIO API
class OcioColorManagementSystemImpl;

/// A shared pointer to a OcioColorManagementSystem
using OcioColorManagementSystemPtr = std::shared_ptr<class OcioColorManagementSystem>;

/// @class OcioColorManagementSystem
/// Class for a default color management system.
class MX_GENSHADER_API OcioColorManagementSystem : public DefaultColorManagementSystem
{
  public:
    virtual ~OcioColorManagementSystem();

    /// Create a new OcioColorManagementSystem using using the OCIO environment variable.
    static OcioColorManagementSystemPtr createFromEnv(string target);

    /// Create a new OcioColorManagementSystem using a specific OCIO config file.
    static OcioColorManagementSystemPtr createFromFile(const string& filename, string target);

    /// Create a new OcioColorManagementSystem using an OCIO built-in config.
    static OcioColorManagementSystemPtr createFromBuiltinConfig(const string& configName, string target);

    /// Return the OcioColorManagementSystem name
    const string& getName() const override;

    /// Can the CMS create a shader node implementation for one of its registered CMS transforms
    bool hasImplementation(const string& implName) const override;

    /// Create an OCIO node
    ShaderNodeImplPtr createImplementation(const string& implName) const override;

    /// Returns shader text for an implementation
    string getGpuProcessorCode(const string& implName, const string& functionName) const;

    /// Prefix common to all implementation names
    static const string IMPL_PREFIX;

    /// SourceUri common to all OCIO NodeDefs and Implementations:
    static const string OCIO_SOURCE_URI;

  protected:
    /// Returns a nodedef for a given transform
    NodeDefPtr getNodeDef(const ColorSpaceTransform& transform) const override;

    /// Looks for a valid color space name, either in the current config, or using built-in
    /// color spaces to find an equivalent one.
    const char* getSupportedColorSpaceName(const char* colorSpace) const;

    /// Protected constructor
    OcioColorManagementSystem(OcioColorManagementSystemImpl*);

  private:
    std::unique_ptr<OcioColorManagementSystemImpl> _impl;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO

#endif
