//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HWSHADERGENERATOR_H
#define MATERIALX_HWSHADERGENERATOR_H

/// @file
/// Hardware shader generator base class

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <algorithm>

namespace MaterialX
{

/// Shader stage identifiers.
namespace Stage
{
    /// Identifier for vertex stage.
    extern const string VERTEX;
}

/// HW specific identifiers.
namespace HW
{
    /// Identifiers for variable blocks.
    extern const string VERTEX_INPUTS;    // Geometric inputs for vertex stage.
    extern const string VERTEX_DATA;      // Connector block for data transfer from vertex stage to pixel stage.
    extern const string PRIVATE_UNIFORMS; // Uniform inputs set privately by application.
    extern const string PUBLIC_UNIFORMS;  // Uniform inputs visible in UI and set by user.
    extern const string LIGHT_DATA;       // Uniform inputs for light sources.
    extern const string PIXEL_OUTPUTS;    // Outputs from the main/pixel stage.

    /// Default variable names for direction vectors.
    extern const string NORMAL_DIR;
    extern const string LIGHT_DIR;
    extern const string VIEW_DIR;

    /// Attribute names.
    extern const string ATTR_TRANSPARENT;

    /// User data names.
    extern const string USER_DATA_CLOSURE_CONTEXT;
    extern const string USER_DATA_LIGHT_SHADERS;
}

class HwClosureContext;
class HwLightShaders;
class HwShaderGenerator;

/// Shared pointer to a HwClosureContext
using HwClosureContextPtr = shared_ptr<class HwClosureContext>;
/// Shared pointer to a HwLightShaders
using HwLightShadersPtr = shared_ptr<class HwLightShaders>;
/// Shared pointer to a HwShaderGenerator
using HwShaderGeneratorPtr = shared_ptr<class HwShaderGenerator>;

/// @class HwClosureContext
/// Class representing a context for closure evaluation on hardware targets.
/// On hardware BSDF closures are evaluated differently in reflection, transmission
/// or environment/indirect contexts. This class represents with context we are in
/// and if extra arguments and function decorators are needed for that context.
class HwClosureContext : public GenUserData
{
public:
    /// Types of closure contexts.
    enum Type
    {
        REFLECTION,
        TRANSMISSION,
        INDIRECT,
        EMISSION
    };

    /// An extra argument for closure functions.
    /// An argument is a pair of strings holding the
    /// 'type' and 'name' of the argument.
    using Argument = std::pair<const TypeDesc*, string>;
    /// An array of arguments
    using Arguments = vector<Argument>;

    /// Constructor
    HwClosureContext(int type) : _type(type) {}

    /// Create and return a new instance.
    static HwClosureContextPtr create(int type)
    {
        return std::make_shared<HwClosureContext>(type);
    }

    /// Return the identifier for this context.
    int getType() const { return _type; }

    /// Add an extra argument to be used for functions in this context.
    void addArgument(const TypeDesc* type, const string& name)
    {
        _arguments.push_back(Argument(type,name));
    }

    /// Return a list of extra argument to be used for functions in this context.
    const Arguments& getArguments() const { return _arguments; }

    /// Set a function name suffix to be used for the function in this context.
    void setSuffix(const string& suffix) { _suffix = suffix; }

    /// Return the function name suffix to be used for the function in this context.
    const string& getSuffix() const { return _suffix; }

protected:
    const int _type;
    Arguments _arguments;
    string _suffix;
};

/// @class HwLightShaders 
/// Hardware light shader user data
class HwLightShaders : public GenUserData
{
public:
    /// Create and return a new instance.
    static HwLightShadersPtr create()
    {
        return std::make_shared<HwLightShaders>();
    }

    /// Bind a light shader to a light type id.
    void bind(unsigned int type, ShaderNodePtr shader)
    {
        _shaders[type] = shader;
    }

    /// Unbind a light shader previously bound to a light type id.
    void unbind(unsigned int type)
    {
        _shaders.erase(type);
    }

    /// Clear all light shaders previously bound.
    void clear()
    {
        _shaders.clear();
    }

    /// Return the light shader bound to the given light type,
    /// or nullptr if not light shader is bound to this type.
    const ShaderNode* get(unsigned int type) const
    {
        auto it = _shaders.find(type);
        return it != _shaders.end() ? it->second.get() : nullptr;
    }

    /// Return the map of bound light shaders.
    const std::unordered_map<unsigned int, ShaderNodePtr>& get() const
    {
        return _shaders;
    }

protected:
    std::unordered_map<unsigned int, ShaderNodePtr> _shaders;
};


/// @class HwShaderGenerator
/// Base class for shader generators targeting HW rendering.
class HwShaderGenerator : public ShaderGenerator
{
public:
    /// Add the function call for a single node.
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage, 
                          bool checkScope = true) const override;

    /// Emit code for all texturing nodes.
    virtual void emitTextureNodes(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Emit code for calculating BSDF response for a shader, 
    /// given the incident and outgoing light directions.
    /// The output 'bsdf' will hold the variable name keeping the result.
    virtual void emitBsdfNodes(const ShaderGraph& graph, const ShaderNode& shaderNode, HwClosureContextPtr ccx,
                               GenContext& context, ShaderStage& stage, string& bsdf) const;

    /// Emit code for calculating emission for a surface or light shader,
    /// given the normal direction of the EDF and the evaluation direction.
    /// The output 'edf' will hold the variable keeping the result.
    virtual void emitEdfNodes(const ShaderGraph& graph, const ShaderNode& shaderNode, HwClosureContextPtr ccx,
                              GenContext& context, ShaderStage& stage, string& edf) const;

    /// Return the closure contexts defined for the given node.
    void getNodeClosureContexts(const ShaderNode& node, vector<HwClosureContextPtr>& ccx) const;

    /// Bind a light shader to a light type id, for usage in surface shaders created 
    /// by the generator. The lightTypeId should be a unique identifier for the light 
    /// type (node definition) and the same id should be used when setting light parameters on a 
    /// generated surface shader.
    static void bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context);

    /// Unbind a light shader previously bound to the given light type id.
    static void unbindLightShader(unsigned int lightTypeId, GenContext& context);

    /// Unbind all light shaders previously bound.
    static void unbindLightShaders(GenContext& context);

protected:
    HwShaderGenerator(SyntaxPtr syntax);

    /// Create and initialize a new HW shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

    /// Override the source code implementation creator.
    ShaderNodeImplPtr createSourceCodeImplementation(const Implementation& impl) const override;

    /// Override the compound implementation creator.
    ShaderNodeImplPtr createCompoundImplementation(const NodeGraph& impl) const override;

    /// Closure contexts for defining closure functions.
    HwClosureContextPtr _defReflection;
    HwClosureContextPtr _defTransmission;
    HwClosureContextPtr _defIndirect;
    HwClosureContextPtr _defEmission;
};

} // namespace MaterialX

#endif
