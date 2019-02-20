#ifndef MATERIALX_HWSHADERGENERATOR_H
#define MATERIALX_HWSHADERGENERATOR_H

#include <MaterialXGenShader/ShaderGenerator.h>
#include <algorithm>

namespace MaterialX
{

namespace HW
{
    /// Identifiers for shader stages.
    extern const string VERTEX_STAGE;
    extern const string PIXEL_STAGE;

    /// Identifiers for variable blocks.
    extern const string VERTEX_INPUTS;    // Geometric inputs for vertex stage.
    extern const string VERTEX_DATA;      // Connector block for data transfer from vertex stage to pixel stage.
    extern const string PRIVATE_UNIFORMS; // Uniform inputs not visible to user but set privately by application.
    extern const string PUBLIC_UNIFORMS;  // Uniform inputs visible in UI and set by users.
    extern const string LIGHT_DATA;       // Uniform inputs for light sources.
    extern const string PIXEL_OUTPUTS;    // Outputs from the main/pixel stage.

    /// Attribute names.
    extern const string TRANSPARENT;

    /// User data names.
    extern const string CLOSURE_CONTEXT;
}

class HwClosureContext
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
    using Argument = std::pair<string, string>;
    using Arguments = vector<Argument>;

    /// Constructor
    HwClosureContext(int type) : _type(type) {}

    /// Return the identifier for this context.
    int getType() const { return _type; }

    /// Add an extra argument to be used for functions in this context.
    void addArgument(const string& type, const string& name)
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

using HwShaderGeneratorPtr = shared_ptr<class HwShaderGenerator>;

/// Base class for shader generators targeting HW rendering.
class HwShaderGenerator : public ShaderGenerator
{
public:
    using LightShaderMap = std::unordered_map<size_t, ShaderNodePtr>;

public:
    /// Add the function call for a single node.
    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, bool checkScope) override;

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
    void bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context);

    /// Return a map of all light shaders that has been bound. The map contains the 
    /// light shader implementations with their bound light type id's.
    const LightShaderMap& getBoundLightShaders() const { return _boundLightShaders; }

    /// Return the light shader implementation for the given light type id.
    /// If no light shader with that light type has been bound a nullptr is 
    /// returned instead.
    const ShaderNode* getBoundLightShader(unsigned int lightTypeId) const;

    /// Return the closure contexts defined for the given node.
    void getClosureContexts(const ShaderNode& node, vector<const HwClosureContext*>& ccx) const;

    /// String constants for direction vectors
    static const string LIGHT_DIR;
    static const string VIEW_DIR;

    /// Enum to identify common BSDF direction vectors
    enum class BsdfDir
    {
        NORMAL_DIR,
        LIGHT_DIR,
        VIEW_DIR,
        REFL_DIR
    };

protected:
    HwShaderGenerator(SyntaxPtr syntax);

    /// Create and initialize a new HW shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context);

    /// Override the source code implementation creator.
    ShaderNodeImplPtr createSourceCodeImplementation(ImplementationPtr impl) override;

    /// Override the compound implementation creator.
    ShaderNodeImplPtr createCompoundImplementation(NodeGraphPtr impl) override;

    unsigned int _maxActiveLightSources;
    LightShaderMap _boundLightShaders;

    /// Closure contexts supported by this generator.
    HwClosureContext _reflection;
    HwClosureContext _transmission;
    HwClosureContext _indirect;
    HwClosureContext _emission;

    /// Internal string constants
    static const string INCIDENT;
    static const string OUTGOING;
    static const string NORMAL;
    static const string EVAL;
};

} // namespace MaterialX

#endif
