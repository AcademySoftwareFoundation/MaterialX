#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/SgNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXCore/Util.h>

#include <stack>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <fcntl.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace MaterialX
{

void makeDirectory(const std::string& directoryPath)
{
#ifdef _WIN32
    _mkdir(directoryPath.c_str());
#else
    mkdir(directoryPath.c_str(), 0777);
#endif
}

std::string removeExtension(const std::string& filename)
{
    size_t lastDot = filename.find_last_of(".");
    if (lastDot == std::string::npos) return filename;
    return filename.substr(0, lastDot);
}

void getSubDirectories(const std::string& baseDirectory, StringVec& relativePaths)
{
    relativePaths.push_back(baseDirectory);

#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile((baseDirectory + "\\*").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string filename = fd.cFileName;
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (filename != "." && filename != ".."))
            {
                std::string newBaseDirectory = baseDirectory + "\\" + filename;
                getSubDirectories(newBaseDirectory, relativePaths);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    struct dirent *entry = nullptr;
    DIR* dir = opendir(baseDirectory.c_str());
    if (dir)
    {
        while ((entry = readdir(dir)))
        {
            std::string filename = entry->d_name;
            if (entry->d_type == DT_DIR && (filename != "." && filename != ".."))
            {
                std::string newBaseDirectory = baseDirectory + "/" + filename;
                getSubDirectories(newBaseDirectory, relativePaths);
            }
        }
        closedir(dir);
    }
#endif
}

void getFilesInDirectory(const std::string& directory, StringVec& files, const std::string& extension)
{
#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile((directory + "/*." + extension).c_str(), &fd);
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
            if (entry->d_type != DT_DIR && getFileExtension(entry->d_name) == "mtlx")
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

namespace
{
    const float EPS_ZERO = 0.00001f;
    const float EPS_ONE  = 1.0f - EPS_ZERO;

    inline bool isZero(float v)
    {
        return v < EPS_ZERO;
    }

    inline bool isOne(float v)
    {
        return v > EPS_ONE;
    }

    inline bool isBlack(const Color3& c)
    {
        return isZero(c[0]) && isZero(c[1]) && isZero(c[0]);
    }

    inline bool isWhite(const Color3& c)
    {
        return isOne(c[0]) && isOne(c[1]) && isOne(c[0]);
    }

    bool isTransparentShaderGraph(OutputPtr output)
    {
        // Track how many nodes has the potential of being transparent
        // and how many of these we can say for sure are 100% opaque.
        size_t numCandidates = 0;
        size_t numOpaque = 0;

        for (GraphIterator it = output->traverseGraph().begin(); it != GraphIterator::end(); ++it)
        {
            ElementPtr upstreamElem = it.getUpstreamElement();

            const string& typeName = upstreamElem->asA<TypedElement>()->getType();
            const TypeDesc* type = TypeDesc::get(typeName);
            if (type != Type::SURFACESHADER && type != Type::BSDF)
            {
                it.setPruneSubgraph(true);
                continue;
            }

            if (upstreamElem->isA<Node>())
            {
                NodePtr node = upstreamElem->asA<Node>();

                const string& nodetype = node->getCategory();
                if (nodetype == "surface")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    InputPtr opacity = node->getInput("opacity");
                    if (!opacity)
                    {
                        opaque = true;
                    }
                    else if (opacity->getNodeName() == EMPTY_STRING && opacity->getInterfaceName() == EMPTY_STRING)
                    {
                        ValuePtr value = opacity->getValue();
                        if (!value || isOne(value->asA<float>()))
                        {
                            opaque = true;
                        }
                    }

                    if (opaque)
                    {
                        ++numOpaque;
                    }
                }
                else if (nodetype == "dielectricbtdf")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    // First check the weight
                    InputPtr weight = node->getInput("weight");
                    if (weight && weight->getNodeName() == EMPTY_STRING && weight->getInterfaceName() == EMPTY_STRING)
                    {
                        // Unconnected, check the value
                        ValuePtr value = weight->getValue();
                        if (value && isZero(value->asA<float>()))
                        {
                            opaque = true;
                        }
                    }

                    if (!opaque)
                    {
                        // Second check the tint
                        InputPtr tint = node->getInput("tint");
                        if (tint && tint->getNodeName() == EMPTY_STRING && tint->getInterfaceName() == EMPTY_STRING)
                        {
                            // Unconnected, check the value
                            ValuePtr value = tint->getValue();
                            if (value && isBlack(value->asA<Color3>()))
                            {
                                opaque = true;
                            }
                        }
                    }

                    if (opaque)
                    {
                        ++numOpaque;
                    }
                }
                else if (nodetype == "standard_surface")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    // First check the transmission weight
                    InputPtr transmission = node->getInput("transmission");
                    if (!transmission)
                    {
                        opaque = true;
                    }
                    else if (transmission->getNodeName() == EMPTY_STRING && transmission->getInterfaceName() == EMPTY_STRING)
                    {
                        // Unconnected, check the value
                        ValuePtr value = transmission->getValue();
                        if (!value || isZero(value->asA<float>()))
                        {
                            opaque = true;
                        }
                    }

                    // Second check the opacity
                    if (opaque)
                    {
                        InputPtr opacity = node->getInput("opacity");
                        if (!opacity)
                        {
                            opaque = true;
                        }
                        else if (opacity->getNodeName() == EMPTY_STRING && opacity->getInterfaceName() == EMPTY_STRING)
                        {
                            // Unconnected, check the value
                            ValuePtr value = opacity->getValue();
                            if (!value || isWhite(value->asA<Color3>()))
                            {
                                opaque = true;
                            }
                        }
                    }

                    if (opaque)
                    {
                        // We know for sure this is opaque
                        ++numOpaque;
                    }
                }

                if (numOpaque != numCandidates)
                {
                    // We found at least one candidate that we can't 
                    // say for sure is opaque. So we might need transparency.
                    return true;
                }
            }
        }

        return numCandidates > 0 ? numOpaque != numCandidates : false;
    }
}

