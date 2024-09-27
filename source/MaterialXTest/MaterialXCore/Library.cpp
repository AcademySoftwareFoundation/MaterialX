//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXCore/Library.h>

namespace mx = MaterialX;

TEST_CASE("Version comparison", "[library]")
{
    // Test for version comparison
    REQUIRE(MATERIALX_VERSION_INDEX > MATERIALX_GENERATE_INDEX(1, 38, 8));
}
