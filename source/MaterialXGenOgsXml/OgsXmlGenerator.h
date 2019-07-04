//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OGSXMLGENERATOR_H
#define MATERIALX_OGSXMLGENERATOR_H

/// @file
/// OGS XML fragments generator

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

class OgsXmlGenerator
{
public:
    OgsXmlGenerator();

    /// Generate OSG XML for the given shader fragments, output to the given stream.
    static void generate(const string& shaderName, const Shader* glsl, const Shader* hlsl, std::ostream& stream);

    /// String constants
    static const string OUTPUT_NAME;
    static const string SAMPLER_SUFFIX;
};

}

#endif
