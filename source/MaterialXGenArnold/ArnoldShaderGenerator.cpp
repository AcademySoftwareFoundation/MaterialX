//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>
#include <MaterialXGenShader/Nodes/BsdfNodes.h>

namespace MaterialX
{

const string ArnoldShaderGenerator::TARGET = "arnold";


ArnoldShaderGenerator::ArnoldShaderGenerator()
    : OslShaderGenerator()
{
    const StringSet reservedWords = { "metal", "sheen", "bssrdf", "empirical_bssrdf", "randomwalk_bssrdf", 
                                      "volume_absorption", "volume_emission", "volume_henyey_greenstein", 
                                      "volume_matte" };

    _syntax->registerReservedWords(reservedWords);

    // <!-- <dielectric_bsdf> -->
    registerImplementation("IM_dielectric_bsdf_" + ArnoldShaderGenerator::TARGET, DielectricBsdfNode::create);

    // <!-- <generalized_schlick_bsdf> -->
    registerImplementation("IM_generalized_schlick_bsdf_" + ArnoldShaderGenerator::TARGET, DielectricBsdfNode::create);

    // <!-- <conductor_bsdf> -->
    registerImplementation("IM_conductor_bsdf_" + ArnoldShaderGenerator::TARGET, ConductorBsdfNode::create);

    // <!-- <sheen_bsdf> -->
    registerImplementation("IM_sheen_bsdf_" + ArnoldShaderGenerator::TARGET, SheenBsdfNode::create);
}

}
