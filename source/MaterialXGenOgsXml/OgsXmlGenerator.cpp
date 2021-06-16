//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/PugiXML/pugixml.hpp>

#include <MaterialXCore/Value.h>

using namespace pugi;

namespace MaterialX
{

namespace
{
	typedef std::unordered_map<string, const pugi::char_t*> OGS_TYPE_MAP_T;

	const OGS_TYPE_MAP_T& getOgsTypeMap() {
		// Data types used by OGS
		// Delayed initialization to survive C++ init order fiasco.
		// Keyed by string to survive multiple redinitions of global symbols.
		static const OGS_TYPE_MAP_T OGS_TYPE_MAP =
		{
			{ Type::BOOLEAN->getName(), "bool" },
			{ Type::FLOAT->getName(), "float" },
			{ Type::INTEGER->getName(), "int" },
			{ Type::STRING->getName(), "int" },
			{ Type::COLOR3->getName(), "float3" },
			{ Type::COLOR4->getName(), "float4" },
			{ Type::VECTOR2->getName(), "float2" },
			{ Type::VECTOR3->getName(), "float3" },
			{ Type::VECTOR4->getName(), "float4" },
			{ Type::MATRIX33->getName(), "float4x4" },
			{ Type::MATRIX44->getName(), "float4x4" }
		};
		return OGS_TYPE_MAP;
	}

    // Semantics used by OGS
    static const std::unordered_map<string, const pugi::char_t*> OGS_SEMANTICS_MAP =
    {
        // Note: The semantics of Pw, Nw and Tw have to match the name of the property.
        { "Pw", "Pw"},
        { "Pm", "Pm"},
        { "Nw", "Nw" },
        { "Nm", "Nm" },
        { "Tw", "Tw" },
        { "Tm", "Tm" },
        { "Bw", "Bw" },
        { "Bm", "Bm" },

        { "u_worldMatrix", "World" },
        { "u_worldInverseMatrix", "WorldInverse" },
        { "u_worldTransposeMatrix", "WorldTranspose" },
        { "u_worldInverseTransposeMatrix", "WorldInverseTranspose" },

        { "u_viewMatrix", "View" },
        { "u_viewInverseMatrix", "ViewInverse" },
        { "u_viewTransposeMatrix", "ViewTranspose" },
        { "u_viewInverseTransposeMatrix", "ViewInverseTranspose" },

        { "u_projectionMatrix", "Projection" },
        { "u_projectionInverseMatrix", "ProjectionInverse" },
        { "u_projectionTransposeMatrix", "ProjectionTranspose" },
        { "u_projectionInverseTransposeMatrix", "ProjectionInverseTranspose" },

        { "u_worldViewMatrix", "WorldView" },
        { "u_viewProjectionMatrix", "ViewProjection" },
        { "u_worldViewProjectionMatrix", "WorldViewProjection" },

        { "u_viewDirection", "ViewDirection" },
        { "u_viewPosition", "WorldCameraPosition" },

        { "u_frame", "Frame" },
        { "u_time", "Time" }
    };

    static const vector<std::pair<string, const pugi::char_t*> > OGS_SEMANTICS_PREFIX_MAP =
    {
        { "texcoord_", "mayaUvCoordSemantic" },
        { "color_", "colorset" },
        { "i_geomprop_", "mayaUvCoordSemantic" }
    };

    namespace OgsParameterFlags
    {
        static const std::string
            VARYING_INPUT_PARAM = "varyingInputParam",
            IS_REQUIREMENT_ONLY = "isRequirementOnly";
    }

