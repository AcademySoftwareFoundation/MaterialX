//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Types.h>

namespace mx = MaterialX;

TEST_CASE("Vector operators", "[types]")
{
    mx::Vector3 v1(1, 2, 3);
    mx::Vector3 v2(2, 4, 6);

    // Indexing operators
    REQUIRE(v1[2] == 3);
    v1[2] = 4;
    REQUIRE(v1[2] == 4);
    v1[2] = 3;

    // Component-wise operators
    REQUIRE(v2 + v1 == mx::Vector3(3, 6, 9));
    REQUIRE(v2 - v1 == mx::Vector3(1, 2, 3));
    REQUIRE(v2 * v1 == mx::Vector3(2, 8, 18));
    REQUIRE(v2 / v1 == mx::Vector3(2, 2, 2));
    REQUIRE((v2 += v1) == mx::Vector3(3, 6, 9));
    REQUIRE((v2 -= v1) == mx::Vector3(2, 4, 6));
    REQUIRE((v2 *= v1) == mx::Vector3(2, 8, 18));
    REQUIRE((v2 /= v1) == mx::Vector3(2, 4, 6));
    REQUIRE(v1 * 2 == mx::Vector3(2, 4, 6));
    REQUIRE(2 * v1 == mx::Vector3(2, 4, 6));
}

TEST_CASE("Matrix operators", "[types]")
{
    mx::Matrix44 trans(1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       3, 0, 0, 1);
    mx::Matrix44 scale(2, 0, 0, 0,
                       0, 2, 0, 0,
                       0, 0, 2, 0,
                       0, 0, 0, 1);

    // Indexing operators
    REQUIRE(trans[3][0] == 3);
    trans[3][0] = 4;
    REQUIRE(trans[3][0] == 4);
    trans[3][0] = 3;

    // Matrix multiplication
    mx::Matrix44 prod1 = trans * scale;
    mx::Matrix44 prod2 = scale * trans;
    mx::Matrix44 prod3 = trans * 2;
    mx::Matrix44 prod4 = 2 * trans;
    REQUIRE(prod1 == mx::Matrix44(2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 2, 0,
                                  6, 0, 0, 1));
    REQUIRE(prod2 == mx::Matrix44(2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 2, 0,
                                  3, 0, 0, 1));
    REQUIRE(prod3 == mx::Matrix44(2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 2, 0,
                                  6, 0, 0, 2));
    REQUIRE(prod4 == prod3);
    mx::Matrix44 prod5 = prod1;
    prod5 *= trans;
    REQUIRE(prod5 == mx::Matrix44(2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 2, 0,
                                  9, 0, 0, 1));

    // Matrix division
    mx::Matrix44 quot1 = prod1 / scale;
    mx::Matrix44 quot2 = prod2 / trans;
    mx::Matrix44 quot3 = prod3 / 2;
    REQUIRE(quot1 == trans);
    REQUIRE(quot2 == scale);
    REQUIRE(quot3 == trans);
    mx::Matrix44 quot4 = quot1;
    quot4 /= trans;
    REQUIRE(quot4 == mx::Matrix44::IDENTITY);
}
