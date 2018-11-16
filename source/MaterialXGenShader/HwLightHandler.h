#ifndef MATERIALX_HWLIGHTHANDLER_H
#define MATERIALX_HWLIGHTHANDLER_H

#include <MaterialXCore/Definition.h>

#include <string>
#include <memory>

namespace MaterialX
{

class HwShaderGenerator;
class GenOptions;

/// Shared pointer to a LightSource
using LightSourcePtr = std::shared_ptr<class LightSource>;

/// @class @LightSource
/// Class holding data for a light source.
///
class LightSource
{
public:
    using ParameterMap = std::unordered_map<string, ValuePtr>;

    /// Return the light type
    size_t getType() const
    {
        return _typeId;
    }

    /// Return the light parameters
    const ParameterMap& getParameters() const
    {
        return _parameters;
    }

    /// Set a light parameter value
    template<typename T>
    void setParameter(const string& name, const T& value)
    {
        ParameterMap::iterator it = _parameters.find(name);
        if (it != _parameters.end())
        {
            if (!it->second)
            {
                it->second = Value::createValue<T>(value);
            }
            else
            {
                TypedValue<T>* typedVal = dynamic_cast<TypedValue<T>*>(it->second.get());
                if (!typedVal)
                {
                    throw Exception("Incorrect type when setting light paramater");
                }
                typedVal->setData(value);
            }
        }
    }

protected:
    LightSource(size_t typeId, const NodeDef& nodeDef);

    const size_t _typeId;
    ParameterMap _parameters;

    friend class HwLightHandler;
};


/// Shared pointer to a LightHandler
using HwLightHandlerPtr = std::shared_ptr<class HwLightHandler>;

/// @class @HwLightHandler
/// Utility light handler for creating and providing 
/// light data for shader binding.
///
class HwLightHandler
{
public:
    using LightShaderMap = std::unordered_map<size_t, ConstNodeDefPtr>;

public:
    /// Static instance create function
    static HwLightHandlerPtr create() { return std::make_shared<HwLightHandler>(); }

    /// Default constructor
    HwLightHandler();
    
    /// Default destructor
    virtual ~HwLightHandler();

    /// Add a light shader to be used for creting light sources
    void addLightShader(size_t typeId, ConstNodeDefPtr nodeDef);

    /// Create a light source of given type. The typeId must match
    /// a light shader that has been previously added to the handler.
    LightSourcePtr createLightSource(size_t typeId);

    /// Return a vector of all created light sources.
    const vector<LightSourcePtr>& getLightSources() const
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
    LightShaderMap _lightShaders;
    vector<LightSourcePtr> _lightSources;
    string _lightEnvIrradiancePath;
    string _lightEnvRadiancePath;
};

} // namespace MaterialX

#endif
