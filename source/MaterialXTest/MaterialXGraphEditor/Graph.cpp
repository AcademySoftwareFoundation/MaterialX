//
// Created by devel on 2023-07-25.
//
#include <MaterialXTest/External/Catch/catch.hpp>


TEST_CASE("Graph Editor!!", "[grapheditor]")
{
    // Test simple text substitution
    std::string test1 = "Look behind you, a $threeheaded $monkey!";
    std::string result1 = "Look behind you, a mighty pirate!";

    REQUIRE(1 == 2);
}