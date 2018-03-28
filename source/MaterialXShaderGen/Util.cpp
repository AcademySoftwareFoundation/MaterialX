#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXCore/Util.h>

#include <stack>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <fcntl.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

namespace MaterialX
{

void getDocumentsInDirectory(const std::string& directory, StringVec& files)
{
#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile((directory + "/*.mtlx").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                files.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    struct dirent *entry = nullptr;
    DIR* dir = opendir(directory.c_str());
    if (dir)
    {
        while ((entry = readdir(dir)))
        {
            if (entry->d_type != DT_DIR && mx::getFileExtension(entry->d_name) == "mtlx")
            {
                files.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
}

bool readFile(const string& filename, string& contents)
{
#if defined(_WIN32)
    // Protection in case someone sets fmode to binary
    int oldMode;
    _get_fmode(&oldMode);
    _set_fmode(_O_TEXT);
#endif

    bool result = false;
    
    std::ifstream file(filename, std::ios::in );
    if (file)
    {
        string buffer;
        file.seekg(0, std::ios::end);
        buffer.resize(size_t(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(&buffer[0], buffer.size());
        file.close();
        if (buffer.length() > 0)
        {
            size_t pos = buffer.find_last_not_of('\0');
            contents = buffer.substr(0, pos + 1);
        }
        result = true;
    }
#if defined(_WIN32)
    _set_fmode(oldMode ? oldMode : _O_TEXT);
#endif

    return result;
}

string getFileExtension(const string& filename)
{
    size_t i = filename.rfind('.');
    return i != string::npos ? filename.substr(i + 1) : EMPTY_STRING;
}

} // namespace MaterialX