    // String constants (alphabetical, please)
    const pugi::char_t* CLASS("class");
    const pugi::char_t* CONNECT("connect");
    const pugi::char_t* CONNECTIONS("connections");
    const pugi::char_t* DESCRIPTION("description");
    const pugi::char_t* DIFFUSEI("diffuseI");
    const pugi::char_t* FEATURE_LEVEL("feature_level");
    const pugi::char_t* FLAGS("flags");
    const pugi::char_t* FRAGMENT("fragment");
    const pugi::char_t* FRAGMENTGRAPH("FragmentGraph");
    const pugi::char_t* FRAGMENTS("fragments");
    const pugi::char_t* FRAGMENT_GRAPH("fragment_graph");
    const pugi::char_t* FRAGMENT_REF("fragment_ref");
    const pugi::char_t* FROM("from");
    const pugi::char_t* FUNCTION_NAME("function_name");
    const pugi::char_t* FUNCTION_VAL("val");
    const pugi::char_t* IMPLEMENTATION("implementation");
    const pugi::char_t* INDENTATION("  ");
    const pugi::char_t* IRRADIANCEENV("IrradianceEnv");
    const pugi::char_t* LANGUAGE("language");
    const pugi::char_t* LANGUAGE_VERSION("lang_version");
    const pugi::char_t* LIGHTLOOPRESULT("lightLoopResult");
    const pugi::char_t* LIGHTTYPE("lightType");
    const pugi::char_t* LIGHT_ACCUM("maya16LightAccum");
    const pugi::char_t* LIGHT_BUILDER("materialXLightDataBuilder");
    const pugi::char_t* LIGHT_SELECTOR("mayaLightSelector16");
    const pugi::char_t* NAME("name");
    const pugi::char_t* OGS_RENDER("OGSRenderer");
    const pugi::char_t* OUTPUTS("outputs");
    const pugi::char_t* PLUMBING("plumbing");
    const pugi::char_t* PROPERTIES("properties");
    const pugi::char_t* REF("ref");
    const pugi::char_t* RENDER("render");
    const pugi::char_t* SAMPLER("sampler");
    const pugi::char_t* SCALEDDIFFUSE("scaledDiffuse");
    const pugi::char_t* SELECTOR("selector");
    const pugi::char_t* SEMANTIC("semantic");
    const pugi::char_t* SHADER_FRAGMENT("ShadeFragment");
    const pugi::char_t* SOURCE("source");
    const pugi::char_t* SPECULARENV("SpecularEnv");
    const pugi::char_t* SPECULARI("specularI");
    const pugi::char_t* TEXTURE2("texture2");
    const pugi::char_t* TO("to");
    const pugi::char_t* TYPE("type");
    const pugi::char_t* UI_NAME("uiName");
    const pugi::char_t* VALUE("value");
    const pugi::char_t* VALUES("values");
    const pugi::char_t* VERSION("version");

    std::string DOT_COMBINE(const pugi::char_t* frag, const pugi::char_t* attr) {
        std::string retVal(frag);
        retVal += ".";
        retVal += attr;
        return retVal;
    }

    void xmlAddImplementation(pugi::xml_node& parent, const string& language, const string& languageVersion,
        const string& functionName, const string& functionSource)
    {
        pugi::xml_node impl = parent.append_child(IMPLEMENTATION);
        {
            impl.append_attribute(RENDER) = OGS_RENDER;
            impl.append_attribute(LANGUAGE) = language.c_str();
            impl.append_attribute(LANGUAGE_VERSION) = languageVersion.c_str();
            pugi::xml_node func = impl.append_child(FUNCTION_NAME);
            func.append_attribute(FUNCTION_VAL) = functionName.c_str();
            pugi::xml_node source = impl.append_child(SOURCE);
            source.append_child(pugi::node_cdata).set_value(functionSource.c_str());
        }
    }

    void xmlSetProperty(pugi::xml_node& prop, const string& name, const string& variable, const string& parameterFlags = EMPTY_STRING, const pugi::char_t* refNode = nullptr)
    {
        prop.append_attribute(NAME) = variable.c_str();
        const auto semantic = OGS_SEMANTICS_MAP.find(name);
        if (semantic != OGS_SEMANTICS_MAP.end())
        {
            prop.append_attribute(SEMANTIC) = semantic->second;
        }
        for (auto const& ogsSemantic: OGS_SEMANTICS_PREFIX_MAP) {
            // STL startswith:
            if (name.rfind(ogsSemantic.first, 0) == 0) {
                prop.append_attribute(SEMANTIC) = ogsSemantic.second;
            }
        }
        if (!parameterFlags.empty())
        {
            prop.append_attribute(FLAGS) = parameterFlags.c_str();
        }
        if (refNode)
        {
            prop.append_attribute(REF) = DOT_COMBINE(refNode, variable.c_str()).c_str();
        }
    }

