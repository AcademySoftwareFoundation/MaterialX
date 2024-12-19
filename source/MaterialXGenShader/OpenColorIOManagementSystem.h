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
#include <OpenColorIO/OpenColorABI.h>
#include <OpenColorIO/OpenColorIO.h>

#include <OpenColorIO/OpenColorTypes.h>
#include <map>
#include <memory>

namespace OCIO = OCIO_NAMESPACE;

MATERIALX_NAMESPACE_BEGIN

/// A shared pointer to a OpenColorIOManagementSystem
using OpenColorIOManagementSystemPtr = std::shared_ptr<class OpenColorIOManagementSystem>;

/// @class OpenColorIOManagementSystem
/// Class for a default color management system.
class MX_GENSHADER_API OpenColorIOManagementSystem : public DefaultColorManagementSystem
{
  public:
    virtual ~OpenColorIOManagementSystem() { }

    /// Create a new OpenColorIOManagementSystem
    static OpenColorIOManagementSystemPtr create(const OCIO::ConstConfigRcPtr& config, const string& target);

    /// Return the OpenColorIOManagementSystem name
    const string& getName() const override;

    /// Can the CMS create a shader node implementation for one of its registered CMS transforms
    bool hasImplementation(const string& implName) const override;

    /// Create an OCIO node
    ShaderNodeImplPtr createImplementation(const string& implName) const override;

    /// Returns a cached GPU processor registered for an implementation
    OCIO::ConstGPUProcessorRcPtr getGpuProcessor(const string& implName);

    /// Prefix common to all implementation names
    static const string IMPL_PREFIX;

    /// Prefix common to all node definition names
    static const string ND_PREFIX;

    /// Prefix common to all node graph names
    static const string NG_PREFIX;

  protected:
    /// Returns a nodedef for a given transform
    NodeDefPtr getNodeDef(const ColorSpaceTransform& transform) const override;

    /// Looks for a valid color space name, either in the current config, or using built-in
    /// color spaces to find an equivalent one.
    const char* getSupportedColorSpaceName(const char* colorSpace) const;

    /// Protected constructor
    OpenColorIOManagementSystem(const OCIO::ConstConfigRcPtr& config, const string& target);

  private:
    string _target;
    OCIO::ConstConfigRcPtr _config;
    mutable std::map<string, OCIO::ConstGPUProcessorRcPtr> _implementations;
};

MATERIALX_NAMESPACE_END

#endif
#endif
