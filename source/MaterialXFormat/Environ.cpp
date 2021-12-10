//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/Environ.h>

#include <MaterialXCore/Util.h>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

MATERIALX_NAMESPACE_BEGIN

string getEnviron(const string& name)
{
#if defined(_WIN32)
    if (uint32_t size = GetEnvironmentVariable(name.c_str(), nullptr, 0))
    {
        vector<char> buffer(size);
        GetEnvironmentVariable(name.c_str(), buffer.data(), size);
        return string(buffer.data());
    }
#else
    if (const char* const result = getenv(name.c_str()))
    {
        return string(result);
    }
#endif
    return EMPTY_STRING;
}

bool setEnviron(const string& name, const string& value)
{
#if defined(_WIN32)
    return SetEnvironmentVariable(name.c_str(), value.c_str()) != 0;
#else
    return setenv(name.c_str(), value.c_str(), true);
#endif
}

bool removeEnviron(const string& name)
{
#if defined(_WIN32)
    return SetEnvironmentVariable(name.c_str(), nullptr) != 0;
#else
    return unsetenv(name.c_str()) == 0;
#endif
}

MATERIALX_NAMESPACE_END