    void xmlAddProperties(pugi::xml_node& parent, const VariableBlock& block, const string& parameterFlags = EMPTY_STRING, const pugi::char_t* refNode = nullptr)
    {
        for (size_t i = 0; i < block.size(); ++i)
        {
            const ShaderPort* const shaderPort = block[i];

            if (refNode && (shaderPort->getVariable() == DIFFUSEI || shaderPort->getVariable() == SPECULARI))
            {
                // Skip diffuseI and specularI when generating light rig graph.
                continue;
            }

            if (shaderPort->getType() == Type::FILENAME)
            {
                const string& samplerName = shaderPort->getVariable();
                const string textureName = OgsXmlGenerator::samplerToTextureName(samplerName);
                if (!textureName.empty())
                {
                    pugi::xml_node texture = parent.append_child(TEXTURE2);
                    xmlSetProperty(texture, shaderPort->getName(), textureName, parameterFlags, refNode);
                    pugi::xml_node sampler = parent.append_child(SAMPLER);
                    xmlSetProperty(sampler, shaderPort->getName(), samplerName, parameterFlags, refNode);
                }
            }
            else
            {
                const auto type = getOgsTypeMap().find(shaderPort->getType()->getName());
                if (type != getOgsTypeMap().end())
                {
                    pugi::xml_node prop = parent.append_child(type->second);
                    if (shaderPort->getType() == Type::MATRIX33)
                    {
                        const string var = shaderPort->getVariable() + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX;
                        xmlSetProperty(prop, shaderPort->getName(), var, parameterFlags, refNode);
                    }
                    else
                    {
                        xmlSetProperty(prop, shaderPort->getName(), shaderPort->getVariable(), parameterFlags, refNode);
                    }
                }
            }
        }
    }

