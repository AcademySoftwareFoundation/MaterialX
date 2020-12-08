//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>
#include <MaterialXGenShader/Nodes/ThinFilmNode.h>

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

    // <!-- <dielectric_brdf> -->
    registerImplementation("IM_dielectric_brdf_" + ArnoldShaderGenerator::TARGET, ThinFilmSupport::create);
}

}
