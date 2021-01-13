//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_DIELECTRICBSDFNODE_H
#define MATERIALX_DIELECTRICBSDFNODE_H

#include <MaterialXGenShader/Nodes/ThinFilmNode.h>

namespace MaterialX
{

/// Dielectric BSDF node.
/// A base BSDF input is added as an extra input to support dielectric layering.
class DielectricBsdfNode : public ThinFilmSupport
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Dielectric BSDF node specifically for HW.
/// A base BSDF input is added as an extra input to support dielectric layering.
class HwDielectricBsdfNode : public HwThinFilmSupport
{
  public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

} // namespace MaterialX

#endif