    void xmlAddValues(pugi::xml_node& parent, const VariableBlock& block, bool skipLightRig = false)
    {
        for (size_t i = 0; i < block.size(); ++i)
        {
            const ShaderPort* p = block[i];
            if (skipLightRig && (p->getVariable() == DIFFUSEI || p->getVariable() == SPECULARI))
            {
                // Skip diffuseI and specularI when generating light rig graph.
                continue;
            }
            if (p->getValue())
            {
                auto type = getOgsTypeMap().find(p->getType()->getName());
                if (type != getOgsTypeMap().end())
                {
                    pugi::xml_node val = parent.append_child(type->second);
                    if (p->getType() == Type::MATRIX33)
                    {
                        // Change the variable name + promote from a matrix33 to a matrix44
                        string var = p->getVariable() + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX;
                        val.append_attribute(NAME) = var.c_str();

                        Matrix33 matrix33 = fromValueString<Matrix33>(p->getValue()->getValueString());
                        const Matrix44 matrix44(
                            matrix33[0][0], matrix33[0][1], matrix33[0][2], 0,
                            matrix33[1][0], matrix33[1][1], matrix33[1][2], 0,
                            matrix33[2][0], matrix33[2][1], matrix33[2][2], 0,
                            0.0f, 0.0f, 0.0f, 1.0f);
                        ValuePtr matrix44Value = Value::createValue<Matrix44>(matrix44);
                        val.append_attribute(VALUE) = matrix44Value->getValueString().c_str();
                    }
                    else
                    {
                        val.append_attribute(NAME) = p->getVariable().c_str();
                        val.append_attribute(VALUE) = p->getValue()->getValueString().c_str();
                    }
                }
            }
        }
    }
}

const string OgsXmlGenerator::OUTPUT_NAME = "outColor";
const string OgsXmlGenerator::VP_TRANSPARENCY_NAME = "vp2Transparency";
const string OgsXmlGenerator::SAMPLER_SUFFIX = "_sampler";

bool OgsXmlGenerator::isSamplerName(const string& name)
{
    // We follow the SPIRV-Cross naming conventions for HLSL samplers
    // auto-generated from combined GLSL samplers. This happens to be compatible
    // with the OGS convention for pairing samplers and textures which only
    // requires that the texture name be a substring of the sampler name.
    
    static const size_t SUFFIX_LENGTH = SAMPLER_SUFFIX.size();
    return name.size() > SUFFIX_LENGTH + 1
        && name[0] == '_'
        && 0 == name.compare(name.size() - SUFFIX_LENGTH, SUFFIX_LENGTH, SAMPLER_SUFFIX);
}

string OgsXmlGenerator::textureToSamplerName(const string& textureName)
{
    string result = "_";
    result += textureName;
    result += SAMPLER_SUFFIX;
    return result;
}

string OgsXmlGenerator::samplerToTextureName(const string& samplerName)
{
    static const size_t PREFIX_SUFFIX_LENGTH = SAMPLER_SUFFIX.size() + 1;
    return isSamplerName(samplerName)
        ? samplerName.substr(1, samplerName.size() - PREFIX_SUFFIX_LENGTH) : "";
}

string OgsXmlGenerator::generate(
    const std::string& shaderName,
    const Shader& glslShader,
    const std::string& hlslSource
)
{
    // Create the interface using one of the shaders (interface should be the same)
    const ShaderStage& glslPixelStage = glslShader.getStage(Stage::PIXEL);

    xml_document xmlDocument;

    pugi::xml_node xmlRoot = xmlDocument.append_child(FRAGMENT);
    xmlRoot.append_attribute(UI_NAME) = shaderName.c_str();
    xmlRoot.append_attribute(NAME) = shaderName.c_str();
    xmlRoot.append_attribute(TYPE) = PLUMBING;
    xmlRoot.append_attribute(CLASS) = SHADER_FRAGMENT;
    xmlRoot.append_attribute(VERSION) = "1";

    pugi::xml_node xmlDescription = xmlRoot.append_child(DESCRIPTION);
    xmlDescription.append_child(pugi::node_cdata).set_value("Code generated from MaterialX description");

    // Add properties
    pugi::xml_node xmlProperties = xmlRoot.append_child(PROPERTIES);
    xmlAddProperties(xmlProperties, glslPixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS), OgsParameterFlags::IS_REQUIREMENT_ONLY);
    xmlAddProperties(xmlProperties, glslPixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS));
    xmlAddProperties(xmlProperties, glslPixelStage.getInputBlock(HW::VERTEX_DATA), OgsParameterFlags::VARYING_INPUT_PARAM);

    const bool hwTransparency = glslShader.hasAttribute(HW::ATTR_TRANSPARENT);
    if (hwTransparency)
    {
        // A dummy argument not used in the generated shader code but necessary to
        // map onto an OGS fragment parameter and a shading node DG attribute with
        // the same name that can be set to a non-0 value to let Maya know that the
        // surface is transparent.
        pugi::xml_node p = xmlProperties.append_child("float");
        xmlSetProperty(p, "", VP_TRANSPARENCY_NAME.c_str());
    }

    // Add values
    pugi::xml_node xmlValues = xmlRoot.append_child(VALUES);
    xmlAddValues(xmlValues, glslPixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS));
    xmlAddValues(xmlValues, glslPixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS));

    // Add a color3 output
    const VariableBlock& outputs = glslPixelStage.getOutputBlock(HW::PIXEL_OUTPUTS);
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("Shader stage has no output");
    }
    pugi::xml_node xmlOutputs = xmlRoot.append_child(OUTPUTS);
    pugi::xml_node xmlOut = xmlOutputs.append_child(getOgsTypeMap().at(hwTransparency ? Type::COLOR4->getName() : Type::COLOR3->getName()));
    xmlOut.append_attribute(NAME) = OUTPUT_NAME.c_str();

    // Add implementations
    pugi::xml_node xmlImpementations = xmlRoot.append_child(IMPLEMENTATION);
    xmlAddImplementation(xmlImpementations, "GLSL", "3.0",  glslPixelStage.getFunctionName(), glslShader.getSourceCode(Stage::PIXEL));
    xmlAddImplementation(xmlImpementations, "HLSL", "11.0", glslPixelStage.getFunctionName(), hlslSource);
    xmlAddImplementation(xmlImpementations, "Cg", "2.1", glslPixelStage.getFunctionName(), "// Cg");

    std::ostringstream stream;
    xmlDocument.save(stream, INDENTATION);
    return stream.str();
}

