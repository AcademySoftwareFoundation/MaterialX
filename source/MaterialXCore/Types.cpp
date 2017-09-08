//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>

#include <iterator>
#include <sstream>

namespace MaterialX
{

const string DEFAULT_TYPE_STRING = "color3";
const string FILENAME_TYPE_STRING = "filename";
const string SURFACE_SHADER_TYPE_STRING = "surfaceshader";
const string VOLUME_SHADER_TYPE_STRING = "volumeshader";
const string VALUE_STRING_TRUE = "true";
const string VALUE_STRING_FALSE = "false";
const string NAME_PATH_SEPARATOR = "/";

std::istream& operator>>(std::istream& is, vector<string>& v)
{
    while (!is.eof())
    {
        is >> std::ws;
        if (is.eof())
            break;

        std::stringbuf sb;
        is.get(sb, ',');

        if (sb.in_avail())
            v.push_back(sb.str());

        if (!is.eof())
            is.ignore();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const vector<string>& v)
{
    if (!v.empty())
    {
        std::copy(v.begin(), v.end() - 1, std::ostream_iterator<string>(os, ", "));
        os << *v.rbegin();
    }

    return os;
}
} // namespace MaterialX
