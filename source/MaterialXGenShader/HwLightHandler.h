#ifndef MATERIALX_HWLIGHTHANDLER_H
#define MATERIALX_HWLIGHTHANDLER_H

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Node.h>

#include <string>
#include <memory>

namespace MaterialX
{

class HwShaderGenerator;
class GenOptions;

/// Shared pointer to a LightHandler
using HwLightHandlerPtr = std::shared_ptr<class HwLightHandler>;

/// @class @HwLightHandler
/// Utility light handler for creating and providing
/// light data for shader binding.
///
class HwLightHandler
{
public:
    /// Static instance create function
    static HwLightHandlerPtr create() { return std::make_shared<HwLightHandler>(); }

    static unsigned int getLightType(NodePtr node);

    /// Default constructor
    HwLightHandler();

    /// Default destructor
    virtual ~HwLightHandler();

    // Adds a light source node
    void addLightSource(NodePtr node);

    /// Return a vector of all created light sources.
    const vector<NodePtr>& getLightSources() const
    {
        return _lightSources;
    }

    /// Bind all added light shaders to the given shader generator.
    /// Only the light shaders bound to the generator will have their
    /// code emitted during shader generation.
    void bindLightShaders(HwShaderGenerator& shadergen, const GenOptions& options) const;

    /// Set path to irradiance IBL image
    void setLightEnvIrradiancePath(const string& path)
    {
        _lightEnvIrradiancePath = path;
    }

    /// Get path to irradiance IBL image
    const string& getLightEnvIrradiancePath() const
    {
        return _lightEnvIrradiancePath;
    }

    /// Set path to radiance IBL image
    void setLightEnvRadiancePath(const string& path)
    {
        _lightEnvRadiancePath = path;
    }

    /// Get path to radiance IBL image
    const string& getLightEnvRadiancePath() const
    {
        return _lightEnvRadiancePath;
    }

private:
    vector<NodePtr> _lightSources;
    string _lightEnvIrradiancePath;
    string _lightEnvRadiancePath;
};

} // namespace MaterialX

#endif
