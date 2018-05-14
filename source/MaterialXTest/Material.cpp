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
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("nd_simpleSrf", "surfaceshader", "simpleSrf");
    mx::InputPtr diffColor = simpleSrf->setInputValue("diffColor", mx::Color3(1.0f));
    mx::InputPtr specColor = simpleSrf->setInputValue("specColor", mx::Color3(0.0f));
    mx::ParameterPtr roughness = simpleSrf->setParameterValue("roughness", 0.25f);
    REQUIRE(simpleSrf->getInputValue("diffColor")->asA<mx::Color3>() == mx::Color3(1.0f));
    REQUIRE(simpleSrf->getInputValue("specColor")->asA<mx::Color3>() == mx::Color3(0.0f));
    REQUIRE(simpleSrf->getParameterValue("roughness")->asA<float>() == 0.25f);

    // Create an inherited shader nodedef.
    mx::NodeDefPtr anisoSrf = doc->addNodeDef("nd_anisoSrf", "surfaceshader", "anisoSrf");
    anisoSrf->setInheritsFrom(simpleSrf);
    mx::ParameterPtr anisotropy = anisoSrf->setParameterValue("anisotropy", 0.0f);
    REQUIRE(anisoSrf->getInheritsFrom() == simpleSrf);

    // Create a material.
    mx::MaterialPtr material = doc->addMaterial();
    REQUIRE(material->getPrimaryShaderName().empty());

    // Add a shader reference.
    mx::ShaderRefPtr refAnisoSrf = material->addShaderRef("sr_anisoSrf", "anisoSrf");
    REQUIRE(anisoSrf->getInstantiatingShaderRefs()[0] == refAnisoSrf);
    REQUIRE(refAnisoSrf->getNodeDef() == anisoSrf);
    REQUIRE(material->getPrimaryShaderName() == refAnisoSrf->getNodeString());
    REQUIRE(material->getPrimaryShaderParameters().size() == 2);
    REQUIRE(material->getPrimaryShaderInputs().size() == 2);

    // Bind a shader input to a value.
    mx::BindInputPtr bindInput = refAnisoSrf->addBindInput("specColor");
    bindInput->setValue(mx::Color3(0.5f));
    REQUIRE(specColor->getBoundValue(material)->asA<mx::Color3>() == mx::Color3(0.5f));
    REQUIRE(specColor->getDefaultValue()->asA<mx::Color3>() == mx::Color3(0.0f));

    // Bind a shader parameter to a value.
    mx::BindParamPtr bindParam = refAnisoSrf->addBindParam("roughness");
    bindParam->setValue(0.5f);
    REQUIRE(roughness->getBoundValue(material)->asA<float>() == 0.5f);
    REQUIRE(roughness->getDefaultValue()->asA<float>() == 0.25f);

    // Add an invalid shader reference.
    mx::ShaderRefPtr refInvalid = material->addShaderRef("sr_invalidSrf", "invalidSrf");
    REQUIRE(!doc->validate());
    material->removeShaderRef("sr_invalidSrf");
    REQUIRE(doc->validate());

    // Create an inherited material.
    mx::MaterialPtr material2 = doc->addMaterial();
    material2->setInheritsFrom(material);
    REQUIRE(material2->getPrimaryShaderName() == refAnisoSrf->getNodeString());
    REQUIRE(material2->getPrimaryShaderParameters().size() == 2);
    REQUIRE(material2->getPrimaryShaderInputs().size() == 2);
    REQUIRE(roughness->getBoundValue(material2)->asA<float>() == 0.5f);

    // Create and detect an inheritance cycle.
    material->setInheritsFrom(material2);
    REQUIRE(!doc->validate());
    material->setInheritsFrom(nullptr);
    REQUIRE(doc->validate());

    // Disconnect the inherited material.
    material2->setInheritsFrom(nullptr);
    REQUIRE(material2->getPrimaryShaderName().empty());
    REQUIRE(material2->getPrimaryShaderParameters().empty());
    REQUIRE(material2->getPrimaryShaderInputs().empty());
    REQUIRE(roughness->getBoundValue(material2)->asA<float>() == 0.25f);

    // Remove shader reference.
    material->removeShaderRef(refAnisoSrf->getName());
    REQUIRE(anisoSrf->getInstantiatingShaderRefs().empty());
    REQUIRE(material->getPrimaryShaderName().empty());
    REQUIRE(material->getPrimaryShaderParameters().empty());
    REQUIRE(material->getPrimaryShaderInputs().empty());
}