bool isTransparentSurface(ElementPtr element, const ShaderGenerator& shadergen)
{
    if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();
        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shaderref '" + shaderRef->getName() + "'");
        }
        if (TypeDesc::get(nodeDef->getType()) != Type::SURFACESHADER)
        {
            return false;
        }

        const string& nodetype = nodeDef->getNodeString();
        if (nodetype == "standard_surface")
        {
            bool opaque = false;

            // First check the transmission weight
            BindInputPtr transmission = shaderRef->getBindInput("transmission");
            if (!transmission)
            {
                opaque = true;
            }
            else if (transmission->getOutputString() == EMPTY_STRING)
            {
                // Unconnected, check the value
                ValuePtr value = transmission->getValue();
                if (!value || isZero(value->asA<float>()))
                {
                    opaque = true;
                }
            }

            // Second check the opacity
            if (opaque)
            {
                BindInputPtr opacity = shaderRef->getBindInput("opacity");
                if (!opacity)
                {
                    opaque = true;
                }
                else if (opacity->getOutputString() == EMPTY_STRING)
                {
                    // Unconnected, check the value
                    ValuePtr value = opacity->getValue();
                    if (!value || isWhite(value->asA<Color3>()))
                    {
                        opaque = true;
                    }
                }
            }

            return !opaque;
        }
        else
        {
            InterfaceElementPtr impl = nodeDef->getImplementation(shadergen.getTarget(), shadergen.getLanguage());
            if (!impl)
            {
                throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef->getNodeString() +
                    "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
            }

            if (impl->isA<NodeGraph>())
            {
                NodeGraphPtr graph = impl->asA<NodeGraph>();

                vector<OutputPtr> outputs = graph->getActiveOutputs();
                if (outputs.size() > 0)
                {
                    const OutputPtr& output = outputs[0];
                    if (TypeDesc::get(output->getType()) == Type::SURFACESHADER)
                    {
                        return isTransparentShaderGraph(output);
                    }
                }
            }
        }
    }
    else if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        return isTransparentShaderGraph(output);
    }

    return false;
}

} // namespace MaterialX
