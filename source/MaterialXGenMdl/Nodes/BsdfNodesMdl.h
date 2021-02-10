//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_BSDFNODES_MDL_H
#define MATERIALX_BSDFNODES_MDL_H

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

namespace MaterialX
{

/// Dielectric BSDF node.
class DielectricBsdfNodeMdl : public SourceCodeNodeMdl
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

/// Sheen BSDF node.
class SheenBsdfNodeMdl : public SourceCodeNodeMdl
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

} // namespace MaterialX

#endif
