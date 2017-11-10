//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>

namespace mx = MaterialX;

template<class T> void testTypedValue(const T& v1, const T& v2)
{
    T v0(mx::TypedValue<T>::ZERO);

    // Constructor and assignment
    mx::ValuePtr value0 = mx::Value::createValue(v0);
    mx::ValuePtr value1 = mx::Value::createValue(v1);
    mx::ValuePtr value2 = mx::Value::createValue(v2);
    REQUIRE(value0->isA<T>());
    REQUIRE(value1->isA<T>());
    REQUIRE(value2->isA<T>());
    REQUIRE(value0->asA<T>() == v0);
    REQUIRE(value1->asA<T>() == v1);
    REQUIRE(value2->asA<T>() == v2);

    // Equality and inequality
    REQUIRE(value0->copy()->asA<T>() == value0->asA<T>());
    REQUIRE(value1->copy()->asA<T>() == value1->asA<T>());
    REQUIRE(value2->copy()->asA<T>() == value2->asA<T>());
    REQUIRE(value1->asA<T>() != value2->asA<T>());

    // Serialization and deserialization
    mx::ValuePtr newValue0 = mx::TypedValue<T>::createFromString(value0->getValueString());
    mx::ValuePtr newValue1 = mx::TypedValue<T>::createFromString(value1->getValueString());
    mx::ValuePtr newValue2 = mx::TypedValue<T>::createFromString(value2->getValueString());
    REQUIRE(newValue0->asA<T>() == v0);
    REQUIRE(newValue1->asA<T>() == v1);
    REQUIRE(newValue2->asA<T>() == v2);
}

TEST_CASE("Typed values", "[value]")
{
    testTypedValue<int>(1, 2);
    testTypedValue<bool>(false, true);
    testTypedValue<float>(1.0f, 2.0f);
    testTypedValue(mx::Color2(0.1f, 0.2f),
                   mx::Color2(0.5f, 0.6f));
    testTypedValue(mx::Color3(0.1f, 0.2f, 0.3f),
                   mx::Color3(0.5f, 0.6f, 0.7f));
    testTypedValue(mx::Color4(0.1f, 0.2f, 0.3f, 0.4f),
                   mx::Color4(0.5f, 0.6f, 0.7f, 0.8f));
    testTypedValue(mx::Vector2(1.0f, 2.0f),
                   mx::Vector2(1.5f, 2.5f));
    testTypedValue(mx::Vector3(1.0f, 2.0f, 3.0f),
                   mx::Vector3(1.5f, 2.5f, 3.5f));
    testTypedValue(mx::Vector4(1.0f, 2.0f, 3.0f, 4.0f),
                   mx::Vector4(1.5f, 2.5f, 3.5f, 4.5f));
    testTypedValue(mx::Matrix3x3(0.0f),
                   mx::Matrix3x3(1.0f));
    testTypedValue(mx::Matrix4x4(0.0f),
                   mx::Matrix4x4(1.0f));
    testTypedValue(std::string("first_value"),
                   std::string("second_value"));
    testTypedValue(std::vector<std::string>{"one", "two", "three"},
                   std::vector<std::string>{"four", "five", "six"});
}
