//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN


const string ArnoldShaderGenerator::TARGET = "arnold";


ArnoldShaderGenerator::ArnoldShaderGenerator()
    : OslShaderGenerator()
{
    const StringSet reservedWords = { "metal", "sheen", "bssrdf", "empirical_bssrdf", "randomwalk_bssrdf", 
                                      "volume_absorption", "volume_emission", "volume_henyey_greenstein", 
                                      "volume_matte" };

    _syntax->registerReservedWords(reservedWords);

    // Set colorspace argument for texture lookups
    _tokenSubstitutions[T_FILE_EXTRA_ARGUMENTS] = ", \"colorspace\", file.colorspace";
}

MATERIALX_NAMESPACE_END
