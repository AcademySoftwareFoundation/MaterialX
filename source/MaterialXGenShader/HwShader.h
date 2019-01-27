#ifndef MATERIALX_HWSHADER_H
#define MATERIALX_HWSHADER_H

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

using HwShaderPtr = shared_ptr<class HwShader>;

/// HwShader class extending the base Shader with a vertex shader stage.
class HwShader : public Shader
{
    using ParentClass = Shader;

public:
    /// Identifier for additional vertex shader stage
    static const string VERTEX_STAGE;

    /// Identifier for light data uniform block.
    static const string LIGHT_DATA_BLOCK;

public:
    HwShader(const string& name);

    /// Initialize the shader before shader generation.
    /// @param element The root element to generate the shader from. 
    /// @param shadergen The shader generator instance.
    /// @param options Generation options
    void initialize(ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options) override;

    /// Returns true if the shader has transparency fragments.
    /// Will return false if the shader is opaque.
    bool hasTransparency() const
    {
        return _transparency;
    }

protected:
    /// Create the stages used by this shader.
    void createStages() override;

    /// Return a container with all top level graphs use by this shader.
    void getTopLevelShaderGraphs(ShaderGenerator& shadergen, std::deque<ShaderGraph*>& graphs) const override;

private:
    bool _transparency;
};

} // namespace MaterialX

#endif
