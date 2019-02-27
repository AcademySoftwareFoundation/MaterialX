#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

#include <MaterialXGenShader/HwShaderGenerator.h>

/*
The GLSL shader generator has a number of predefined variables (inputs and uniforms) with set binding rules.
When these are used by a shader the application must bind them to the expected data. The following table is
a listing of the variables with a description of what data they should be bound to.

------------------------------------------------------------------------------------------------------------
    NAME                                TYPE    BINDING
------------------------------------------------------------------------------------------------------------

Vertex input variables :
    i_position                          vec3       Vertex position in object space
    i_normal                            vec3       Vertex normal in object space
    i_tangent                           vec3       Vertex tangent in object space
    i_bitangent                         vec3       Vertex bitangent in object space
    i_texcoord_N                        vec2       Vertex texture coordinate for the N:th uv set
    i_color_N                           vec4       Vertex color for the N:th color set (RGBA)

Uniform variables :
    u_worldMatrix                       mat4       World transformation
    u_worldInverseMatrix                mat4       World transformation, inverted
    u_worldTransposeMatrix              mat4       World transformation, transposed
    u_worldInverseTransposeMatrix       mat4       World transformation, inverted and transposed
    u_viewMatrix                        mat4       View transformation
    u_viewInverseMatrix                 mat4       View transformation, inverted
    u_viewTransposeMatrix               mat4       View transformation, transposed
    u_viewInverseTransposeMatrix        mat4       View transformation, inverted and transposed
    u_projectionMatrix                  mat4       Projection transformation
    u_projectionInverseMatrix           mat4       Projection transformation, inverted
    u_projectionTransposeMatrix         mat4       Projection transformation, transposed
    u_projectionInverseTransposeMatrix  mat4       Projection transformation, inverted and transposed
    u_worldViewMatrix                   mat4       World-view transformation
    u_viewProjectionMatrix              mat4       View-projection transformation
    u_worldViewProjectionMatrix         mat4       World-view-projection transformation
    u_viewPosition                      vec3       World-space position of the view (camera)
    u_viewDirection                     vec3       World-space direction of the view (camera)
    u_frame                             float      The current frame number as defined by the host application
    u_time                              float      The current time in seconds
    u_geomattr_<name>                   <type>     A named attribute of given <type> where <name> is the name of the variable on the geometry
    u_numActiveLightSources             int        The number of currently active light sources. Note that in shader this is clamped against
                                                   the maximum allowed number of lights sources. The maximum number is set by calling 
                                                   HwShaderGenerator::setMaxActiveLightSources().
    u_lightData[]                       struct     Array of struct LightData holding parameters for active light sources.
                                                   The LightData struct is built dynamically depending on requirements for
                                                   bound light shaders.
    u_envIrradiance                     sampler2D  Sampler for the texture used for diffuse environment lighting.
    u_envRadiance                       sampler2D  Sampler for the texture used for specular environment lighting.
    u_envRadianceMips                   int        Number of mipmaps used on the specular environment texture.
    u_envMatrix                         mat4       Rotation matrix for the environment.
    u_envSamples                        int        Samples to use if Filtered Importance Sampling is used for specular environment lighting.

------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
*/

namespace MaterialX
{

using GlslShaderGeneratorPtr = shared_ptr<class GlslShaderGenerator>;

/// Base class for GLSL (OpenGL Shading Language) code generation.
/// A generator for a specific GLSL target should be derived from this class.
class GlslShaderGenerator : public HwShaderGenerator
{
  public:
    GlslShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<GlslShaderGenerator>(); }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Emit function definitions for all nodes
    void emitFunctionDefinitions(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const override;

    /// Emit all functon calls constructing the shader body
    void emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const override;

    /// Emit a shader variable.
    void emitVariableDeclaration(ShaderStage& stage, const ShaderPort* variable, const string& qualifier, bool assingValue) const override;

    /// Given a element attempt to remap a value to an enumeration which is accepted by
    /// the shader generator.
    ValuePtr remapEnumeration(const ValueElementPtr& input, const InterfaceElement& mappingElement, const TypeDesc*& enumerationType) const override;

    /// Given a input specification (name, value, type) attempt to remap a value to an enumeration 
    /// which is accepted by the shader generator.
    ValuePtr remapEnumeration(const string& inputName, const string& inputValue, const string& inputType, 
                              const InterfaceElement& mappingElement, const TypeDesc*& enumerationType) const override;

  public:
    /// Unique identifyer for the glsl language
    static const string LANGUAGE;

    /// Unique identifyer for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

  protected:
    virtual void emitVertexStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const;
    virtual void emitPixelStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const;

    /// Override the compound implementation creator in order to handle light compounds.
    ShaderNodeImplPtr createCompoundImplementation(NodeGraphPtr impl) const override;

    static void toVec4(const TypeDesc* type, string& variable);

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;
};


/// Base class for common GLSL node implementations
class GlslImplementation : public ShaderNodeImpl
{
  public:
    const string& getLanguage() const override;
    const string& getTarget() const override;

  protected:
    GlslImplementation() {}

    // Integer identifiers for corrdinate spaces
    // The order must match the order given for
    // the space enum string in stdlib.
    enum Space
    {
        MODEL_SPACE,
        OBJECT_SPACE,
        WORLD_SPACE
    };

    /// Internal string constants
    static const string SPACE;
    static const string WORLD;
    static const string OBJECT;
    static const string MODEL;
    static const string INDEX;
    static const string ATTRNAME;
};


} // namespace MaterialX

#endif
