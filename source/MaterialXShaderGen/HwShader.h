#ifndef MATERIALX_HWSHADER_H
#define MATERIALX_HWSHADER_H

#include <MaterialXShaderGen/Shader.h>

namespace MaterialX
{

using HwShaderPtr = shared_ptr<class HwShader>;

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

    /// Query if an output has been calculated by the vertex stage
    bool isCalculated(const string& outputName) const { return _calculatedOutputs.count(outputName) > 0; }

    /// Set an output as calculated by the vertex stage
    void setCalculated(const string& outputName) { _calculatedOutputs.insert(outputName); }

private:
    std::set<string> _calculatedOutputs;
};

} // namespace MaterialX

#endif
