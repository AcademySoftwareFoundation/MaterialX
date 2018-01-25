#ifndef MATERIALX_HWSHADER_H
#define MATERIALX_HWSHADER_H

#include <MaterialXShaderGen/Shader.h>

namespace MaterialX
{

/// HwShader class extending the base Shader with a vertex shader stage.
class HwShader : public Shader
{
public:
    // Identifier for additional vertex shader stage
    static const size_t VERTEX_STAGE = Shader::NUM_STAGES;
    static const size_t NUM_STAGES = Shader::NUM_STAGES + 1;

public:
    HwShader(const string& name) : Shader(name)  {}

    /// Return the number of shader stages for this shader.
    virtual size_t numStages() const { return NUM_STAGES; }

    /// Query if a varying has been calculated by the vertex stage
    bool isCalculated(const string& varying) const { return _calculatedVaryings.count(varying) > 0; }

    /// Set a varying as calculated by the vertex stage
    void setCalculated(const string& varying) { _calculatedVaryings.insert(varying); }

private:
    std::set<string> _calculatedVaryings;
};

} // namespace MaterialX

#endif
