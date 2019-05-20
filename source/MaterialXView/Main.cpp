#include <MaterialXView/Viewer.h>

#include <iostream>

NANOGUI_FORCE_DISCRETE_GPU();

const std::string docstring = 
" Options: \n"
"    --library [PATH]         Additional library folder location\n"
"    --path [PATH]            Additional file search path location\n"
"    --mesh [PATH]            Mesh filename (Default: resources/Geometry/shaderball.obj)\n"
"    --material [PATH]        Material filename\n"
"    --remap [TOKEN1:TOKEN2]  Remap one token to another when MaterialX document is loaded\n"
"    --skip [STRING ...]      Skip elements with the given name attribute\n"
"    --terminator [STRING]    Enforce the given terminator string for file prefixes\n"
"    --envMethod [INTEGER]    Environment lighting method. 0 = filtered importance sampling (default); 1 = prefiltered environment maps.\n"
"    --envRad [PATH]          Specify the environment radiance HDR (Default: resources/Images/san_giuseppe_bridge.hdr)\n"
"    --envIrrad [PATH]        Specify the environment irradiance HDR (Default: resources/Images/san_giuseppe_bridge_diffuse.hdr)\n"
"    --msaa [INTEGER]         Multisampling count for anti-aliasing (Default: 0)\n"
"    -h, --help               Print this help\n";

int main(int argc, char* const argv[])
{  
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.push_back(std::string(argv[i]));
    }

    mx::StringVec libraryFolders = { "libraries/stdlib", "libraries/pbrlib", "libraries/stdlib/genglsl", "libraries/pbrlib/genglsl", "libraries/bxdf" };
    mx::FileSearchPath searchPath;
    std::string meshFilename = "resources/Geometry/shaderball.obj";
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    std::string envRadiancePath = "resources/Images/san_giuseppe_bridge.hdr";
    std::string envIrradiancePath = "resources/Images/san_giuseppe_bridge_diffuse.hdr";
    DocumentModifiers modifiers;
    int multiSampleCount = 0;
    mx::HwSpecularEnvironmentMethod specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;
        if (token == "--library" && !nextToken.empty())
        {
            libraryFolders.push_back(nextToken);
        }
        if (token == "--path" && !nextToken.empty())
        {
            searchPath = mx::FileSearchPath(nextToken);
        }
        if (token == "--mesh" && !nextToken.empty())
        {
            meshFilename = nextToken;
        }
        if (token == "--material" && !nextToken.empty())
        {
            materialFilename = nextToken;
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
        if (token == "--envIrrad" && !nextToken.empty())
        {
            envIrradiancePath = nextToken;
        }
        if (token == "--msaa" && !nextToken.empty())
        {
            multiSampleCount = std::stoi(nextToken);
        }
        if (token == "--help" || token == "-h")
        {
            std::cout << docstring << std::endl;
            return 0;
        }
    }

    // Search current directory and parent directory if not found.
    mx::FilePath currentPath(mx::FilePath::getCurrentPath());
    mx::FilePath parentCurrentPath(currentPath);
    parentCurrentPath.pop();
    std::vector<mx::FilePath> libraryPaths =
    { 
        mx::FilePath("libraries")
    };
    for (auto libraryPath : libraryPaths)
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

    try
    {
        ng::init();
        {
            ng::ref<Viewer> viewer = new Viewer(libraryFolders,
                                                searchPath,
                                                meshFilename,
                                                materialFilename,
                                                modifiers,
                                                specularEnvironmentMethod,
                                                envRadiancePath,
                                                envIrradiancePath,
                                                multiSampleCount);
            viewer->setVisible(true);
            ng::mainloop();
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
