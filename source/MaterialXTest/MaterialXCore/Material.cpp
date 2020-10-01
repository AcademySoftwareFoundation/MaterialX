//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

namespace mx = MaterialX;

TEST_CASE("Material", "[material]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Create a base shader nodedef.
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("ND_simpleSrf", "surfaceshader", "simpleSrf");
    simpleSrf->setInputValue("diffColor", mx::Color3(1.0f));
    mx::InputPtr specColor = simpleSrf->setInputValue("specColor", mx::Color3(0.0f));
    mx::InputPtr roughness = simpleSrf->setInputValue("roughness", 0.25f);
    mx::TokenPtr texId = simpleSrf->setTokenValue("texId", "01");
    REQUIRE(simpleSrf->getInputValue("diffColor")->asA<mx::Color3>() == mx::Color3(1.0f));
    REQUIRE(simpleSrf->getInputValue("specColor")->asA<mx::Color3>() == mx::Color3(0.0f));
    REQUIRE(simpleSrf->getInputValue("roughness")->asA<float>() == 0.25f);
    REQUIRE(simpleSrf->getTokenValue("texId") == "01");

    // Create an inherited shader nodedef.
    mx::NodeDefPtr anisoSrf = doc->addNodeDef("ND_anisoSrf", "surfaceshader", "anisoSrf");
    anisoSrf->setInheritsFrom(simpleSrf);
    anisoSrf->setInputValue("anisotropy", 0.0f);
    REQUIRE(anisoSrf->getInheritsFrom() == simpleSrf);

    // Create a material.
    mx::MaterialPtr material = doc->addMaterial();
    REQUIRE(material->getPrimaryShaderName().empty());

    // Add a shader reference.
    mx::ShaderRefPtr refAnisoSrf = material->addShaderRef("SR_anisoSrf", "anisoSrf");
    REQUIRE(anisoSrf->getInstantiatingShaderRefs()[0] == refAnisoSrf);
    REQUIRE(refAnisoSrf->getNodeDef() == anisoSrf);
    REQUIRE(material->getPrimaryShaderName() == refAnisoSrf->getNodeString());
    REQUIRE(material->getPrimaryShaderInputs().size() == 4);
    REQUIRE(material->getPrimaryShaderTokens().size() == 1);

    // Set nodedef and shader reference qualifiers.
    refAnisoSrf->setVersionString("2.0");
    REQUIRE(refAnisoSrf->getNodeDef() == nullptr);
    anisoSrf->setVersionString("2");
    REQUIRE(refAnisoSrf->getNodeDef() == anisoSrf);
    refAnisoSrf->setType("volumeshader");
    REQUIRE(refAnisoSrf->getNodeDef() == nullptr);
    refAnisoSrf->setType("surfaceshader");
    REQUIRE(refAnisoSrf->getNodeDef() == anisoSrf);

    // Bind a shader parameter to a value.
    mx::BindInputPtr bindInput = refAnisoSrf->addBindInput("roughness");
    bindInput->setValue(0.5f);
    REQUIRE(roughness->getBoundValue(material)->asA<float>() == 0.5f);
    REQUIRE(roughness->getDefaultValue()->asA<float>() == 0.25f);

    // Bind a shader input to a value.
    bindInput = refAnisoSrf->addBindInput("specColor");
    bindInput->setValue(mx::Color3(0.5f));
    REQUIRE(specColor->getBoundValue(material)->asA<mx::Color3>() == mx::Color3(0.5f));
    REQUIRE(specColor->getDefaultValue()->asA<mx::Color3>() == mx::Color3(0.0f));

    // Bind a shader token to a value.
    mx::BindTokenPtr bindToken = refAnisoSrf->addBindToken("texId");
    bindToken->setValue("02");
    REQUIRE(texId->getBoundValue(material)->asA<std::string>() == "02");
    REQUIRE(texId->getDefaultValue()->asA<std::string>() == "01");
    mx::StringResolverPtr resolver = doc->createStringResolver(mx::UNIVERSAL_GEOM_NAME, material);
    REQUIRE(resolver->resolve("diffColor_[texId].tif", mx::FILENAME_TYPE_STRING) == "diffColor_02.tif");

    // Add an invalid shader reference.
    material->addShaderRef("SR_invalidSrf", "invalidSrf");
    REQUIRE(!doc->validate());
    material->removeShaderRef("SR_invalidSrf");
    REQUIRE(doc->validate());

    // Create an inherited material.
    mx::MaterialPtr material2 = doc->addMaterial();
    material2->setInheritsFrom(material);
    REQUIRE(material2->getPrimaryShaderName() == refAnisoSrf->getNodeString());
    REQUIRE(material2->getPrimaryShaderInputs().size() == 4);
    REQUIRE(material2->getPrimaryShaderTokens().size() == 1);
    REQUIRE(roughness->getBoundValue(material2)->asA<float>() == 0.5f);

    // Create and detect an inheritance cycle.
    material->setInheritsFrom(material2);
    REQUIRE(!doc->validate());
    material->setInheritsFrom(nullptr);
    REQUIRE(doc->validate());

    // Disconnect the inherited material.
    material2->setInheritsFrom(nullptr);
    REQUIRE(material2->getPrimaryShaderName().empty());
    REQUIRE(material2->getPrimaryShaderInputs().empty());
    REQUIRE(material2->getPrimaryShaderTokens().empty());
    REQUIRE(roughness->getBoundValue(material2)->asA<float>() == 0.25f);

    // Remove shader reference.
    material->removeShaderRef(refAnisoSrf->getName());
    REQUIRE(anisoSrf->getInstantiatingShaderRefs().empty());
    REQUIRE(material->getPrimaryShaderName().empty());
    REQUIRE(material->getPrimaryShaderInputs().empty());
    REQUIRE(material->getPrimaryShaderTokens().empty());
}
