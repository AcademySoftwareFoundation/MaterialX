#include <MaterialXView/Viewer.h>

#include <iostream>

NANOGUI_FORCE_DISCRETE_GPU();

const std::string options = 
" Options: \n"
"    --material [FILENAME]          Specify the displayed material\n"
"    --mesh [FILENAME]              Specify the displayed geometry\n"
"    --meshRotation [VECTOR3]       Specify the rotation of the displayed geometry as three comma-separated floats (e.g. '0,90,0'), representing rotations in degrees about the X, Y, and Z axes.\n"
"    --envRad [FILENAME]            Specify the displayed environment light, stored as HDR environment radiance in the latitude-longitude format\n"
"    --envMethod [INTEGER]          Specify the environment lighting method (0 = filtered importance sampling, 1 = prefiltered environment maps, Default is 0)\n"
"    --lightRotation [FLOAT]        Specify the rotation in degrees of the lighting environment about the Y axis.\n"
"    --path [FILEPATH]              Specify an additional absolute search path location (e.g. '/projects/MaterialX').  This path will be queried when locating standard data libraries, XInclude references, and referenced images.\n"
"    --library [FILEPATH]           Specify an additional relative path to a custom data library folder (e.g. 'libraries/custom').  MaterialX files at the root of this folder will be included in all content documents.\n"
"    --msaa [INTEGER]               Multisampling count for anti-aliasing (0 = disabled, Default is 0)\n"
"    --refresh [INTEGER]            Refresh period for the viewer in milliseconds (-1 = disabled, Default is 50)\n"
"    --remap [TOKEN1:TOKEN2]        Remap one token to another when MaterialX document is loaded\n"
"    --skip [NAME]                  Skip elements matching the given name attribute\n"
"    --terminator [STRING]          Enforce the given terminator string for file prefixes\n"
"    --help                         Print this list\n";

int main(int argc, char* const argv[])
{  
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    mx::FilePathVec libraryFolders = { "libraries/stdlib", "libraries/pbrlib", "libraries/stdlib/genglsl", "libraries/pbrlib/genglsl", 
                                       "libraries/bxdf", "libraries/lights", "libraries/lights/genglsl" };
    mx::FileSearchPath searchPath;
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    std::string meshFilename = "resources/Geometry/shaderball.obj";
    mx::Vector3 meshRotation;
    std::string envRadiancePath = "resources/Lights/san_giuseppe_bridge_split.hdr";
    float lightRotation = 0.0f;
    DocumentModifiers modifiers;
    int multiSampleCount = 0;
    int refresh = 50;
    mx::HwSpecularEnvironmentMethod specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;
        if (token == "--material")
        {
            materialFilename = nextToken;
        }
        else if (token == "--mesh")
        {
            meshFilename = nextToken;
        }
        else if (token == "--meshRotation")
        {
            mx::ValuePtr value = mx::Value::createValueFromStrings(nextToken, "vector3");
            if (value)
            {
                meshRotation = value->asA<mx::Vector3>();
            }
            else if (!nextToken.empty())
            {
                std::cout << "Unable to parse token following command-line option: " << token << std::endl;
            }
        }
        else if (token == "--envRad")
        {
            envRadiancePath = nextToken;
        }
        else if (token == "--envMethod")
        {
            if (std::stoi(nextToken) == 1)
            {
                specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_PREFILTER;
            }
        }
        else if (token == "--lightRotation")
        {
            mx::ValuePtr value = mx::Value::createValueFromStrings(nextToken, "float");
            if (value)
            {
                lightRotation = value->asA<float>();
            }
            else if (!nextToken.empty())
            {
                std::cout << "Unable to parse token following command-line option: " << token << std::endl;
            }
        }
        else if (token == "--path")
        {
            searchPath.append(mx::FileSearchPath(nextToken));
        }
        else if (token == "--library")
        {
            libraryFolders.push_back(nextToken);
        }
        else if (token == "--msaa")
        {
            multiSampleCount = std::stoi(nextToken);
        }
        else if (token == "--refresh")
        {
            refresh = std::stoi(nextToken);
        }
        else if (token == "--remap")
        {
            mx::StringVec vec = mx::splitString(nextToken, ":");
            if (vec.size() == 2)
            {
                modifiers.remapElements[vec[0]] = vec[1];
            }
            else if (!nextToken.empty())
            {
                std::cout << "Unable to parse token following command-line option: " << token << std::endl;
            }
        }
        else if (token == "--skip")
        {
            modifiers.skipElements.insert(nextToken);
        }
        else if (token == "--terminator")
        {
            modifiers.filePrefixTerminator = nextToken;
        }
        else if (token == "--help")
        {
            std::cout << options << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Unrecognized command-line option: " << token << std::endl;
            std::cout << "Launch the viewer with '--help' for a complete list of supported options." << std::endl;
        }

        if (nextToken.empty())
        {
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        }
        else
        {
            i++;
        }
    }

    // Add default search paths for the viewer.
    mx::FilePath installSearchPath = mx::FilePath::getModulePath().getParentPath();
    mx::FilePath devSearchPath = installSearchPath.getParentPath().getParentPath().getParentPath();
    searchPath.append(installSearchPath);
    if (!devSearchPath.isEmpty() && (devSearchPath / "libraries").exists())
    {
        searchPath.append(devSearchPath);
    }

    try
    {
        ng::init();
        {
            ng::ref<Viewer> viewer = new Viewer(materialFilename,
                                                meshFilename,
                                                meshRotation,
                                                libraryFolders,
                                                searchPath,
                                                modifiers,
                                                specularEnvironmentMethod,
                                                envRadiancePath,
                                                lightRotation,
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
