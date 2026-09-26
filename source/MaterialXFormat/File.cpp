//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/File.h>

#include <MaterialXFormat/Environ.h>

#include <MaterialXCore/Exception.h>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <stdlib.h>
#endif

#if defined(__linux__)
    #include <linux/limits.h>
#elif defined(__FreeBSD__)
    #include <sys/syslimits.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif

#include <array>
#include <cctype>
#include <cerrno>
#include <cstring>

#if defined(_WIN32)
    #include <random>
#endif

MATERIALX_NAMESPACE_BEGIN

const string VALID_SEPARATORS = "/\\";

const char PREFERRED_SEPARATOR_WINDOWS = '\\';
const char PREFERRED_SEPARATOR_POSIX = '/';

const string CURRENT_PATH_STRING = ".";
const string PARENT_PATH_STRING = "..";

#if defined(_WIN32)
const string PATH_LIST_SEPARATOR = ";";
#else
const string PATH_LIST_SEPARATOR = ":";
#endif
const string MATERIALX_SEARCH_PATH_ENV_VAR = "MATERIALX_SEARCH_PATH";

const string TEMPORARY_DIRECTORY_PREFIX = "materialx_tmp_";

#if defined(_WIN32)
const int TEMPORARY_DIRECTORY_MAX_ATTEMPTS = 1000;
#else
const StringVec TEMPORARY_DIRECTORY_ENV_VARS = { "TMPDIR", "TMP", "TEMP", "TEMPDIR" };
const string TEMPORARY_DIRECTORY_DEFAULT = "/tmp";
#endif

inline bool hasWindowsDriveSpecifier(const string& val)
{
    return (val.length() > 1 && std::isalpha((unsigned char) val[0]) && (val[1] == ':'));
}

//
// FilePath methods
//

void FilePath::assign(const string& str)
{
    _type = TypeRelative;
    _vec = splitString(str, VALID_SEPARATORS);
    if (!str.empty())
    {
        if (str[0] == PREFERRED_SEPARATOR_POSIX)
        {
            _type = TypeAbsolute;
        }
        else if (str.size() >= 2)
        {
            if (hasWindowsDriveSpecifier(str))
            {
                _type = TypeAbsolute;
            }
            else if (str[0] == '\\' && str[1] == '\\')
            {
                _type = TypeNetwork;
            }
        }
    }
}

