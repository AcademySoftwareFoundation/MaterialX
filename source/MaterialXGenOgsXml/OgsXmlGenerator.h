//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OGSXMLGENERATOR_H
#define MATERIALX_OGSXMLGENERATOR_H

/// @file
/// OGS XML fragments generator
#include <MaterialXGenOgsXml/Export.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

class MX_GENOGSXML_API OgsXmlGenerator
{
public:
    /// Generate OSG XML for the given shader fragments, output to the given stream.
    static string generate(
        const string& shaderName,
        const Shader& glslShader,
        const std::string& hlslSource
    );

    /// Generate light rig graph for the given shader fragments, output to the given stream.
    static string generateLightRig(
        const string& shaderName,
        const string& baseShaderName,
        const Shader& glslShader);

    static bool isSamplerName(const string&);
    static string textureToSamplerName(const string&);
    static string samplerToTextureName(const string&);

    /// String constants
    static const string OUTPUT_NAME;
    static const string VP_TRANSPARENCY_NAME;

    /// Use Maya's latest external light functions
    static bool useLightAPIV2();
    static void setUseLightAPIV2(bool);

private:
    static const string SAMPLER_SUFFIX;
    static bool sUseLightAPIV2;
};

}

#endif
