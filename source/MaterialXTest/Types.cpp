//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>

namespace mx = MaterialX;

TEST_CASE("Vectors", "[types]")
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
    REQUIRE(v1 * 2 == v2);
    REQUIRE(v2 / 2 == v1);
    
    // Geometric operators
    mx::Vector3 v3(1, 2, 2);
    REQUIRE(v3.getMagnitude() == 3);
    mx::Vector3 normalized_v3 = v3.getNormalized();
    REQUIRE(normalized_v3.getMagnitude() == 1);
}
#include <iostream>

TEST_CASE("Matrices", "[types]")
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

    // Matrix methods
    REQUIRE(trans.getTranspose() == mx::Matrix44(1, 0, 0, 3,
                                                 0, 1, 0, 0,
                                                 0, 0, 1, 0,
                                                 0, 0, 0, 1));
    REQUIRE(scale.getTranspose() == scale);
    REQUIRE(trans.getDeterminant() == 1);
    REQUIRE(scale.getDeterminant() == 8);
    REQUIRE(trans.getInverse() == mx::Matrix44(1, 0, 0, 0,
                                               0, 1, 0, 0,
                                               0, 0, 1, 0,
                                              -3, 0, 0, 1));

    // Matrix product
    mx::Matrix44 prod1 = trans * scale;
    mx::Matrix44 prod2 = scale * trans;
    mx::Matrix44 prod3 = trans * 2;
    mx::Matrix44 prod4 = prod3 / 2;
    mx::Matrix44 prod5 = prod1;
    prod5 *= trans;
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
    REQUIRE(prod4 == trans);
    REQUIRE(prod5 == mx::Matrix44(2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 2, 0,
                                  9, 0, 0, 1));

    // Matrix division
    mx::Matrix44 quot1 = prod1 / scale;
    mx::Matrix44 quot2 = prod2 / trans;
    mx::Matrix44 quot3 = prod3 / 2;
    mx::Matrix44 quot4 = quot1;
    quot4 /= trans;
    REQUIRE(quot1 == trans);
    REQUIRE(quot2 == scale);
    REQUIRE(quot3 == trans);
    REQUIRE(quot4 == mx::Matrix44::IDENTITY);

    // Matrix translation
    mx::Vector3 amount44{ 1.0f, 2.0f, 3.0f };
    mx::Matrix44 trans44 = mx::Matrix44::createTranslation(amount44);
    mx::Vector3 result44(trans44[3][0], trans44[3][1], trans44[3][2]);
    REQUIRE(amount44 == result44);
    amount44 = { -1.0f, -2.0f, -3.0f };
    mx::Matrix44 translateBy = mx::Matrix44::createTranslation(amount44);
    mx::Matrix44 translateResult = trans44 * translateBy;
    REQUIRE(translateResult == mx::Matrix44::IDENTITY);

    mx::Vector2 amount33{ 5.0f, 10.0f };
    mx::Matrix33 trans33 = mx::Matrix33::createTranslation(amount33);
    mx::Vector2 result33(trans33[2][0], trans33[2][1]);
    REQUIRE(amount33 == result33);
    amount33 = { -5.0f, -10.0f };
    mx::Matrix33 translateBy33 = mx::Matrix33::createTranslation(amount33);
    mx::Matrix33 translateResult33 = trans33 * translateBy33;
    REQUIRE(translateResult33 == mx::Matrix33::IDENTITY);

    // Matrix rotation
    const float DEG_TO_RADIANS = 3.14159265358979323846f / 180.0f;
    const float angleX = 33.0f * DEG_TO_RADIANS;
    const float angleY = 33.0f * DEG_TO_RADIANS;
    const float angleZ = 33.0f * DEG_TO_RADIANS;
    mx::Matrix44 rotate44(mx::Matrix44::IDENTITY);
    mx::Matrix44 rotateBy = mx::Matrix44::createRotationX(angleX);
    mx::Matrix44 rotateResult = rotate44 * rotateBy;
    mx::Matrix44 rotcheck1(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 0.838671f, -0.544639f, 0.0f,
                           0.0f, 0.544639f, 0.838671f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck1.equivalent(rotateResult, 0.000001f));

    rotateBy = mx::Matrix44::createRotationY(angleY);
    rotateResult = rotateResult * rotateBy;
    mx::Matrix44 rotcheck2(0.838671f, 0.0f, 0.544639f, 0.0f,
                           0.296632f, 0.838671f, -0.456773f, 0.0f,
                           -0.456773f, 0.544639f, 0.703368f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck2.equivalent(rotateResult, 0.000001f));

    rotateBy = mx::Matrix44::createRotationZ(angleZ);
    rotateResult = rotateResult * rotateBy;
    mx::Matrix44 rotcheck3(0.703368f, -0.456773f, 0.544639f, 0.0f,
                            0.705549f, 0.541811f, -0.456773f, 0.0f,
                            -0.086450f, 0.705549f, 0.703368f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck3.equivalent(rotateResult, 0.000001f));

    mx::Matrix33 rotate33 = mx::Matrix33::createRotation(angleZ);
    mx::Matrix33 rotcheck4(0.838671f, -0.544639f, 0.0f,
                            0.544639f, 0.838671f, 0.0f,
                            0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck4.equivalent(rotate33, 0.000001f));

    // Matrix scale
    mx::Matrix44 scale44(2, 0, 0, 0,
                         0, 4, 0, 0,
                         0, 0, 6, 0,
                         1, 3, 5, 1);
    mx::Vector3 scalar{ 8, 7, 6 };
    mx::Matrix44 scaleBy = mx::Matrix44::createScale(scalar);
    mx::Matrix44 scaleResult = scale44 * scaleBy;
    mx::Matrix44 scaleCheck1(16.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 28.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 36.0f, 0.0f,
                             8.0f, 21.0f, 30.0f, 1.0f);
    REQUIRE(scaleCheck1.equivalent(scaleResult, 0.000001f));

    mx::Matrix33 scale33(2, 0, 0,
        0, 4, 0,
        1, 3, 1);
    mx::Vector2 scalar3{ 8, 7 };
    mx::Matrix33 scaleBy33 = mx::Matrix33::createScale(scalar3);
    mx::Matrix33 scaleResult33 = scale33 * scaleBy33;
    mx::Matrix33 scaleCheck2(16.0f, 0.0f, 0.0f,
                             0.0f, 28.0f, 0.0f,
                             8.0f, 21.0f, 1.0f);
    REQUIRE(scaleCheck2.equivalent(scaleResult33, 0.000001f));    
}
