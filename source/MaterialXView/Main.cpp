#include <MaterialXView/Viewer.h>

#include <iostream>

NANOGUI_FORCE_DISCRETE_GPU();

const std::string options = 
" Options: \n"
"    --material [FILENAME]    Specify the displayed material\n"
"    --mesh [FILENAME]        Specify the displayed geometry\n"
"    --library [FILEPATH]     Specify an additional library folder\n"
"    --path [FILEPATH]        Specify an additional search-path folder\n"
"    --envMethod [INTEGER]    Environment lighting method (0 = filtered importance sampling, 1 = prefiltered environment maps, Default is 0)\n"
"    --envRad [FILENAME]      Specify the environment radiance HDR\n"
"    --msaa [INTEGER]         Multisampling count for anti-aliasing (0 = disabled, Default is 0)\n"
"    --refresh [INTEGER]      Refresh period for the viewer in milliseconds (-1 = disabled, Default is 50)\n"
"    --remap [TOKEN1:TOKEN2]  Remap one token to another when MaterialX document is loaded\n"
"    --skip [NAME]            Skip elements matching the given name attribute\n"
"    --terminator [STRING]    Enforce the given terminator string for file prefixes\n"
"    --help                   Print this list\n";

int main(int argc, char* const argv[])
{  
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.push_back(std::string(argv[i]));
    }

    mx::FilePathVec libraryFolders = { "libraries/stdlib", "libraries/pbrlib", "libraries/stdlib/genglsl", "libraries/pbrlib/genglsl", 
                                       "libraries/bxdf", "libraries/lights", "libraries/lights/genglsl" };
    mx::FileSearchPath searchPath;
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    std::string meshFilename = "resources/Geometry/shaderball.obj";
    std::string envRadiancePath = "resources/Lights/san_giuseppe_bridge_split.hdr";
    DocumentModifiers modifiers;
    int multiSampleCount = 0;
    int refresh = 50;
    mx::HwSpecularEnvironmentMethod specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;
        if (token == "--material" && !nextToken.empty())
        {
            materialFilename = nextToken;
        }
        if (token == "--mesh" && !nextToken.empty())
        {
            meshFilename = nextToken;
        }
        if (token == "--library" && !nextToken.empty())
        {
            libraryFolders.push_back(nextToken);
        }
        if (token == "--path" && !nextToken.empty())
        {
            searchPath.append(mx::FileSearchPath(nextToken));
        }
        if (token == "--envMethod" && !nextToken.empty())
        {
            if (std::stoi(nextToken) == 1)
            {
                specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_PREFILTER;
            }
        }
        if (token == "--envRad" && !nextToken.empty())
        {
            envRadiancePath = nextToken;
        }
        if (token == "--msaa" && !nextToken.empty())
        {
            multiSampleCount = std::stoi(nextToken);
        }
        if (token == "--refresh" && !nextToken.empty())
        {
            refresh = std::stoi(nextToken);
        }
        if (token == "--remap" && !nextToken.empty())
        {
            mx::StringVec vec = mx::splitString(nextToken, ":");
            if (vec.size() == 2)
            {
                modifiers.remapElements[vec[0]] = vec[1];
            }
        }
        if (token == "--skip" && !nextToken.empty())
        {
            modifiers.skipElements.insert(nextToken);
        }
        if (token == "--terminator" && !nextToken.empty())
        {
            modifiers.filePrefixTerminator = nextToken;
        }
        if (token == "--help")
        {
            std::cout << options << std::endl;
            return 0;
        }
    }

    // Search current directory and parent directory if not found.
    mx::FilePath currentPath(mx::FilePath::getCurrentPath());
    mx::FilePath parentCurrentPath = currentPath.getParentPath();
    std::vector<mx::FilePath> libraryPaths =
    { 
        mx::FilePath("libraries")
    };
    for (const auto& libraryPath : libraryPaths)
    {
        mx::FilePath fullPath(currentPath / libraryPath);
        if (!fullPath.exists())
        {
            fullPath = parentCurrentPath / libraryPath;
            if (fullPath.exists())
            {
                searchPath.append(fullPath);
            }
        }
        else
        {
            searchPath.append(fullPath);
        }
    }
    searchPath.append(parentCurrentPath);
    searchPath.prepend(currentPath);

    try
    {
        ng::init();
        {
            ng::ref<Viewer> viewer = new Viewer(materialFilename,
                                                meshFilename,
                                                libraryFolders,
                                                searchPath,
                                                modifiers,
                                                specularEnvironmentMethod,
                                                envRadiancePath,
                                                multiSampleCount);
            viewer->setVisible(true);
            ng::mainloop(refresh);
        }
    
        ng::shutdown();
    }
    catch (const std::runtime_error& e)
    {
        std::string error_msg = std::string("Fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return -1;
    }

    return 0;
}
