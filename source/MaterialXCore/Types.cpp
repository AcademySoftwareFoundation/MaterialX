//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

const string DEFAULT_TYPE_STRING = "color3";
const string FILENAME_TYPE_STRING = "filename";
const string SURFACE_SHADER_TYPE_STRING = "surfaceshader";
const string VOLUME_SHADER_TYPE_STRING = "volumeshader";
const string VALUE_STRING_TRUE = "true";
const string VALUE_STRING_FALSE = "false";
const string NAME_PATH_SEPARATOR = "/";
const string ARRAY_VALID_SEPARATORS = ", ";
const string ARRAY_PREFERRED_SEPARATOR = ", ";

std::istream& operator>>(std::istream& is, vector<string>& v)
{
    string str(std::istreambuf_iterator<char>(is), { });
    v = splitString(str, ARRAY_VALID_SEPARATORS);
    return is;
}

std::ostream& operator<<(std::ostream& os, const vector<string>& v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        os << v[i];
        if (i < v.size() - (size_t) 1)
        {
            os << ARRAY_PREFERRED_SEPARATOR;
        }
    }
    return os;
}

} // namespace MaterialX
