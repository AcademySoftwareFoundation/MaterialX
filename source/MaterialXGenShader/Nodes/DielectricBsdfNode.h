//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_DIELECTRICBSDFNODE_H
#define MATERIALX_DIELECTRICBSDFNODE_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

namespace MaterialX
{

/// Dielectric BSDF node.
class DielectricBsdfNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Dielectric BSDF node specifically for HW.
class HwDielectricBsdfNode : public HwSourceCodeNode
{
  public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

} // namespace MaterialX

#endif
