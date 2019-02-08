#ifndef MATERIALX_HWSHADERGENERATOR_H
#define MATERIALX_HWSHADERGENERATOR_H

#include <MaterialXGenShader/ShaderGenerator.h>
#include <algorithm>

namespace MaterialX
{

using HwShaderGeneratorPtr = shared_ptr<class HwShaderGenerator>;

/// Base class for shader generators targeting HW rendering.
class HwShaderGenerator : public ShaderGenerator
{
public:
    using LightShaderMap = std::unordered_map<size_t, ShaderNodeImplPtr>;

public:
    /// Set the maximum number of light sources that can be active at once.
    void setMaxActiveLightSources(unsigned int count) 
    { 
        _maxActiveLightSources = std::max((unsigned int)1, count);
    }

    /// Get the maximum number of light sources that can be active at once.
    unsigned int getMaxActiveLightSources() const { return _maxActiveLightSources; }

    /// Bind a light shader to a light type id, for usage in surface shaders created 
    /// by the generator. The lightTypeId should be a unique identifier for the light 
    /// type (node definition) and the same id should be used when setting light parameters on a 
    /// generated surface shader.
    void bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, const GenOptions& options);

    /// Return a map of all light shaders that has been bound. The map contains the 
    /// light shader implementations with their bound light type id's.
    const LightShaderMap& getBoundLightShaders() const { return _boundLightShaders; }

    /// Return the light shader implementation for the given light type id.
    /// If no light shader with that light type has been bound a nullptr is 
    /// returned instead.
    ShaderNodeImpl* getBoundLightShader(unsigned int lightTypeId);

protected:
    HwShaderGenerator(SyntaxPtr syntax);

    unsigned int _maxActiveLightSources;
    LightShaderMap _boundLightShaders;
};

} // namespace MaterialX

#endif
