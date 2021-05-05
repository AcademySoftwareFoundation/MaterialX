//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_BSDFNODES_H
#define MATERIALX_BSDFNODES_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

namespace MaterialX
{

/// Dielectric BSDF node.
class MX_GENSHADER_API DielectricBsdfNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Dielectric BSDF node specifically for HW.
class MX_GENSHADER_API HwDielectricBsdfNode : public HwSourceCodeNode
{
  public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Conductor BSDF node.
class MX_GENSHADER_API ConductorBsdfNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Conductor BSDF node specifically for HW.
class MX_GENSHADER_API HwConductorBsdfNode : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Sheen BSDF node.
class MX_GENSHADER_API SheenBsdfNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Sheen BSDF node specifically for HW.
class MX_GENSHADER_API HwSheenBsdfNode : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

} // namespace MaterialX

#endif
