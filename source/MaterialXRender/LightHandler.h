//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LIGHTHANDLER_H
#define MATERIALX_LIGHTHANDLER_H

/// @file
/// Handler for hardware lights

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Node.h>

#include <string>
#include <memory>

namespace MaterialX
{
class GenContext;

/// Shared pointer to a LightHandler
using LightHandlerPtr = std::shared_ptr<class LightHandler>;

/// @class LightHandler
/// Utility light handler for creating and providing
/// light data for shader binding.
///
class LightHandler
{
  public:
    /// Static instance create function
    static LightHandlerPtr create() { return std::make_shared<LightHandler>(); }

    /// Default constructor
    LightHandler();

    /// Default destructor
    virtual ~LightHandler();

    /// Adds a light source node
    void addLightSource(NodePtr node);

    /// Get the list of light sources.
    const vector<NodePtr>& getLightSources() const
    {
        return _lightSources;
    }

    /// Get a list of identifiers associated with a given light nodedef
    const std::unordered_map<string, unsigned int>& getLightIdentifierMap() const
    {
        return _lightIdentifierMap;
    }

    /// Set the list of light sources.
    void setLightSources(const vector<NodePtr>& lights)
    {
        _lightSources = lights;
    }

    /// Set path to irradiance IBL image
    void setLightEnvIrradiancePath(const FilePath& filePath)
    {
        _lightEnvIrradiancePath = filePath;
    }

    /// Get path to irradiance IBL image
    const FilePath& getLightEnvIrradiancePath() const
    {
        return _lightEnvIrradiancePath;
    }

    /// Set path to radiance IBL image
    void setLightEnvRadiancePath(const FilePath& filePath)
    {
        _lightEnvRadiancePath = filePath;
    }

    /// Get path to radiance IBL image
    const FilePath& getLightEnvRadiancePath() const
    {
        return _lightEnvRadiancePath;
    }

    /// From a set of nodes, create a mapping of corresponding
    /// nodedef identifiers to numbers
    void mapNodeDefToIdentiers(const std::vector<NodePtr>& nodes,
                               std::unordered_map<string, unsigned int>& ids);

    /// Find lights to use based on an input document
    /// @param doc Document to scan for lights
    /// @param lights List of lights found in document
    void findLights(DocumentPtr doc, std::vector<NodePtr>& lights);

    /// Register light node definitions and light count with a given generation context
    /// @param doc Document containing light nodes and definitions
    /// @param lights Lights to register
    /// @param context Context to update
    void registerLights(DocumentPtr doc, const std::vector<NodePtr>& lights, GenContext& context);

  private:
    vector<NodePtr> _lightSources;
    std::unordered_map<string, unsigned int> _lightIdentifierMap;
    FilePath _lightEnvIrradiancePath;
    FilePath _lightEnvRadiancePath;
};

} // namespace MaterialX

#endif
