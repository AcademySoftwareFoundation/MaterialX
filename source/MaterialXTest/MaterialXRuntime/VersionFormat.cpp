#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXRuntime/RtVersionResolver.h>

namespace mx = MaterialX;

TEST_CASE("Version formatting", "[version_format]")
{
    // Correct cases for whole integer based version format
    REQUIRE(mx::isValidIntegerVersionFormat("prefix#postfix") == true);
    REQUIRE(mx::isValidIntegerVersionFormat("#postfix") == true);
    REQUIRE(mx::isValidIntegerVersionFormat("prefix#") == true);
    REQUIRE(mx::isValidIntegerVersionFormat("#") == true);
    REQUIRE(mx::isValidIntegerVersionFormat("prefix_#") == true);

    // Incorrect cases for whole integer based verison format
    REQUIRE(mx::isValidIntegerVersionFormat("pref342ix#postfix") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("#post432fix") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("pre423fix#") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("234#") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("prefi234x_#") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("prefix##") == false);
    REQUIRE(mx::isValidIntegerVersionFormat("prefix##middle#") == false);
    
    // Correct cases for whole float based version format
    REQUIRE(mx::isValidFloatVersionFormat("prefix.#postfix") == true);
    REQUIRE(mx::isValidFloatVersionFormat("prefix.##postfix") == true);
    REQUIRE(mx::isValidFloatVersionFormat("prefix.######postfix") == true);
    REQUIRE(mx::isValidFloatVersionFormat("prefix.#") == true);
    REQUIRE(mx::isValidFloatVersionFormat(".#postfix") == true);
    REQUIRE(mx::isValidFloatVersionFormat(".#") == true);
    REQUIRE(mx::isValidFloatVersionFormat(".##") == true);

    // Incorrect cases for whole float based version format
    REQUIRE(mx::isValidFloatVersionFormat("pre3fix.#postfix") == false);
    REQUIRE(mx::isValidFloatVersionFormat("prefix.##pos4tfix") == false);
    REQUIRE(mx::isValidFloatVersionFormat("pre43fix.######pos3tfix") == false);
    REQUIRE(mx::isValidFloatVersionFormat("pre34fix.#") == false);
    REQUIRE(mx::isValidFloatVersionFormat(".#po43stfix") == false);
    REQUIRE(mx::isValidFloatVersionFormat(".#43") == false);
    REQUIRE(mx::isValidFloatVersionFormat("43.###") == false);
    REQUIRE(mx::isValidFloatVersionFormat("prefix.###middle###") == false);
    REQUIRE(mx::isValidFloatVersionFormat("#.#") == false);

    // Version format correct validatin (both integer and flot)
    REQUIRE(mx::isValidVersionFormat("prefix#") == true);
    REQUIRE(mx::isValidVersionFormat("prefix.#") == true);
    REQUIRE(mx::isValidVersionFormat("prefix#postfix") == true);
    REQUIRE(mx::isValidVersionFormat("prefix.#postfix") == true);
    REQUIRE(mx::isValidVersionFormat("#postfix") == true);
    REQUIRE(mx::isValidVersionFormat(".#postfix") == true);


    // Incorrect Version format validatin (both integer and flot)
    REQUIRE(mx::isValidVersionFormat("pref234ix#") == false);
    REQUIRE(mx::isValidVersionFormat("pref4ix.#") == false);
    REQUIRE(mx::isValidVersionFormat("pre3fix#pos43tfix") == false);
    REQUIRE(mx::isValidVersionFormat("pre4fix.#po4stfix") == false);
    REQUIRE(mx::isValidVersionFormat("#po43stfix") == false);
    REQUIRE(mx::isValidVersionFormat(".#po3stfix") == false);
    REQUIRE(mx::isValidVersionFormat("prefix##") == false);
    REQUIRE(mx::isValidVersionFormat("prefix#.#") == false);
    REQUIRE(mx::isValidVersionFormat("prefix##postfix") == false);
    REQUIRE(mx::isValidVersionFormat("prefix#.#postfix") == false);
    REQUIRE(mx::isValidVersionFormat("##postfix") == false);
    REQUIRE(mx::isValidVersionFormat("#.#postfix") == false);

    // Version format precision test
    REQUIRE(mx::getVersionFormatDecimalPrecision("#") == 0);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix#") == 0);
    REQUIRE(mx::getVersionFormatDecimalPrecision("#postfix") == 0);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix#postfix") == 0);

    REQUIRE(mx::getVersionFormatDecimalPrecision(".#") == 1);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.#") == 1);
    REQUIRE(mx::getVersionFormatDecimalPrecision(".#postfix") == 1);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.#postfix") == 1);

    REQUIRE(mx::getVersionFormatDecimalPrecision(".##") == 2);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.##") == 2);
    REQUIRE(mx::getVersionFormatDecimalPrecision(".##postfix") == 2);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.##postfix") == 2);

    REQUIRE(mx::getVersionFormatDecimalPrecision(".###") == 3);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.###") == 3);
    REQUIRE(mx::getVersionFormatDecimalPrecision(".###postfix") == 3);
    REQUIRE(mx::getVersionFormatDecimalPrecision("prefix.###postfix") == 3);

    // Version format increment step tests
    REQUIRE(mx::getVersionIncrementStep(0) == 1.0);
    REQUIRE(mx::getVersionIncrementStep(1) == 0.1);
    REQUIRE(mx::getVersionIncrementStep(2) == 0.01);
    REQUIRE(mx::getVersionIncrementStep(3) == 0.001);
    REQUIRE(mx::getVersionIncrementStep(4) == 0.0001);
    REQUIRE(mx::getVersionIncrementStep(5) == 0.00001);


    // Substituting the version number into the Version Format string
    REQUIRE(mx::getFormattedVersionString("1", "#") == "1");
    REQUIRE(mx::getFormattedVersionString("1", "prefix#") == "prefix1");
    REQUIRE(mx::getFormattedVersionString("1", "#postfix") == "1postfix");
    REQUIRE(mx::getFormattedVersionString("1", "prefix#postfix") == "prefix1postfix");

    REQUIRE(mx::getFormattedVersionString("1.1", ".#") == "1.1");
    REQUIRE(mx::getFormattedVersionString("1.1", "prefix.#") == "prefix1.1");
    REQUIRE(mx::getFormattedVersionString("1.1", ".#postfix") == "1.1postfix");
    REQUIRE(mx::getFormattedVersionString("1.1", "prefix.#postfix") == "prefix1.1postfix");
    
}