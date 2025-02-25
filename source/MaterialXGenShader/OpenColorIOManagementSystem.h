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
#include <memory>

MATERIALX_NAMESPACE_BEGIN

// Opaque class to insulate from the full OCIO API
class OpenColorIOManagementSystemImpl;

/// A shared pointer to a OpenColorIOManagementSystem
using OpenColorIOManagementSystemPtr = std::shared_ptr<class OpenColorIOManagementSystem>;

/// @class OpenColorIOManagementSystem
/// Class for a default color management system.
class MX_GENSHADER_API OpenColorIOManagementSystem : public DefaultColorManagementSystem
{
  public:
    virtual ~OpenColorIOManagementSystem();

    /// Create a new OpenColorIOManagementSystem using using the OCIO environment variable.
    static OpenColorIOManagementSystemPtr createFromEnv(string target);

    /// Create a new OpenColorIOManagementSystem using a specific OCIO config file.
    static OpenColorIOManagementSystemPtr createFromFile(const string& filename, string target);

    /// Create a new OpenColorIOManagementSystem using an OCIO built-in config.
    static OpenColorIOManagementSystemPtr createFromBuiltinConfig(const string& configName, string target);

    /// Return the OpenColorIOManagementSystem name
    const string& getName() const override;

    /// Can the CMS create a shader node implementation for one of its registered CMS transforms
    bool hasImplementation(const string& implName) const override;

    /// Create an OCIO node
    ShaderNodeImplPtr createImplementation(const string& implName) const override;

    /// Returns shader text for an implementation
    string getGpuProcessorCode(const string& implName, const string& functionName) const;

    /// Prefix common to all implementation names
    static const string IMPL_PREFIX;

  protected:
    /// Returns a nodedef for a given transform
    NodeDefPtr getNodeDef(const ColorSpaceTransform& transform) const override;

    /// Looks for a valid color space name, either in the current config, or using built-in
    /// color spaces to find an equivalent one.
    const char* getSupportedColorSpaceName(const char* colorSpace) const;

    /// Protected constructor
    OpenColorIOManagementSystem(OpenColorIOManagementSystemImpl*);

  private:
    std::unique_ptr<OpenColorIOManagementSystemImpl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
#endif
