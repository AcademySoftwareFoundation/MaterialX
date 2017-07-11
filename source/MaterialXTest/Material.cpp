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

    // Create shader nodedefs.
    mx::NodeDefPtr shaderDef = doc->addNodeDef("shader1", "surfaceshader", "simpleSrf");
    mx::InputPtr diffColor = shaderDef->addInput("diffColor");
    diffColor->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::InputPtr specColor = shaderDef->addInput("specColor");
    specColor->setValue(mx::Color3(0.0f, 0.0f, 0.0f));
    mx::ParameterPtr roughness = shaderDef->addParameter("roughness");
    roughness->setValue(0.2f);
    roughness->setPublicName("editRoughness");
    mx::InputPtr input1 = shaderDef->addInput("input1", mx::getTypeString<mx::Color3>());
    mx::NodeDefPtr shaderDef2 = doc->addNodeDef("shader2", "surfaceshader", "otherSrf");
    REQUIRE(*shaderDef != *shaderDef2);

    // Create material.
    mx::MaterialPtr material = doc->addMaterial();
    REQUIRE_THROWS_AS(doc->addMaterial(material->getName()), mx::Exception);

    SECTION("Shader references")
    {
        // Add shader reference.
        mx::ShaderRefPtr shaderRef = material->addShaderRef("", "simpleSrf");
        REQUIRE(shaderDef->getInstantiatingShaderRefs()[0] == shaderRef);
        REQUIRE(shaderRef->getReferencedShaderDef() == shaderDef);

        // Bind a shader input to a value.
        mx::BindInputPtr bindInput = shaderRef->addBindInput("specColor");
        bindInput->setValue(mx::Color3(0.5f, 0.5f, 0.5f));
        REQUIRE(specColor->getUpstreamElement(material) == bindInput);

        // Bind a shader parameter to a value.
        mx::BindParamPtr bindParam = shaderRef->addBindParam("roughness");
        bindParam->setValue(0.5f);
        REQUIRE(roughness->getUpstreamElement(material) == bindParam);

        // Add invalid shader reference.
        mx::ShaderRefPtr shaderRef2 = material->addShaderRef("", "invalidSrf");
        REQUIRE(!shaderRef2->getReferencedShaderDef());
        material->removeShaderRef(shaderRef2->getName());

        // Remove shader references.
        material->removeShaderRef(shaderRef->getName());
        material->removeShaderRef(shaderRef2->getName());
        REQUIRE(shaderDef->getInstantiatingShaderRefs().empty());
        REQUIRE(material->getChildren().empty());
    }

    SECTION("Overrides")
    {
        // Add valid override.
        mx::OverridePtr roughOverride = material->setOverrideValue("editRoughness", 0.1f);
        REQUIRE(roughOverride->getValue()->isA<float>());
        REQUIRE(roughOverride->getReceiver() == roughness);
        REQUIRE(roughness->getUpstreamElement(material) == roughOverride);

        // Add invalid override.
        mx::OverridePtr anisoOverride = material->setOverrideValue("anisotropic", 0.5f);
        REQUIRE(!anisoOverride->getReceiver());

        // Remove overrides.
        material->removeOverride(roughOverride->getName());
        material->removeOverride(anisoOverride->getName());
        REQUIRE(material->getOverrides().empty());
    }

    SECTION("MaterialInherits")
    {
        // Create a derived material.
        mx::MaterialPtr material2 = doc->addMaterial();
        material2->setInheritsFrom(material);
        REQUIRE(material2->getInheritsFrom() == material);
        material2->setInheritsFrom(mx::MaterialPtr());
        REQUIRE(material2->getInheritsFrom() == mx::MaterialPtr());

        // Remove inheriting material.
        doc->removeMaterial(material2->getName());
    }
}
