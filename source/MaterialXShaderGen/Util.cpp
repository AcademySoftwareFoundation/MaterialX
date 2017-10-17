#include <MaterialXShaderGen/Util.h>
#include <MaterialXCore/Util.h>

#include <fstream>
#if defined(_WIN32)
#include <fcntl.h>
#endif

namespace MaterialX
{

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
    _set_fmode(oldMode);
#endif

    return result;
}

string getFileExtension(const string& filename)
{
    size_t i = filename.rfind('.');
    return i != string::npos ? filename.substr(i + 1) : EMPTY_STRING;
}


} // namespace MaterialX