string FilePath::asString(Format format) const
{
    string str;

    if (format == FormatPosix && isAbsolute())
    {
        // Don't prepend a POSIX separator on a Windows absolute path
        if (_vec.empty() || !hasWindowsDriveSpecifier(_vec[0]))
        {
            str += "/";
        }
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

    FilePath combined(*this);
    for (const string& str : rhs._vec)
    {
        combined._vec.push_back(str);
    }
    return combined;
}

FilePath FilePath::getNormalized() const
{
    FilePath res;
    for (const string& str : _vec)
    {
        if (str == CURRENT_PATH_STRING)
        {
            continue;
        }
        if (str == PARENT_PATH_STRING && !res.isEmpty() && res[res.size() - 1] != PARENT_PATH_STRING)
        {
            res._vec.pop_back();
            continue;
        }
        res._vec.push_back(str);
    }
    res._type = _type;
    return res;
}

bool FilePath::exists() const
{
#if defined(_WIN32)
    uint32_t result = GetFileAttributesA(asString().c_str());
    return result != INVALID_FILE_ATTRIBUTES;
#else
    struct stat sb;
    return stat(asString().c_str(), &sb) == 0;
#endif
}

bool FilePath::isDirectory() const
{
#if defined(_WIN32)
    uint32_t result = GetFileAttributesA(asString().c_str());
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
    WIN32_FIND_DATAA fd;
    FilePath query = extension.empty() ? (*this / "*") : (*this / ("*." + extension));
    HANDLE hFind = FindFirstFileA(query.asString().c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                files.emplace_back(fd.cFileName);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(asString().c_str());
    if (dir)
    {
        while (struct dirent* entry = readdir(dir))
        {
            if (entry->d_type != DT_DIR && (extension.empty() || FilePath(entry->d_name).getExtension() == extension))
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

    FilePathVec dirs{ *this };

#if defined(_WIN32)
    WIN32_FIND_DATAA fd;
    string wildcard = "*";
    HANDLE hFind = FindFirstFileA((*this / wildcard).asString().c_str(), &fd);
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
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(asString().c_str());
    if (dir)
    {
        while (struct dirent* entry = readdir(dir))
        {
            string path = entry->d_name;
            if (path == "." || path == "..")
            {
                continue;
            }

            auto d_type = entry->d_type;
            FilePath newDir = *this / path;

            if (d_type == DT_UNKNOWN)
            {
                if (newDir.isDirectory())
                {
                    d_type = DT_DIR;
                }
            }
            if (d_type == DT_DIR)
            {
                FilePathVec newDirs = newDir.getSubDirectories();
                dirs.insert(dirs.end(), newDirs.begin(), newDirs.end());
            }
        }
        closedir(dir);
    }
#endif

    return dirs;
}

void FilePath::createDirectory(bool recursive) const
{
    if (recursive)
    {
        if (isEmpty() || exists())
        {
            return;
        }

        FilePath parent = getParentPath();
        if (!parent.isEmpty() && !parent.exists())
        {
            parent.createDirectory(true);
        }
    }

#if defined(_WIN32)
    _mkdir(asString().c_str());
#else
    mkdir(asString().c_str(), 0777);
#endif
}

bool FilePath::removeDirectory(bool recursive) const
{
    if (recursive)
    {
        // Remove the contents of the directory, taking care not to follow symbolic
        // links to locations outside of this directory.
#if defined(_WIN32)
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA((*this / "*").asString().c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                string entryName = fd.cFileName;
                if (entryName == CURRENT_PATH_STRING || entryName == PARENT_PATH_STRING)
                {
                    continue;
                }

                FilePath entryPath = *this / entryName;
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                {
                    entryPath.removeDirectory(true);
                }
                else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    RemoveDirectoryA(entryPath.asString().c_str());
                }
                else
                {
                    DeleteFileA(entryPath.asString().c_str());
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(asString().c_str());
        if (dir)
        {
            while (struct dirent* entry = readdir(dir))
            {
                string entryName = entry->d_name;
                if (entryName == CURRENT_PATH_STRING || entryName == PARENT_PATH_STRING)
                {
                    continue;
                }

                FilePath entryPath = *this / entryName;
                struct stat sb;
                if (lstat(entryPath.asString().c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
                {
                    entryPath.removeDirectory(true);
                }
                else
                {
                    unlink(entryPath.asString().c_str());
                }
            }
            closedir(dir);
        }
#endif
    }

#if defined(_WIN32)
    return (RemoveDirectoryA(asString().c_str()) != 0);
#else
    return (rmdir(asString().c_str()) == 0);
#endif
}

bool FilePath::setCurrentPath()
{
#if defined(_WIN32)
    return (_chdir(asString().c_str()) == 0);
#else
    return (chdir(asString().c_str()) == 0);
#endif
}

FilePath FilePath::getCurrentPath()
{
#if defined(_WIN32)
    std::array<char, MAX_PATH> buf;
    if (!GetCurrentDirectoryA(MAX_PATH, buf.data()))
    {
        throw Exception("Error in getCurrentPath: " + std::to_string(GetLastError()));
    }
    return FilePath(buf.data());
#else
    std::array<char, PATH_MAX> buf;
    if (getcwd(buf.data(), PATH_MAX) == NULL)
    {
        throw Exception("Error in getCurrentPath: " + string(strerror(errno)));
    }
    return FilePath(buf.data());
#endif
}

FilePath FilePath::getModulePath()
{
#if defined(_WIN32)
    vector<char> buf(MAX_PATH);
    while (true)
    {
        uint32_t reqSize = GetModuleFileNameA(NULL, buf.data(), (uint32_t) buf.size());
        if (!reqSize)
        {
            throw Exception("Error in getModulePath: " + std::to_string(GetLastError()));
        }
        else if ((size_t) reqSize >= buf.size())
        {
            buf.resize(buf.size() * 2);
        }
        else
        {
            return FilePath(buf.data()).getParentPath();
        }
    }
#elif defined(__APPLE__)
    vector<char> buf(PATH_MAX);
    while (true)
    {
        uint32_t reqSize = static_cast<uint32_t>(buf.size());
        if (_NSGetExecutablePath(buf.data(), &reqSize) == -1)
        {
            buf.resize((size_t) reqSize);
        }
        else
        {
            return FilePath(buf.data()).getParentPath();
        }
    }
#else
    vector<char> buf(PATH_MAX);
    while (true)
    {
        ssize_t reqSize = readlink("/proc/self/exe", buf.data(), buf.size());
        if (reqSize == -1)
        {
            throw Exception("Error in getModulePath: " + string(strerror(errno)));
        }
        else if ((size_t) reqSize >= buf.size())
        {
            buf.resize(buf.size() * 2);
        }
        else
        {
            buf.data()[reqSize] = '\0';
            return FilePath(buf.data()).getParentPath();
        }
    }
#endif
}

FilePath FilePath::createTemporaryDirectory(const FilePath& parentDir)
{
    FilePath tempParent = parentDir.isEmpty() ? getSystemTemporaryDirectory() : parentDir;
    tempParent.createDirectory(true);
    if (!tempParent.isDirectory())
    {
        throw Exception("Error in createTemporaryDirectory: parent path is not a directory: " + tempParent.asString());
    }

#if defined(_WIN32)
    // Windows provides no atomic equivalent of mkdtemp, so generate candidate names
    // until CreateDirectory succeeds.  Because CreateDirectory fails rather than
    // succeeding when the path already exists, no other process can be given the
    // same directory.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(100000, 999999);

    for (int attempt = 0; attempt < TEMPORARY_DIRECTORY_MAX_ATTEMPTS; attempt++)
    {
        FilePath tempDir = tempParent / (TEMPORARY_DIRECTORY_PREFIX + std::to_string(dis(gen)));
        if (CreateDirectoryA(tempDir.asString().c_str(), nullptr))
        {
            return tempDir;
        }
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            throw Exception("Error in createTemporaryDirectory: " + std::to_string(GetLastError()));
        }
    }

    throw Exception("Error in createTemporaryDirectory: no unique name found in " + tempParent.asString());
#else
    // Create the directory with mkdtemp, which generates a unique name, creates the
    // directory atomically, and restricts access to the current user.
    string nameTemplate = (tempParent / (TEMPORARY_DIRECTORY_PREFIX + "XXXXXX")).asString();
    vector<char> buf(nameTemplate.begin(), nameTemplate.end());
    buf.push_back('\0');
    if (!mkdtemp(buf.data()))
    {
        throw Exception("Error in createTemporaryDirectory: " + string(strerror(errno)));
    }
    return FilePath(buf.data());
#endif
}

FilePath FilePath::getSystemTemporaryDirectory()
{
#if defined(_WIN32)
    std::array<char, MAX_PATH + 1> buf;
    DWORD length = GetTempPathA((DWORD) buf.size(), buf.data());
    if (!length || length > buf.size())
    {
        throw Exception("Error in getSystemTemporaryDirectory: " + std::to_string(GetLastError()));
    }
    return FilePath(string(buf.data(), length));
#else
    for (const string& envVar : TEMPORARY_DIRECTORY_ENV_VARS)
    {
        string tempDir = getEnviron(envVar);
        if (!tempDir.empty())
        {
            return FilePath(tempDir);
        }
    }
    return FilePath(TEMPORARY_DIRECTORY_DEFAULT);
#endif
}

FileSearchPath getEnvironmentPath(const string& sep)
{
    string searchPathEnv = getEnviron(MATERIALX_SEARCH_PATH_ENV_VAR);
    return FileSearchPath(searchPathEnv, sep);
}

MATERIALX_NAMESPACE_END
