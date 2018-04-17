//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>

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
    
    // Geometric operators
    mx::Vector3 v3(1.0f, 2.0f, 2.0f);
    REQUIRE(v3.magnitude() == 3.0f);
    v3.normalize();
    REQUIRE(v3.magnitude() == 1.0f);
}

#include <iostream>

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

    std::string res;

    // Matrix translation
    mx::Matrix44 trans44(mx::Matrix44::IDENTITY);
    mx::Vector4 amount44(1.0f, 2.0f, 3.0f, 1.0f);
    trans44.setTranslation(amount44);
    REQUIRE(amount44 == trans44.getRow(3));
    amount44 = { -1.0f, -2.0f, -3.0f, 1.0f };
    mx::Matrix44 translateBy;
    translateBy.setTranslation(amount44);
    mx::Matrix44 translateResult = trans44 * translateBy;
    REQUIRE(translateResult == mx::Matrix44::IDENTITY);

    mx::Matrix33 trans33(mx::Matrix33::IDENTITY);
    mx::Vector3 amount33(5.0f, 10.0f, 1.0f);
    trans33.setTranslation(amount33);
    REQUIRE(amount33 == trans33.getRow(2));
    amount33 -= 2.0f*amount33;
    mx::Matrix33 translateBy33;
    translateBy33.setTranslation(amount33);
    mx::Matrix33 translateResult33 = trans33 * translateBy33;
    REQUIRE(translateResult33 == mx::Matrix33::IDENTITY);

    // Matrix rotation
    const float DEG_TO_RADIANS = 3.14159265358979323846f / 180.0f;
    const float angleX = 33.0f * DEG_TO_RADIANS;
    const float angleY = 33.0f * DEG_TO_RADIANS;
    const float angleZ = 33.0f * DEG_TO_RADIANS;
    mx::Matrix44 rotate44(mx::Matrix44::IDENTITY);
    mx::Matrix44 rotateBy;
    rotateBy.setRotationX(angleX);
    mx::Matrix44 rotateResult = rotate44 * rotateBy;
    mx::Matrix44 rotcheck1(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.838671f, -0.544639f, 0.0f,
        0.0f, 0.544639f, 0.838671f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck1.equivalent(rotateResult, 0.000001f));

    rotateBy.setRotationY(angleY);
    rotateResult = rotateResult * rotateBy;
    mx::Matrix44 rotcheck2(0.838671f, 0.0f, 0.544639f, 0.0f,
                            0.296632f, 0.838671f, -0.456773f, 0.0f,
                            -0.456773f, 0.544639f, 0.703368f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck2.equivalent(rotateResult, 0.000001f));

    rotateBy.setRotationZ(angleZ);
    rotateResult = rotateResult * rotateBy;
    mx::Matrix44 rotcheck3(0.703368f, -0.456773f, 0.544639f, 0.0f,
                            0.705549f, 0.541811f, -0.456773f, 0.0f,
                            -0.086450f, 0.705549f, 0.703368f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck3.equivalent(rotateResult, 0.000001f));

    mx::Matrix33 rotate33(mx::Matrix33::IDENTITY);
    rotate33.setRotation(angleZ);
    mx::Matrix33 rotcheck4(0.838671f, -0.544639f, 0.0f,
                            0.544639f, 0.838671f, 0.0f,
                            0.0f, 0.0f, 1.0f);
    REQUIRE(rotcheck4.equivalent(rotate33, 0.000001f));

    // Matrix scale
    mx::Matrix44 scale44(2, 0, 0, 0,
        0, 4, 0, 0,
        0, 0, 6, 0,
        1, 3, 5, 1);
    mx::Vector4 scalar(8, 7, 6, 1);
    mx::Matrix44 scaleBy;
    scaleBy.setScale(scalar);
    mx::Matrix44 scaleResult = scale44 * scaleBy;
    mx::Matrix44 scaleCheck1(16.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 28.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 36.0f, 0.0f,
        8.0f, 21.0f, 30.0f, 1.0f);
    REQUIRE(scaleCheck1.equivalent(scaleResult, 0.000001f));

    mx::Matrix33 scale33(2, 0, 0,
        0, 4, 0,
        1, 3, 1);
    mx::Vector3 scalar3(8, 7, 1);
    mx::Matrix33 scaleBy33;
    scaleBy33.setScale(scalar3);
    mx::Matrix33 scaleResult33 = scale33 * scaleBy33;
    mx::Matrix33 scaleCheck2(16.0, 0.0f, 0.0f,
        0.0f, 28.0f, 0.0f,
        8.0f, 21.0f, 1.0f);
    REQUIRE(scaleCheck2.equivalent(scaleResult33, 0.000001f));

    // Matrix transpose
    mx::Matrix44 transp44(1, 2, 3, 4,
                          5, 6, 7, 8,
                          9, 10, 11, 12,
                          13, 14, 15, 16);
    mx::Matrix44 after_transp44 (transp44);
    after_transp44.transpose();
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            REQUIRE(transp44[i][j] == after_transp44[j][i]);
        }
    }

    mx::Matrix33 transp33(1, 2, 3,
        4, 5, 6,
        7, 8, 9);
    mx::Matrix33 after_transp33(transp33);
    after_transp33.transpose();
    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            REQUIRE(transp33[i][j] == after_transp33[j][i]);
        }
    }
}
