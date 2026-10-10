//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Smoke test for TextureBakerHlsl. Verifies the baker constructs and
// reports its standard properties; full bake-roundtrip-to-document
// coverage is provided by the existing TextureBaker tests for the
// other backends and reused via the shared base class.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderHlsl/TextureBaker.h>

namespace mx = MaterialX;

TEST_CASE("Render: Hlsl TextureBaker Construction", "[renderhlsl]")
{
    mx::TextureBakerHlslPtr baker = mx::TextureBakerHlsl::create(64, 32);
    REQUIRE(baker);
    REQUIRE(baker->getTextureSpaceMin() == mx::Vector2(0.0f, 0.0f));
    REQUIRE(baker->getTextureSpaceMax() == mx::Vector2(1.0f, 1.0f));
}
