//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADER_H
#define MATERIALX_SHADER_H

/// @file
/// Shader instance class created during shader generation

#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenOptions.h>

namespace MaterialX
{

class ShaderGenerator;
class Shader;
// Shared pointer to a Shader
using ShaderPtr = shared_ptr<Shader>;

/// @class Shader
/// Class containing all data needed during shader generation.
/// After generation is completed it will contain the resulting source code
/// emitted by shader generators.
///
/// The class contains a default implementation using a single shader stage.
/// Derived shaders can override this, as well as overriding all methods 
/// that add code to the shader.
///
class Shader
{
  public:
    /// Constructor
    Shader(const string& name, ShaderGraphPtr graph);

    /// Destructor
    virtual ~Shader() {}

    /// Return the shader name
    const string& getName() const { return _name; }

    /// Return the number of shader stages for this shader.
    size_t numStages() const { return _stages.size(); }

    /// Return a stage by index.
    ShaderStage& getStage(size_t index);

    /// Return a stage by index.
    const ShaderStage& getStage(size_t index) const;

    /// Return a stage by name.
    ShaderStage& getStage(const string& name);

    /// Return a stage by name.
    const ShaderStage& getStage(const string& name) const;

    /// Return true if the shader has a given named attribute.
    bool hasAttribute(const string& attrib) const
    {
        return _attributeMap.count(attrib) != 0;
    }

    /// Return the value for a named attribute,
    /// or nullptr if no such attribute is found.
    ValuePtr getAttribute(const string& attrib) const
    {
        auto it = _attributeMap.find(attrib);
        return it != _attributeMap.end() ? it->second : nullptr;
    }

    /// Set a value attribute on the shader.
    void setAttribute(const string& attrib, ValuePtr value)
    {
        _attributeMap[attrib] = value;
    }

    /// Set a flag attribute on the shader.
    void setAttribute(const string& attrib)
    {
        _attributeMap[attrib] = Value::createValue<bool>(true);
    }

    /// Return the shader graph.
    const ShaderGraph& getGraph() const { return *_graph; }

    /// Return true if this shader matches the given classification.
    bool hasClassification(unsigned int c) const { return _graph->hasClassification(c); }

    /// Return the final shader source code for a given shader stage
    const string& getSourceCode(const string& stage = Stage::PIXEL) const { return getStage(stage).getSourceCode(); }

  protected: 
    /// Create a new stage in the shader.
    ShaderStagePtr createStage(const string& name, ConstSyntaxPtr syntax);

    string _name;
    ShaderGraphPtr _graph;
    std::unordered_map<string, ShaderStagePtr> _stagesMap;
    vector<ShaderStage*> _stages;
    std::unordered_map<string, ValuePtr> _attributeMap;

    friend class ShaderGenerator;
};

} // namespace MaterialX

#endif
