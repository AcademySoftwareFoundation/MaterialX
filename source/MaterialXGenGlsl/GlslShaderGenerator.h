//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

/// @file
/// GLSL shader generator

#include <MaterialXGenShader/HwShaderGenerator.h>

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

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Emit function definitions for all nodes
    void emitFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// Emit all functon calls constructing the shader body
    void emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// Emit a shader variable.
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

  public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

  protected:
    virtual void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;
    virtual void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Emit specular environment lookup code
    void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Override the compound implementation creator in order to handle light compounds.
    ShaderNodeImplPtr createCompoundImplementation(const NodeGraph& impl) const override;

    static void toVec4(const TypeDesc* type, string& variable);

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;
};


/// Base class for common GLSL node implementations
class GlslImplementation : public ShaderNodeImpl
{
  public:
    const string& getTarget() const override;

    bool isEditable(const ShaderInput& input) const override;

  protected:
    GlslImplementation() {}

    // Integer identifiers for corrdinate spaces
    // The order must match the order given for
    // the space enum string in stdlib.
    enum Space
    {
        MODEL_SPACE  = 0,
        OBJECT_SPACE = 1,
        WORLD_SPACE  = 2
    };

    /// Internal string constants
    static const string SPACE;
    static const string TO_SPACE;
    static const string FROM_SPACE;
    static const string WORLD;
    static const string OBJECT;
    static const string MODEL;
    static const string INDEX;
    static const string GEOMPROP;
};

} // namespace MaterialX

#endif
