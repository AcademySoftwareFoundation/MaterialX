//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LIGHTHANDLER_H
#define MATERIALX_LIGHTHANDLER_H

/// @file
/// Handler for hardware lights

#include <MaterialXRender/Image.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

class GenContext;

/// Shared pointer to a LightHandler
using LightHandlerPtr = std::shared_ptr<class LightHandler>;

/// An unordered map from light names to light indices.
using LightIdMap = std::unordered_map<string, unsigned int>;

/// @class LightHandler
/// Utility light handler for creating and providing
/// light data for shader binding.
class LightHandler
{
  public:
    LightHandler()
    {
    }
    virtual ~LightHandler() { }

    /// Create a new light handler
    static LightHandlerPtr create() { return std::make_shared<LightHandler>(); }

    /// Adds a light source node
    void addLightSource(NodePtr node);

    /// Return the vector of active light sources.
    const vector<NodePtr>& getLightSources() const
    {
        return _lightSources;
    }

    /// Return the first active light source, if any, of the given category.
    NodePtr getFirstLightOfCategory(const string& category)
    {
        for (NodePtr light : _lightSources)
        {
            if (light->getCategory() == category)
            {
                return light;
            }
        }
        return nullptr;
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

    /// Set the environment radiance map
    void setEnvRadianceMap(ImagePtr map)
    {
        _envRadianceMap = map;
    }

    /// Return the environment radiance map
    ImagePtr getEnvRadianceMap() const
    {
        return _envRadianceMap;
    }

    /// Set the environment irradiance map
    void setEnvIrradianceMap(ImagePtr map)
    {
        _envIrradianceMap = map;
    }

    /// Return the environment irradiance map
    ImagePtr getEnvIrradianceMap() const
    {
        return _envIrradianceMap;
    }

    /// Set the directional albedo table
    void setAlbedoTable(ImagePtr table)
    {
        _albedoTable = table;
    }

    /// Return the directional albedo table
    ImagePtr getAlbedoTable() const
    {
        return _albedoTable;
    }

    /// From a set of nodes, create a mapping of corresponding
    /// nodedef identifiers to numbers
    LightIdMap computeLightIdMap(const vector<NodePtr>& nodes);

    /// Find lights to use based on an input document
    /// @param doc Document to scan for lights
    /// @param lights List of lights found in document
    void findLights(DocumentPtr doc, vector<NodePtr>& lights);

    /// Register light node definitions and light count with a given generation context
    /// @param doc Document containing light nodes and definitions
    /// @param lights Lights to register
    /// @param context Context to update
    void registerLights(DocumentPtr doc, const vector<NodePtr>& lights, GenContext& context);

  private:
    vector<NodePtr> _lightSources;
    std::unordered_map<string, unsigned int> _lightIdentifierMap;
    ImagePtr _envRadianceMap;
    ImagePtr _envIrradianceMap;
    ImagePtr _albedoTable;
};

} // namespace MaterialX

#endif
