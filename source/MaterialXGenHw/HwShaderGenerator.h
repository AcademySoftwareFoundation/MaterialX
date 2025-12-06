//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWSHADERGENERATOR_H
#define MATERIALX_HWSHADERGENERATOR_H

/// @file
/// Hardware shader generator base class

#include <MaterialXGenHw/Export.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

// These headers below are not required for inclusion here
// they are included as an affordance to backwards compatibility
// during the refactor transition - The next breaking API point
// we should remove them
#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwImplementation.h>
#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenHw/HwResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

class HwShaderGenerator;
/// Shared pointer to a HwShaderGenerator
using HwShaderGeneratorPtr = shared_ptr<class HwShaderGenerator>;

/// @class HwShaderGenerator
/// Base class for shader generators targeting HW rendering.
class MX_GENHW_API HwShaderGenerator : public ShaderGenerator
{
  public:
    /// Emit code for active light count definitions and uniforms
    virtual void addStageLightingUniforms(GenContext& context, ShaderStage& stage) const;

    /// Return true if the node needs the ClosureData struct added
    bool nodeNeedsClosureData(const ShaderNode& node) const override;

    void emitClosureDataArg(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitClosureDataParameter(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Logic to indicate whether code to support direct lighting should be emitted.
    /// By default if the graph is classified as a shader, or BSDF node then lighting is assumed to be required.
    /// Derived classes can override this logic.
    virtual bool requiresLighting(const ShaderGraph& graph) const;

    /// Bind a light shader to a light type id, for usage in surface shaders created
    /// by the generator. The lightTypeId should be a unique identifier for the light
    /// type (node definition) and the same id should be used when setting light parameters on a
    /// generated surface shader.
    static void bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context);

    /// Unbind a light shader previously bound to the given light type id.
    static void unbindLightShader(unsigned int lightTypeId, GenContext& context);

    /// Unbind all light shaders previously bound.
    static void unbindLightShaders(GenContext& context);

    /// Determine the prefix of vertex data variables.
    virtual string getVertexDataPrefix(const VariableBlock& vertexData) const = 0;

    /// Create the shader node implementation for a NodeGraph implementation.
    ShaderNodeImplPtr createShaderNodeImplForNodeGraph(const NodeGraph& nodegraph) const override;

    // Note : the order must match the order defined in libraries/pbrlib/genglsl/lib/mx_closure_type.glsl
    // TODO : investigate build time mechanism for ensuring these stay in sync.

    /// Types of closure contexts for HW.
    enum ClosureContextType
    {
        DEFAULT,
        REFLECTION,
        TRANSMISSION,
        INDIRECT,
        EMISSION,
        LIGHTING,
        CLOSURE
    };

  protected:
    HwShaderGenerator(TypeSystemPtr typeSystem, SyntaxPtr syntax);

    /// Create and initialize a new HW shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

    void toVec4(TypeDesc type, string& variable) const;
};

MATERIALX_NAMESPACE_END

#endif
