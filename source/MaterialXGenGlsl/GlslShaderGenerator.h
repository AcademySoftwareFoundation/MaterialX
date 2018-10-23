#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/HwShader.h>

/*
The GLSL shader generator has a number of predefined variables (inputs and uniforms) with set binding rules.
When these are used by a shader the application must bind them to the expected data. The following table is
a listing of the variables with a description of what data they should be bound to.

------------------------------------------------------------------------------------------------------------
    NAME                                TYPE    BINDING
------------------------------------------------------------------------------------------------------------

Vertex input variables :
    i_position                          vec3    Vertex position in object space
    i_normal                            vec3    Vertex normal in object space
    i_tangent                           vec3    Vertex tangent in object space
    i_bitangent                         vec3    Vertex bitangent in object space
    i_texcoord_N                        vec2    Vertex texture coordinate for the N:th uv set
    i_color_N                           vec4    Vertex color for the N:th color set (RGBA)

Uniform variables :
    u_worldMatrix                       mat4    World transformation
    u_worldInverseMatrix                mat4    World transformation, inverted
    u_worldTransposeMatrix              mat4    World transformation, transposed
    u_worldInverseTransposeMatrix       mat4    World transformation, inverted and transposed
    u_viewMatrix                        mat4    View transformation
    u_viewInverseMatrix                 mat4    View transformation, inverted
    u_viewTransposeMatrix               mat4    View transformation, transposed
    u_viewInverseTransposeMatrix        mat4    View transformation, inverted and transposed
    u_projectionMatrix                  mat4    Projection transformation
    u_projectionInverseMatrix           mat4    Projection transformation, inverted
    u_projectionTransposeMatrix         mat4    Projection transformation, transposed
    u_projectionInverseTransposeMatrix  mat4    Projection transformation, inverted and transposed
    u_worldViewMatrix                   mat4    World-view transformation
    u_viewProjectionMatrix              mat4    View-projection transformation
    u_worldViewProjectionMatrix         mat4    World-view-projection transformation
    u_viewPosition                      vec3    World-space position of the view (camera)
    u_viewDirection                     vec3    World-space direction of the view (camera)
    u_frame                             float   The current frame number as defined by the host application
    u_time                              float   The current time in seconds
    u_geomattr_<name>                   <type>  A named attribute of given <type> where <name> is the name of the variable on the geometry
    u_numActiveLightSources             int     The number of currently active light sources. Note that in shader this is clamped against
                                                the maximum allowed number of lights sources. The maximum number is set by calling 
                                                HwShaderGenerator::setMaxActiveLightSources().
    u_lightData[]                       struct  Array of struct LightData holding parameters for active light sources.
                                                The LightData struct is built dynamically depending on requirements for
                                                bound light shaders.

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
    using ParentClass = HwShaderGenerator;

  public:
    GlslShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<GlslShaderGenerator>(); }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element, const GenOptions& options) override;

    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Emit function definitions for all nodes
    void emitFunctionDefinitions(Shader& shader) override;

    /// Emit all functon calls constructing the shader body
    void emitFunctionCalls(const GenContext& context, Shader &shader) override;

    /// Emit the final output expression
    void emitFinalOutput(Shader& shader) const override;

    /// Add node contexts id's to the given node to control 
    /// in which contexts this node should be used
    void addNodeContextIDs(ShaderNode* node) const override;

    /// Emit code for all texturing nodes.
    virtual void emitTextureNodes(Shader& shader);

    /// Emit code for calculating BSDF response for a shader, 
    /// given the incident and outgoing light directions.
    /// The output 'bsdf' will hold the variable name keeping the result.
    virtual void emitBsdfNodes(const ShaderNode& shaderNode, int bsdfContext, const string& incident, const string& outgoing, Shader& shader, string& bsdf);

    /// Emit code for calculating emission for a surface or light shader,
    /// given the normal direction of the EDF and the evaluation direction.
    /// The output 'edf' will hold the variable keeping the result.
    virtual void emitEdfNodes(const ShaderNode& shaderNode, const string& normalDir, const string& evalDir, Shader& shader, string& edf);

  public:
    /// Unique identifyer for the glsl language
    static const string LANGUAGE;

    /// Unique identifyer for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

    /// String constants for direction vectors
    static const string LIGHT_DIR;
    static const string VIEW_DIR;

    /// Identifiers for contexts
    enum Context
    {
        CONTEXT_BSDF_REFLECTION = CONTEXT_DEFAULT + 1,
        CONTEXT_BSDF_TRANSMISSION,
        CONTEXT_BSDF_INDIRECT,
        CONTEXT_EDF,
    };

    /// Enum to identify common BSDF direction vectors
    enum class BsdfDir
    {
        NORMAL_DIR,
        LIGHT_DIR,
        VIEW_DIR,
        REFL_DIR
    };

  protected:   
    void emitVariable(const Shader::Variable& variable, const string& qualifier, Shader& shader) override;

    /// Override the compound implementation creator in order to handle light compounds.
    ShaderNodeImplPtr createCompoundImplementation(NodeGraphPtr impl) override;

    static void toVec4(const TypeDesc* type, string& variable);

    /// Internal string constants
    static const string INCIDENT;
    static const string OUTGOING;
    static const string NORMAL;
    static const string EVAL;
};


/// Base class for common GLSL node implementations
class GlslImplementation : public ShaderNodeImpl
{
  public:
    const string& getLanguage() const override;
    const string& getTarget() const override;

  protected:
    GlslImplementation() {}

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
