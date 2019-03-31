//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/File.h>

#include <MaterialXFormat/Environ.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstring>

namespace MaterialX
{

const string VALID_SEPARATORS_WINDOWS = "/\\";
const string VALID_SEPARATORS_POSIX = "/";

const char PREFERRED_SEPARATOR_WINDOWS = '\\';
const char PREFERRED_SEPARATOR_POSIX = '/';

#if defined(_WIN32)
const string PATH_LIST_SEPARATOR = ";";
#else
const string PATH_LIST_SEPARATOR = ":";
#endif
const string MATERIALX_SEARCH_PATH_ENV_VAR = "MATERIALX_SEARCH_PATH";

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
    string str;

    if (format == FormatPosix && isAbsolute())
    {
        str += "/";
    }
    else if (format == FormatWindows && _type == TypeNetwork)
    {
        str += "\\\\";
    }

    for (size_t i = 0; i < _vec.size(); i++)
    {
        str += _vec[i];
        if (i + 1 < _vec.size())
        {
            if (format == FormatPosix)
            {
                str += PREFERRED_SEPARATOR_POSIX;
            }
            else
            {
                str += PREFERRED_SEPARATOR_WINDOWS;
            }
        }
    }

    return str;
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
    uint32_t result = GetFileAttributes(asString().c_str());
    return result != INVALID_FILE_ATTRIBUTES;
#else
    struct stat sb;
    return stat(asString().c_str(), &sb) == 0;
#endif
}

bool FilePath::isDirectory() const
{
#if defined(_WIN32)
    uint32_t result = GetFileAttributes(asString().c_str());
    if (result == INVALID_FILE_ATTRIBUTES)
        return false;
    return (result & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat sb;
    if (stat(asString().c_str(), &sb))
        return false;
    return S_ISDIR(sb.st_mode);
#endif
}

FilePathVec FilePath::getFilesInDirectory(const string& extension) const
{
    FilePathVec files;

#if defined(_WIN32)
    WIN32_FIND_DATA fd;
    string wildcard = "*." + extension;
    HANDLE hFind = FindFirstFile((*this / wildcard).asString().c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                files.push_back(FilePath(fd.cFileName));
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(asString().c_str());
    if (dir)
    {
        while (struct dirent* entry = readdir(dir))
        {
            if (entry->d_type != DT_DIR && FilePath(entry->d_name).getExtension() == extension)
            {
                files.push_back(FilePath(entry->d_name));
            }
        }
        closedir(dir);
    }
#endif

    return files;
}

FilePathVec FilePath::getSubDirectories() const
{
    if (!isDirectory())
    {
        return FilePathVec();
    }

    FilePathVec dirs { *this };

#if defined(_WIN32)
    WIN32_FIND_DATA fd;
    string wildcard = "*";
    HANDLE hFind = FindFirstFile((*this / wildcard).asString().c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string path = fd.cFileName;
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (path != "." && path != ".."))
            {
                FilePath newDir = *this / path;
                FilePathVec newDirs = newDir.getSubDirectories();
                dirs.insert(dirs.end(), newDirs.begin(), newDirs.end());
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(asString().c_str());
    if (dir)
    {
        while (struct dirent* entry = readdir(dir))
        {
            string path = entry->d_name;
            if (entry->d_type == DT_DIR && (path != "." && path != ".."))
            {
                FilePath newDir = *this / path;
                FilePathVec newDirs = newDir.getSubDirectories();
                dirs.insert(dirs.end(), newDirs.begin(), newDirs.end());
            }
        }
        closedir(dir);
    }
#endif

    return dirs;
}

void FilePath::createDirectory()
{
#if defined(_WIN32)
    _mkdir(asString().c_str());
#else
    mkdir(asString().c_str(), 0777);
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

FileSearchPath getEnvironmentPath(const string& sep)
{
    string searchPathEnv = getEnviron(MATERIALX_SEARCH_PATH_ENV_VAR);
    return FileSearchPath(searchPathEnv, sep);
}

} // namespace MaterialX