string OgsXmlGenerator::generateLightRig(
    const std::string& shaderName,
    const std::string& baseShaderName,
    const Shader& glslShader)
{
    // Create the interface using one of the shaders (interface should be the same)
    const ShaderStage& glslPixelStage = glslShader.getStage(Stage::PIXEL);

    VariableBlock vertexInputs = glslPixelStage.getInputBlock(HW::VERTEX_DATA);

    xml_document xmlDocument;

    pugi::xml_node xmlRoot = xmlDocument.append_child(FRAGMENT_GRAPH);
    xmlRoot.append_attribute(NAME) = shaderName.c_str();
    xmlRoot.append_attribute(REF) = shaderName.c_str();
    xmlRoot.append_attribute(CLASS) = FRAGMENTGRAPH;
    xmlRoot.append_attribute(VERSION) = "1.0";
    xmlRoot.append_attribute(FEATURE_LEVEL) = "0";

    // Add fragments:
    pugi::xml_node xmlFragments = xmlRoot.append_child(FRAGMENTS);
    pugi::xml_node xmlFragmentRef = xmlFragments.append_child(FRAGMENT_REF);
    xmlFragmentRef.append_attribute(NAME) = LIGHT_BUILDER;
    xmlFragmentRef.append_attribute(REF) = LIGHT_BUILDER;
    xmlFragmentRef = xmlFragments.append_child(FRAGMENT_REF);
    xmlFragmentRef.append_attribute(NAME) = LIGHT_ACCUM;
    xmlFragmentRef.append_attribute(REF) = LIGHT_ACCUM;
    xmlFragmentRef = xmlFragments.append_child(FRAGMENT_REF);
    xmlFragmentRef.append_attribute(NAME) = baseShaderName.c_str();
    xmlFragmentRef.append_attribute(REF) = baseShaderName.c_str();

    // Connect fragments:
    pugi::xml_node xmlConnections = xmlRoot.append_child(CONNECTIONS);
    pugi::xml_node xmlConnection = xmlConnections.append_child(CONNECT);
    xmlConnection.append_attribute(FROM) = DOT_COMBINE(LIGHT_BUILDER, OgsXmlGenerator::OUTPUT_NAME.c_str()).c_str();
    xmlConnection.append_attribute(TO) = DOT_COMBINE(LIGHT_ACCUM, SCALEDDIFFUSE).c_str();
    xmlConnection.append_attribute(NAME) = SCALEDDIFFUSE;
    xmlConnection = xmlConnections.append_child(CONNECT);
    xmlConnection.append_attribute(FROM) = DOT_COMBINE(LIGHT_ACCUM, SCALEDDIFFUSE).c_str();
    xmlConnection.append_attribute(TO) = DOT_COMBINE(baseShaderName.c_str(), LIGHTLOOPRESULT).c_str();
    xmlConnection.append_attribute(NAME) = LIGHTLOOPRESULT;

    // Add properties
    pugi::xml_node xmlProperties = xmlRoot.append_child(PROPERTIES);
    xmlAddProperties(xmlProperties, glslPixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS), OgsParameterFlags::IS_REQUIREMENT_ONLY, baseShaderName.c_str());
    xmlAddProperties(xmlProperties, glslPixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS), EMPTY_STRING, baseShaderName.c_str());
    xmlAddProperties(xmlProperties, vertexInputs, OgsParameterFlags::VARYING_INPUT_PARAM, baseShaderName.c_str());

    // Add Light Rig properties:
    auto vec3OGSType = getOgsTypeMap().find(Type::VECTOR3->getName())->second;
    auto intOGSType = getOgsTypeMap().find(Type::INTEGER->getName())->second;
    pugi::xml_node xmlLightProp = xmlProperties.append_child(vec3OGSType);
    xmlLightProp.append_attribute(NAME) = IRRADIANCEENV;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(baseShaderName.c_str(), DIFFUSEI).c_str();
    xmlLightProp = xmlProperties.append_child(vec3OGSType);
    xmlLightProp.append_attribute(NAME) = SPECULARENV;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(baseShaderName.c_str(), SPECULARI).c_str();
    xmlLightProp = xmlProperties.append_child(Type::STRING->getName().c_str());
    xmlLightProp.append_attribute(NAME) = SELECTOR;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(LIGHT_ACCUM, SELECTOR).c_str();
    xmlLightProp = xmlProperties.append_child(intOGSType);
    xmlLightProp.append_attribute(NAME) = LIGHTTYPE;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(LIGHT_BUILDER, LIGHTTYPE).c_str();
    xmlLightProp = xmlProperties.append_child(vec3OGSType);
    xmlLightProp.append_attribute(NAME) = DIFFUSEI;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(LIGHT_BUILDER, DIFFUSEI).c_str();
    xmlLightProp = xmlProperties.append_child(vec3OGSType);
    xmlLightProp.append_attribute(NAME) = SPECULARI;
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(LIGHT_BUILDER, SPECULARI).c_str();
    xmlLightProp = xmlProperties.append_child(vec3OGSType);
    xmlLightProp.append_attribute(NAME) = "Lw";
    xmlLightProp.append_attribute(REF) = DOT_COMBINE(LIGHT_BUILDER, "L").c_str();
    xmlLightProp.append_attribute(FLAGS) = OgsParameterFlags::VARYING_INPUT_PARAM.c_str();

    // Add values
    pugi::xml_node xmlValues = xmlRoot.append_child(VALUES);
    xmlAddValues(xmlValues, glslPixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS));
    xmlAddValues(xmlValues, glslPixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS), true);

    // Add light rig values:
    pugi::xml_node xmlLightValue = xmlValues.append_child(vec3OGSType);
    xmlLightValue.append_attribute(NAME) = DIFFUSEI;
    xmlLightValue.append_attribute(VALUE) = "0, 0, 0";
    xmlLightValue = xmlValues.append_child(vec3OGSType);
    xmlLightValue.append_attribute(NAME) = SPECULARI;
    xmlLightValue.append_attribute(VALUE) = "0, 0, 0";
    xmlLightValue = xmlValues.append_child(Type::STRING->getName().c_str());
    xmlLightValue.append_attribute(NAME) = SELECTOR;
    xmlLightValue.append_attribute(VALUE) = LIGHT_SELECTOR;

    // Add a color3 output
    const VariableBlock& outputs = glslPixelStage.getOutputBlock(HW::PIXEL_OUTPUTS);
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("Shader stage has no output");
    }
    pugi::xml_node xmlOutputs = xmlRoot.append_child(OUTPUTS);
    const bool hwTransparency = glslShader.hasAttribute(HW::ATTR_TRANSPARENT);
    pugi::xml_node xmlOut = xmlOutputs.append_child(getOgsTypeMap().at(hwTransparency ? Type::COLOR4->getName() : Type::COLOR3->getName()));
    xmlOut.append_attribute(NAME) = OUTPUT_NAME.c_str();
    xmlOut.append_attribute(REF) = DOT_COMBINE(baseShaderName.c_str(), OgsXmlGenerator::OUTPUT_NAME.c_str()).c_str();

    std::ostringstream stream;
    xmlDocument.save(stream, INDENTATION);
    return stream.str();
}

bool OgsXmlGenerator::sUseLightAPIV2 = false;

bool OgsXmlGenerator::useLightAPIV2() {
    return sUseLightAPIV2;
}
void OgsXmlGenerator::setUseLightAPIV2(bool val) {
    sUseLightAPIV2 = val;
}


} // namespace MaterialX
