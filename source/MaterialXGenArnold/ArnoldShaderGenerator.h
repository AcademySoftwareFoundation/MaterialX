//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ARNOLDSHADERGENERATOR_H
#define MATERIALX_ARNOLDSHADERGENERATOR_H

/// @file
/// Arnold OSL shader generator

#include <MaterialXGenOsl/OslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN


using ArnoldShaderGeneratorPtr = shared_ptr<class ArnoldShaderGenerator>;

/// @class ArnoldShaderGenerator 
/// An OSL shader generator targeting the Arnold renderer
class ArnoldShaderGenerator : public OslShaderGenerator
{
public:
    ArnoldShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<ArnoldShaderGenerator>(); }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Unique identifyer for this generator target
    static const string TARGET;
};

MATERIALX_NAMESPACE_END

#endif
