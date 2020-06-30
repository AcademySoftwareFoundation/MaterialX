#include <MaterialXView/Viewer.h>

#include <iostream>

NANOGUI_FORCE_DISCRETE_GPU();

const std::string options = 
" Options: \n"
"    --material [FILENAME]          The filename of the MTLX document to be displayed in the viewer\n"
"    --mesh [FILENAME]              The filename of the OBJ mesh to be displayed in the viewer\n"
"    --meshRotation [VECTOR3]       The rotation of the displayed mesh as three comma-separated floats, representing rotations in degrees about the X, Y, and Z axes (defaults to 0,0,0)\n"
"    --meshScale [FLOAT]            The uniform scale of the displayed mesh\n"
"    --cameraPosition [VECTOR3]     The position of the camera as three comma-separated floats (defaults to 0,0,5)\n"
"    --cameraTarget [VECTOR3]       The position of the camera target as three comma-separated floats (defaults to 0,0,0)\n"
"    --cameraViewAngle [FLOAT]      The view angle of the camera (defaults to 45)\n"
"    --envRad [FILENAME]            The filename of the environment light to display, stored as HDR environment radiance in the latitude-longitude format\n"
"    --envMethod [INTEGER]          The environment lighting method (0 = filtered importance sampling, 1 = prefiltered environment maps, defaults to 0)\n"
"    --lightRotation [FLOAT]        The rotation in degrees of the lighting environment about the Y axis (defaults to 0)\n"
"    --path [FILEPATH]              An additional absolute search path location (e.g. '/projects/MaterialX').  This path will be queried when locating standard data libraries, XInclude references, and referenced images.\n"
"    --library [FILEPATH]           An additional relative path to a custom data library folder (e.g. 'libraries/custom').  MaterialX files at the root of this folder will be included in all content documents.\n"
"    --screenWidth [INTEGER]        The width of the screen image in pixels (defaults to 1280)\n"
"    --screenHeight [INTEGER]       The height of the screen image in pixels (defaults to 960)\n"
"    --screenColor [VECTOR3]        The background color of the viewer as three comma-separated floats (defaults to 0.3,0.3,0.32)\n"
"    --msaa [INTEGER]               The multisampling count for screen anti-aliasing (defaults to 0)\n"
"    --refresh [INTEGER]            The refresh period for the viewer in milliseconds (defaults to 50, set to -1 to disable)\n"
"    --remap [TOKEN1:TOKEN2]        Remap one token to another when MaterialX document is loaded\n"
"    --skip [NAME]                  Skip elements matching the given name attribute\n"
"    --terminator [STRING]          Enforce the given terminator string for file prefixes\n"
"    --help                         Print this list\n";

template<class T> void parseToken(std::string token, std::string type, T& res)
{
    if (token.empty())
    {
        return;
    }

    mx::ValuePtr value = mx::Value::createValueFromStrings(token, type);
    if (!value)
    {
        std::cout << "Unable to parse token " << token << " as type " << type << std::endl;
        return;
    }

    res = value->asA<T>();
}

int main(int argc, char* const argv[])
{  
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    mx::FilePathVec libraryFolders = 
    {
        "libraries",
    };

    mx::FileSearchPath searchPath;
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    std::string meshFilename = "resources/Geometry/shaderball.obj";
    mx::Vector3 meshRotation;
    float meshScale = 1.0f;
    mx::Vector3 cameraPosition(DEFAULT_CAMERA_POSITION);
    mx::Vector3 cameraTarget;
    float cameraViewAngle(DEFAULT_CAMERA_VIEW_ANGLE);
    std::string envRadiancePath = "resources/Lights/san_giuseppe_bridge_split.hdr";
    mx::HwSpecularEnvironmentMethod specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;
    float lightRotation = 0.0f;
    DocumentModifiers modifiers;
    int screenWidth = 1280;
    int screenHeight = 960;
    mx::Color3 screenColor(0.3f, 0.3f, 0.32f);
    int multiSampleCount = 0;
    int refresh = 50;

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
            parseToken(nextToken, "vector3", meshRotation);
        }
        else if (token == "--meshScale")
        {
            parseToken(nextToken, "float", meshScale);
        }
        else if (token == "--cameraPosition")
        {
            parseToken(nextToken, "vector3", cameraPosition);
        }
        else if (token == "--cameraTarget")
        {
            parseToken(nextToken, "vector3", cameraTarget);
        }
        else if (token == "--cameraViewAngle")
        {
            parseToken(nextToken, "float", cameraViewAngle);
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
            parseToken(nextToken, "float", lightRotation);
        }
        else if (token == "--path")
        {
            searchPath.append(mx::FileSearchPath(nextToken));
        }
        else if (token == "--library")
        {
            libraryFolders.push_back(nextToken);
        }
        else if (token == "--screenWidth")
        {
            parseToken(nextToken, "integer", screenWidth);
        }
        else if (token == "--screenHeight")
        {
            parseToken(nextToken, "integer", screenHeight);
        }
        else if (token == "--screenColor")
        {
            parseToken(nextToken, "color3", screenColor);
        }
        else if (token == "--msaa")
        {
            parseToken(nextToken, "integer", multiSampleCount);
        }
        else if (token == "--refresh")
        {
            parseToken(nextToken, "integer", refresh);
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
            continue;
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
    mx::FilePath devSearchPath = mx::FilePath(__FILE__).getParentPath().getParentPath().getParentPath();
    searchPath.append(installSearchPath);
    if (!devSearchPath.isEmpty() && devSearchPath.exists())
    {
        searchPath.append(devSearchPath);
        devSearchPath = devSearchPath / "libraries";
        if (devSearchPath.exists())
        {
            searchPath.append(devSearchPath);
        }
    }

    try
    {
        ng::init();
        {
            ng::ref<Viewer> viewer = new Viewer(materialFilename,
                                                meshFilename,
                                                meshRotation,
                                                meshScale,
                                                cameraPosition,
                                                cameraTarget,
                                                cameraViewAngle,
                                                envRadiancePath,
                                                specularEnvironmentMethod,
                                                lightRotation,
                                                libraryFolders,
                                                searchPath,
                                                modifiers,
                                                screenWidth,
                                                screenHeight,
                                                screenColor,
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
