#include <MaterialXShaderGen/Util.h>
#include <MaterialXCore/Util.h>

#include <fstream>

namespace MaterialX
{

bool readFile(const string& filename, string& contents)
{
    std::ifstream file(filename, std::ios::in);
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
        return true;
    }
    return false;
}

string getFileExtension(const string& filename)
{
    size_t i = filename.rfind('.');
    return i != string::npos ? filename.substr(i + 1) : EMPTY_STRING;
}


} // namespace MaterialX
