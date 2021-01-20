//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_THINFILMNODEMDL_H
#define MATERIALX_THINFILMNODEMDL_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Thin-Film node for MDL.
class ThinFilmNodeMdl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

} // namespace MaterialX

#endif
