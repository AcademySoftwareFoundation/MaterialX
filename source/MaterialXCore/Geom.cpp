//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Geom.h>

namespace MaterialX
{

const string UNIVERSAL_GEOM_NAME = "*";
const string UDIM_TOKEN = "%UDIM";

const string GeomElement::GEOM_ATTRIBUTE = "geom";
const string GeomElement::COLLECTION_ATTRIBUTE = "collection";

bool geomStringsMatch(const string& geom1, const string& geom2)
{
    vector<string> vec1 = splitString(geom1, ARRAY_VALID_SEPARATORS);
    vector<string> vec2 = splitString(geom2, ARRAY_VALID_SEPARATORS);
    std::set<string> set1(vec1.begin(), vec1.end());
    std::set<string> set2(vec2.begin(), vec2.end());

    if (set1.empty() || set2.empty())
        return false;
    if (set1.count(UNIVERSAL_GEOM_NAME) || set2.count(UNIVERSAL_GEOM_NAME))
        return true;

    std::set<string> matches;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), 
                          std::inserter(matches, matches.end()));
    return !matches.empty();
}

} // namespace MaterialX
