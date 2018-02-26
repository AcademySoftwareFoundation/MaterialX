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
    /// Identifier for additional vertex shader stage
    static const size_t VERTEX_STAGE = Shader::NUM_STAGES;
    static const size_t NUM_STAGES = Shader::NUM_STAGES + 1;

    /// Identifier for light data uniform block.
    static const string LIGHT_DATA_BLOCK;

public:
    HwShader(const string& name);

    /// Initialize the shader before shader generation.
    /// @param element The root element to generate the shader from. 
    /// @param shadergen The shader generator instance.
    void initialize(ElementPtr element, ShaderGenerator& shadergen) override;

    /// Return the number of shader stages for this shader.
    size_t numStages() const override { return NUM_STAGES; }

    /// Create a new variable for vertex data. This creates an 
    /// output from the vertex stage and and input to the pixel stage.
    virtual void createVertexData(const string& type, const string& name, const string& semantic = EMPTY_STRING);

    /// Return the block of a vertex data variables.
    const VariableBlock& getVertexDataBlock() const { return _vertexData; }
    
    /// Query if an output has been calculated in the vertex stage.
    bool isCalculated(const string& outputName) const 
    {
        return _calculatedVertexData.count(outputName) > 0;
    }

    /// Set an output as calculated in the vertex stage.
    void setCalculated(const string& outputName)
    {
        _calculatedVertexData.insert(outputName);
    }

private:
    VariableBlock _vertexData;
    std::set<string> _calculatedVertexData;
};

} // namespace MaterialX

#endif
