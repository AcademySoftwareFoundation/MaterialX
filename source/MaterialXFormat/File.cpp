//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/File.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#if defined(__linux)
#include <linux/limits.h>
#endif

#include <cctype>
#include <sstream>

namespace MaterialX
{

const string VALID_SEPARATORS_WINDOWS = "/\\";
const string VALID_SEPARATORS_POSIX = "/";

const char PREFERRED_SEPARATOR_WINDOWS = '\\';
const char PREFERRED_SEPARATOR_POSIX = '/';

//
// FilePath methods
//

void FilePath::assign(const string& str, Format format)
{
    _type = TypeRelative;
    if (format == FormatWindows)
    {
        _vec = splitString(str, VALID_SEPARATORS_WINDOWS);
        if (str.size() >= 2)
        {
            if (std::isalpha(str[0]) && str[1] == ':')
            {
                _type = TypeAbsolute;
            }
            else if (str[0] == '\\' && str[1] == '\\')
            {
                _type = TypeNetwork;
            }
        }
    }
    else
    {
        _vec = splitString(str, VALID_SEPARATORS_POSIX);
        if (!str.empty() && str[0] == PREFERRED_SEPARATOR_POSIX)
        {
            _type = TypeAbsolute;
        }
    }
    _format = format;
}

string FilePath::asString(Format format) const
{
    std::ostringstream stream;

    if (format == FormatPosix && isAbsolute())
    {
        stream << "/";
    }
    else if (format == FormatWindows && _type == TypeNetwork)
    {
        stream << "\\\\";
    }

    for (size_t i = 0; i < _vec.size(); i++)
    {
        stream << _vec[i];
        if (i + 1 < _vec.size())
        {
            if (format == FormatPosix)
            {
                stream << PREFERRED_SEPARATOR_POSIX;
            }
            else
            {
                stream << PREFERRED_SEPARATOR_WINDOWS;
            }
        }
    }

    return stream.str();
}

FilePath FilePath::operator/(const FilePath& rhs) const
{
    if (rhs.isAbsolute())
    {
        throw Exception("Appended path must be relative.");
    }
    if (_format != rhs._format)
    {
        throw Exception("Appended path must have the same format.");
    }

    FilePath combined(*this);
    for (const string& str : rhs._vec)
    {
        combined._vec.push_back(str);
    }
    return combined;
}

bool FilePath::exists() const
{
#if defined(_WIN32)
    return GetFileAttributes(asString().c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat sb;
    return stat(asString().c_str(), &sb) == 0;
#endif
}

FilePath FilePath::getCurrentPath()
{
#if defined(_WIN32)
    char buf[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, buf))
    {
        throw Exception("Error in getCurrentPath: " + std::to_string(GetLastError()));
    }
    return FilePath(buf);
#else
    char buf[PATH_MAX];
    if (getcwd(buf, PATH_MAX) == NULL)
    {
        throw Exception("Error in getCurrentPath: " + string(strerror(errno)));
    }
    return FilePath(buf);
#endif
}

} // namespace MaterialX
