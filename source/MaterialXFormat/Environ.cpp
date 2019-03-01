//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/Environ.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Util.h>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace MaterialX
{

string getEnviron(const string& name)
{
#if defined(_WIN32)
    DWORD size = GetEnvironmentVariable(name.c_str(), nullptr, 0);

    if (size != 0 && size != ERROR_ENVVAR_NOT_FOUND) 
    {
        std::unique_ptr<char[]> buffer(new char[size]);
        GetEnvironmentVariable(name.c_str(), buffer.get(), size);
        return string(buffer.get());
    }
#else
    if (const char* const result = getenv(name.c_str()))
    {
        return string(result);
    }

    return EMPTY_STRING;
#endif
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

}  // namespace MaterialX
