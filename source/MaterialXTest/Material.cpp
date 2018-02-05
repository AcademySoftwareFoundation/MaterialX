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

    // Create a shader nodedef.
    mx::NodeDefPtr shaderDef = doc->addNodeDef("shader1", "surfaceshader", "simpleSrf");
    mx::InputPtr diffColor = shaderDef->setInputValue("diffColor", mx::Color3(1.0f));
    mx::InputPtr specColor = shaderDef->setInputValue("specColor", mx::Color3(0.0f));
    mx::ParameterPtr roughness = shaderDef->setParameterValue("roughness", 0.25f);
    roughness->setPublicName("editRoughness");
    REQUIRE(shaderDef->getInputValue("diffColor")->asA<mx::Color3>() == mx::Color3(1.0f));
    REQUIRE(shaderDef->getInputValue("specColor")->asA<mx::Color3>() == mx::Color3(0.0f));
    REQUIRE(shaderDef->getParameterValue("roughness")->asA<float>() == 0.25f);

    // Create a material.
    mx::MaterialPtr material = doc->addMaterial();
    REQUIRE(material->getPrimaryShaderName().empty());

    // Add a shader reference.
    mx::ShaderRefPtr shaderRef = material->addShaderRef("shaderRef1", "simpleSrf");
    REQUIRE(shaderDef->getInstantiatingShaderRefs()[0] == shaderRef);
    REQUIRE(shaderRef->getNodeDef() == shaderDef);
    REQUIRE(material->getPrimaryShaderName() == shaderRef->getNodeString());
    REQUIRE(material->getPrimaryShaderParameters().size() == 1);
    REQUIRE(material->getPrimaryShaderInputs().size() == 2);

    // Bind a shader input to a value.
    mx::BindInputPtr bindInput = shaderRef->addBindInput("specColor");
    bindInput->setValue(mx::Color3(0.5f));
    REQUIRE(specColor->getBoundValue(material)->asA<mx::Color3>() == mx::Color3(0.5f));
    REQUIRE(specColor->getDefaultValue()->asA<mx::Color3>() == mx::Color3(0.0f));

    // Bind a shader parameter to a value.
    mx::BindParamPtr bindParam = shaderRef->addBindParam("roughness");
    bindParam->setValue(0.5f);
    REQUIRE(roughness->getBoundValue(material)->asA<float>() == 0.5f);
    REQUIRE(roughness->getDefaultValue()->asA<float>() == 0.25f);

    // Add an invalid shader reference.
    mx::ShaderRefPtr shaderRef2 = material->addShaderRef("shaderRef2", "invalidSrf");
    REQUIRE(!shaderRef2->getNodeDef());

    // Create an inherited material.
    mx::MaterialPtr material2 = doc->addMaterial();
    material2->setInheritsFrom(material);
    REQUIRE(material2->getPrimaryShaderName() == shaderRef->getNodeString());
    REQUIRE(material2->getPrimaryShaderParameters().size() == 1);
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

    // Remove shader references.
    material->removeShaderRef(shaderRef->getName());
    material->removeShaderRef(shaderRef2->getName());
    REQUIRE(shaderDef->getInstantiatingShaderRefs().empty());
    REQUIRE(material->getPrimaryShaderName().empty());
    REQUIRE(material->getPrimaryShaderParameters().empty());
    REQUIRE(material->getPrimaryShaderInputs().empty());

    // Add a valid override.
    mx::OverridePtr roughOverride = material->setOverrideValue("editRoughness", 0.5f);
    REQUIRE(roughOverride->getReceiver() == roughness);
    REQUIRE(roughness->getBoundValue(material)->asA<float>() == 0.5f);

    // Add an invalid override.
    mx::OverridePtr anisoOverride = material->setOverrideValue("anisotropic", 0.1f);
    REQUIRE(!anisoOverride->getReceiver());

    // Remove overrides.
    material->removeOverride(roughOverride->getName());
    material->removeOverride(anisoOverride->getName());
    REQUIRE(roughness->getBoundValue(material)->asA<float>() == 0.25f);
}
