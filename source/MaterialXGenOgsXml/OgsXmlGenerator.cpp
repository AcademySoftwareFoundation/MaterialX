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
    // Data types used by OGS
    static const std::unordered_map<const TypeDesc*, const pugi::char_t*> OGS_TYPE_MAP =
    {
        { Type::BOOLEAN, "bool" },
        { Type::FLOAT, "float" },
        { Type::INTEGER, "int" },
        { Type::STRING, "int" },
        { Type::COLOR2, "float2" },
        { Type::COLOR3, "float3" },
        { Type::COLOR4, "float4" },
        { Type::VECTOR2, "float2" },
        { Type::VECTOR3, "float3" },
        { Type::VECTOR4, "float4" },
        { Type::MATRIX33, "float4x4" },
        { Type::MATRIX44, "float4x4" }
    };

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
        { "texcoord_0", "mayaUvCoordSemantic" },
        { "color_0", "colorset" },

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

    // Custom flags required by OGS XML
    static const StringMap OGS_FLAGS_MAP = 
    {
        // Note: Nw has to be a varyingInputParam instead of a global (isRequirementOnly).
        // This is because the pixel shader main function generated preprocesses Nw before passing it 
        // to the main function of the fragment graph.
        //
        { "Pw", "isRequirementOnly" },
        { "Pm", "isRequirementOnly" },
        { "Nw", "varyingInputParam" },
        { "Nm", "isRequirementOnly" },
        { "Tw", "isRequirementOnly" },
        { "Tm", "isRequirementOnly" },
        { "Bw", "isRequirementOnly" },
        { "Bm", "isRequirementOnly" } 
    };

    // String constants
    const pugi::char_t* INDENTATION("  ");
    const pugi::char_t* NAME("name");
    const pugi::char_t* VALUE("value");
    const pugi::char_t* VALUES("values");
    const pugi::char_t* FRAGMENT("fragment");
    const pugi::char_t* UI_NAME("uiName");
    const pugi::char_t* TYPE("type");
    const pugi::char_t* CLASS("class");
    const pugi::char_t* VERSION("version");
    const pugi::char_t* PLUMBING("plumbing");
    const pugi::char_t* SHADER_FRAGMENT("ShadeFragment");
    const pugi::char_t* DESCRIPTION("description");
    const pugi::char_t* PROPERTIES("properties");
    const pugi::char_t* OUTPUTS("outputs");
    const pugi::char_t* IMPLEMENTATION("implementation");
    const pugi::char_t* RENDER("render");
    const pugi::char_t* OGS_RENDER("OGSRenderer");
    const pugi::char_t* LANGUAGE("language");
    const pugi::char_t* LANGUAGE_VERSION("lang_version");
    const pugi::char_t* FUNCTION_NAME("function_name");
    const pugi::char_t* FUNCTION_VAL("val");
    const pugi::char_t* SOURCE("source");
    const pugi::char_t* SEMANTIC("semantic");
    const pugi::char_t* FLAGS("flags");
    const pugi::char_t* TEXTURE2("texture2");
    const pugi::char_t* SAMPLER("sampler");

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

    void xmlSetProperty(pugi::xml_node& prop, const string& name, const string& variable, const string& flags = EMPTY_STRING)
    {
        prop.append_attribute(NAME) = variable.c_str();
        auto semantic = OGS_SEMANTICS_MAP.find(name);
        if (semantic != OGS_SEMANTICS_MAP.end())
        {
            prop.append_attribute(SEMANTIC) = semantic->second;
        }
        auto flag = OGS_FLAGS_MAP.find(name);
        if (flag != OGS_FLAGS_MAP.end())
        {
            prop.append_attribute(FLAGS) = flag->second.c_str();
        }
        else if (!flags.empty())
        {
            prop.append_attribute(FLAGS) = flags.c_str();
        }
    }

    void xmlAddProperties(pugi::xml_node& parent, const VariableBlock& block, const string& flags = EMPTY_STRING)
    {
        for (size_t i = 0; i < block.size(); ++i)
        {
            const ShaderPort* p = block[i];
            if (p->getName() == "Pw")
            {
                auto type = OGS_TYPE_MAP.find(p->getType());
                if (type != OGS_TYPE_MAP.end())
                {
                    pugi::xml_node prop = parent.append_child(type->second);
                    xmlSetProperty(prop, p->getName(), p->getVariable(), flags);
                }
            }
        }

        for (size_t i = 0; i < block.size(); ++i)
        {
            const ShaderPort* p = block[i];

            if (p->getName() == "Pw")
            {
                continue;
            }
            if (p->getType() == Type::FILENAME)
            {
                const string& samplerName = p->getVariable();
                if (samplerName.size() > 7 && samplerName.substr(samplerName.size() - 7) == OgsXmlGenerator::SAMPLER_SUFFIX)
                {
                    string textureName = samplerName.substr(0, samplerName.size() - 7);
                    pugi::xml_node texture = parent.append_child(TEXTURE2);
                    xmlSetProperty(texture, p->getName(), textureName, flags);
                    pugi::xml_node sampler = parent.append_child(SAMPLER);
                    xmlSetProperty(sampler, p->getName(), samplerName, flags);
                }
            }
            else
            {
                auto type = OGS_TYPE_MAP.find(p->getType());
                if (type != OGS_TYPE_MAP.end())
                {
                    pugi::xml_node prop = parent.append_child(type->second);
                    if (p->getType() == Type::MATRIX33)
                    {
                        string var = p->getVariable() + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX;
                        xmlSetProperty(prop, p->getName(), var, flags);
                    }
                    else
                    {
                        xmlSetProperty(prop, p->getName(), p->getVariable(), flags);
                    }
                }
            }
        }
    }

    void xmlAddValues(pugi::xml_node& parent, const VariableBlock& block)
    {
        for (size_t i = 0; i < block.size(); ++i)
        {
            const ShaderPort* p = block[i];
            if (p->getValue())
            {
                auto type = OGS_TYPE_MAP.find(p->getType());
                if (type != OGS_TYPE_MAP.end())
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
const string OgsXmlGenerator::SAMPLER_SUFFIX = "Sampler";

OgsXmlGenerator::OgsXmlGenerator()
{
}

void OgsXmlGenerator::generate( const std::string& shaderName, const Shader* glsl, const Shader* hlsl,
                                bool hwTransparency, std::ostream& stream)
{
    if (glsl == nullptr && hlsl == nullptr)
    {
        throw ExceptionShaderGenError("Both GLSL and HLSL shaders are null, at least one language must be given to generate XML fragments");
    }

    // Create the interface using one of the shaders (interface should be the same)
    const Shader* shader = glsl != nullptr ? glsl : hlsl;
    const ShaderStage& stage = shader->getStage(Stage::PIXEL);

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
    xmlAddProperties(xmlProperties, stage.getUniformBlock(HW::PRIVATE_UNIFORMS), "isRequirementOnly");
    xmlAddProperties(xmlProperties, stage.getUniformBlock(HW::PUBLIC_UNIFORMS));
    xmlAddProperties(xmlProperties, stage.getInputBlock(HW::VERTEX_DATA), "isRequirementOnly, varyingInputParam");

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
    xmlAddValues(xmlValues, stage.getUniformBlock(HW::PRIVATE_UNIFORMS));
    xmlAddValues(xmlValues, stage.getUniformBlock(HW::PUBLIC_UNIFORMS));

    // Add a color3 output
    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("Shader stage has no output");
    }
    pugi::xml_node xmlOutputs = xmlRoot.append_child(OUTPUTS);
    pugi::xml_node xmlOut = xmlOutputs.append_child(OGS_TYPE_MAP.at(hwTransparency ? Type::COLOR4 : Type::COLOR3));
    xmlOut.append_attribute(NAME) = OUTPUT_NAME.c_str();

    // Add implementations
    pugi::xml_node xmlImpementations = xmlRoot.append_child(IMPLEMENTATION);
    xmlAddImplementation(xmlImpementations, "GLSL", "3.0",  stage.getFunctionName(), glsl ? glsl->getSourceCode(Stage::PIXEL) : "// GLSL");
    xmlAddImplementation(xmlImpementations, "HLSL", "11.0", stage.getFunctionName(), hlsl ? hlsl->getSourceCode(Stage::PIXEL) : "// HLSL");
    xmlAddImplementation(xmlImpementations, "Cg", "2.1", stage.getFunctionName(), "// Cg");

    xmlDocument.save(stream, INDENTATION);
}

} // namespace MaterialX
