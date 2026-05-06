//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXRender/CgltfLoader.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/Mesh.h>
#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif
#include <MaterialXRenderOsl/OslRenderer.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace mx = MaterialX;
namespace fs = std::filesystem;

namespace
{

// OSL testrender maps fov over [-0.5, 0.5] screen coordinates.
// This value gives the same framing as a 45 degree MaterialXView camera.
const float TESTRENDER_FOV_FOR_45_DEGREES = 79.27854f;

const mx::StringMap& shaderNameSubstitutions()
{
    static const mx::StringMap substitutions =
    {
        { "/", "_" },
        { ":", "_" }
    };
    return substitutions;
}

//
// Define local overrides for the tangent frame in shader generation, aligning conventions
// between MaterialXRender and testrender.
//

class TangentOsl : public mx::ShaderNodeImpl
{
  public:
    static mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<TangentOsl>();
    }

    void emitFunctionCall(const mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

        DEFINE_SHADER_STAGE(stage, mx::Stage::PIXEL)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = normalize(vector(N[2], 0, -N[0]))", stage);
            shadergen.emitLineEnd(stage);
        }
    }
};

class BitangentOsl : public mx::ShaderNodeImpl
{
  public:
    static mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<BitangentOsl>();
    }

    void emitFunctionCall(const mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

        DEFINE_SHADER_STAGE(stage, mx::Stage::PIXEL)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = normalize(cross(N, vector(N[2], 0, -N[0])))", stage);
            shadergen.emitLineEnd(stage);
        }
    }
};

struct Options
{
    mx::FilePath material;
    mx::FilePath mesh;
    mx::FilePath envRadiance;
    mx::FilePath captureFilename;
    std::vector<mx::FilePath> paths;
    std::string screenColor = "0,0,0";
    unsigned int screenWidth = 512;
    unsigned int screenHeight = 512;
    bool drawEnvironment = true;
    bool shadows = true;
    bool help = false;
};

void printUsage()
{
    std::cout
        << "Usage: materialx-osl --material <file> --captureFilename <file> [options]\n\n"
        << "Options:\n"
        << "  --material <file>             MaterialX document to render\n"
        << "  --mesh <file>                 Accepted for MaterialXView CLI compatibility\n"
        << "  --envRad <file>               Environment radiance image\n"
        << "  --screenWidth <int>           Output width, default 512\n"
        << "  --screenHeight <int>          Output height, default 512\n"
        << "  --captureFilename <file>      Final PNG path\n"
        << "  --path <dir>                  Repeatable MaterialX search path\n"
        << "  --screenColor <r,g,b>         Accepted for MaterialXView CLI compatibility\n"
        << "  --drawEnvironment <bool>      Accepted for MaterialXView CLI compatibility\n"
        << "  --enableDirectLight <bool>    Accepted for MaterialXView CLI compatibility\n"
        << "  --shadowMap <bool>            Accepted for MaterialXView CLI compatibility\n"
        << "  --shadows <bool>              Enable or disable OSL shadow occlusion rays\n"
        << "  --help                        Show this help\n";
}

bool optionNeedsValue(const std::string& option)
{
    static const std::set<std::string> options =
    {
        "--material",
        "--mesh",
        "--envRad",
        "--screenWidth",
        "--screenHeight",
        "--captureFilename",
        "--path",
        "--screenColor",
        "--drawEnvironment",
        "--enableDirectLight",
        "--shadowMap",
        "--shadows"
    };
    return options.count(option) != 0;
}

Options parseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            options.help = true;
            continue;
        }
        if (!optionNeedsValue(arg))
        {
            throw std::runtime_error("Unknown option: " + arg);
        }
        if (i + 1 >= argc)
        {
            throw std::runtime_error("Missing value for option: " + arg);
        }

        const std::string value = argv[++i];
        if (arg == "--material")
        {
            options.material = value;
        }
        else if (arg == "--mesh")
        {
            options.mesh = value;
        }
        else if (arg == "--envRad")
        {
            options.envRadiance = value;
        }
        else if (arg == "--screenWidth")
        {
            options.screenWidth = static_cast<unsigned int>(std::stoul(value));
        }
        else if (arg == "--screenHeight")
        {
            options.screenHeight = static_cast<unsigned int>(std::stoul(value));
        }
        else if (arg == "--captureFilename")
        {
            options.captureFilename = value;
        }
        else if (arg == "--path")
        {
            options.paths.emplace_back(value);
        }
        else if (arg == "--screenColor")
        {
            options.screenColor = value;
        }
        else if (arg == "--drawEnvironment")
        {
            options.drawEnvironment = mx::stringToLower(value) != "false" && value != "0";
        }
        else if (arg == "--shadows")
        {
            const std::string lowerValue = mx::stringToLower(value);
            options.shadows = lowerValue != "false" && lowerValue != "0" && lowerValue != "off" && lowerValue != "no";
        }
    }
    return options;
}

fs::path executablePath(char** argv)
{
#if defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) == 0)
    {
        return fs::weakly_canonical(fs::path(buffer.c_str()));
    }
#elif defined(__linux__)
    std::vector<char> buffer(4096);
    const ssize_t length = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length > 0)
    {
        buffer[static_cast<size_t>(length)] = '\0';
        return fs::weakly_canonical(fs::path(buffer.data()));
    }
#endif
    return fs::weakly_canonical(fs::path(argv[0]));
}

mx::FilePath inferMaterialXRoot(char** argv)
{
    if (fs::exists(fs::path(MATERIALX_SOURCE_ROOT) / "libraries"))
    {
        return fs::weakly_canonical(fs::path(MATERIALX_SOURCE_ROOT)).string();
    }

    fs::path exe = executablePath(argv);
    for (fs::path current = exe.parent_path(); !current.empty(); current = current.parent_path())
    {
        if (fs::exists(current / "libraries") && fs::exists(current / "source" / "MaterialXCore"))
        {
            return current.string();
        }
        if (current == current.parent_path())
        {
            break;
        }
    }
    return mx::FilePath();
}

mx::FilePath findUtilitiesPath(const mx::FileSearchPath& searchPath, const mx::FilePath& materialXRoot)
{
    const fs::path runtimeUtilityPath = fs::path(MATERIALX_RUNTIME_OUTPUT_DIR) / "resources" / "Utilities";
    if (fs::exists(runtimeUtilityPath))
    {
        return fs::weakly_canonical(runtimeUtilityPath).string();
    }

    mx::FilePath utilityPath = searchPath.find("resources/Utilities");
    if (!utilityPath.isEmpty())
    {
        return fs::weakly_canonical(fs::absolute(fs::path(utilityPath.asString()))).string();
    }

    if (!materialXRoot.isEmpty())
    {
        const mx::FilePath buildUtilityPath = materialXRoot / "build-osl/bin/resources/Utilities";
        if (buildUtilityPath.exists())
        {
            return fs::weakly_canonical(fs::path(buildUtilityPath.asString())).string();
        }

        const mx::FilePath sourceUtilityPath = materialXRoot / "source/MaterialXTest/MaterialXRenderOsl/Utilities";
        if (sourceUtilityPath.exists())
        {
            return fs::weakly_canonical(fs::path(sourceUtilityPath.asString())).string();
        }
    }
    return mx::FilePath();
}

mx::FileSearchPath buildSearchPath(const Options& options, const mx::FilePath& materialXRoot)
{
    mx::FileSearchPath searchPath;
    for (const mx::FilePath& path : options.paths)
    {
        searchPath.append(path);
    }
    searchPath.append(mx::FilePath::getCurrentPath());
    if (!materialXRoot.isEmpty())
    {
        searchPath.append(materialXRoot);
    }
    return searchPath;
}

std::string shaderName(mx::TypedElementPtr element)
{
    return mx::createValidName(mx::replaceSubstrings(element->getNamePath(), shaderNameSubstitutions()));
}

std::string xmlEscape(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value)
    {
        switch (ch)
        {
        case '&':
            escaped += "&amp;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}

bool parseScreenColor(const std::string& value, mx::Color3& color)
{
    const mx::StringVec parts = mx::splitString(value, ",");
    if (parts.size() != 3)
    {
        return false;
    }

    try
    {
        color[0] = std::stof(parts[0]);
        color[1] = std::stof(parts[1]);
        color[2] = std::stof(parts[2]);
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

mx::Vector3 getPosition(mx::MeshStreamPtr stream, size_t index)
{
    const mx::MeshFloatBuffer& data = stream->getData();
    const size_t offset = index * stream->getStride();
    return mx::Vector3(data[offset], data[offset + 1], data[offset + 2]);
}

mx::Vector2 getTexCoord(mx::MeshStreamPtr stream, size_t index)
{
    const mx::MeshFloatBuffer& data = stream->getData();
    const size_t offset = index * stream->getStride();
    return mx::Vector2(data[offset], data[offset + 1]);
}

mx::Vector3 computeBounds(const mx::MeshList& meshes, mx::Vector3& minBounds, mx::Vector3& maxBounds)
{
    bool foundPositions = false;
    for (mx::MeshPtr mesh : meshes)
    {
        mx::MeshStreamPtr positions = mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        if (!positions)
        {
            continue;
        }

        for (size_t i = 0; i < positions->getSize(); ++i)
        {
            const mx::Vector3 position = getPosition(positions, i);
            if (!foundPositions)
            {
                minBounds = position;
                maxBounds = position;
                foundPositions = true;
            }
            else
            {
                minBounds[0] = std::min(minBounds[0], position[0]);
                minBounds[1] = std::min(minBounds[1], position[1]);
                minBounds[2] = std::min(minBounds[2], position[2]);
                maxBounds[0] = std::max(maxBounds[0], position[0]);
                maxBounds[1] = std::max(maxBounds[1], position[1]);
                maxBounds[2] = std::max(maxBounds[2], position[2]);
            }
        }
    }

    if (!foundPositions)
    {
        throw std::runtime_error("Mesh contains no position data");
    }
    return (minBounds + maxBounds) * 0.5f;
}

struct NormalizedMeshExport
{
    mx::FilePath objPath;
    mx::Vector3 objectCenter;
    float objectToCommonScale = 1.0f;
};

NormalizedMeshExport exportNormalizedObj(const mx::FilePath& meshPath, const mx::FilePath& outputDir,
                                         const std::string& generatedShaderName)
{
    mx::MeshList meshes;
    if (!mx::CgltfLoader::create()->load(meshPath, meshes, false) || meshes.empty())
    {
        throw std::runtime_error("Could not load GLTF mesh: " + meshPath.asString());
    }

    mx::Vector3 minBounds;
    mx::Vector3 maxBounds;
    const mx::Vector3 center = computeBounds(meshes, minBounds, maxBounds);
    const float radius = (center - minBounds).getMagnitude();
    if (radius <= 0.0f)
    {
        throw std::runtime_error("Mesh bounds are degenerate: " + meshPath.asString());
    }
    const float scale = 2.0f / radius;

    mx::FilePath objPath = outputDir / (generatedShaderName + "_mesh.obj");
    std::ofstream obj(objPath.asString());
    if (!obj)
    {
        throw std::runtime_error("Could not write temporary OBJ: " + objPath.asString());
    }

    obj << "# Generated by materialx-osl from " << meshPath.asString() << "\n";
    size_t vertexOffset = 1;
    size_t texcoordOffset = 1;
    size_t normalOffset = 1;

    for (mx::MeshPtr mesh : meshes)
    {
        mx::MeshStreamPtr positions = mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        if (!positions)
        {
            continue;
        }
        mx::MeshStreamPtr texcoords = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);
        mx::MeshStreamPtr normals = mesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0);

        obj << "o " << mx::createValidName(mesh->getName()) << "\n";
        for (size_t i = 0; i < positions->getSize(); ++i)
        {
            // testrender does not apply named transforms to model geometry, so write common-space vertices.
            const mx::Vector3 position = (getPosition(positions, i) - center) * scale;
            obj << "v " << position[0] << ' ' << position[1] << ' ' << position[2] << "\n";
        }
        if (texcoords && texcoords->getSize() >= positions->getSize())
        {
            for (size_t i = 0; i < positions->getSize(); ++i)
            {
                const mx::Vector2 uv = getTexCoord(texcoords, i);
                obj << "vt " << uv[0] << ' ' << uv[1] << "\n";
            }
        }
        else
        {
            texcoords = nullptr;
        }
        if (normals && normals->getSize() >= positions->getSize())
        {
            for (size_t i = 0; i < positions->getSize(); ++i)
            {
                const mx::Vector3 normal = getPosition(normals, i).getNormalized();
                obj << "vn " << normal[0] << ' ' << normal[1] << ' ' << normal[2] << "\n";
            }
        }
        else
        {
            normals = nullptr;
        }

        for (size_t partitionIndex = 0; partitionIndex < mesh->getPartitionCount(); ++partitionIndex)
        {
            const mx::MeshIndexBuffer& indices = mesh->getPartition(partitionIndex)->getIndices();
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
            {
                obj << "f";
                for (size_t corner = 0; corner < 3; ++corner)
                {
                    const size_t index = indices[i + corner];
                    const size_t vertexIndex = vertexOffset + index;
                    obj << ' ' << vertexIndex;
                    if (texcoords || normals)
                    {
                        obj << '/';
                        if (texcoords)
                        {
                            obj << texcoordOffset + index;
                        }
                        if (normals)
                        {
                            obj << '/' << normalOffset + index;
                        }
                    }
                }
                obj << "\n";
            }
        }

        vertexOffset += positions->getSize();
        if (texcoords)
        {
            texcoordOffset += positions->getSize();
        }
        if (normals)
        {
            normalOffset += positions->getSize();
        }
    }

    return { objPath, center, scale };
}

mx::FilePath writeSceneTemplate(const Options& options, const mx::FilePath& outputDir,
                                const std::string& generatedShaderName,
                                const NormalizedMeshExport& meshExport)
{
    mx::Color3 screenColor(0.0f);
    parseScreenColor(options.screenColor, screenColor);

    mx::FilePath sceneTemplatePath = outputDir / (generatedShaderName + "_scene_template_input.xml");
    std::ofstream scene(sceneTemplatePath.asString());
    if (!scene)
    {
        throw std::runtime_error("Could not write OSL scene template: " + sceneTemplatePath.asString());
    }

    scene << "<World>\n";
    const mx::Vector3 objectTranslation = meshExport.objectCenter * -meshExport.objectToCommonScale;
    scene << "   <Transform name=\"object\" matrix=\""
          << meshExport.objectToCommonScale << ", 0, 0, 0, "
          << "0, " << meshExport.objectToCommonScale << ", 0, 0, "
          << "0, 0, " << meshExport.objectToCommonScale << ", 0, "
          << objectTranslation[0] << ", " << objectTranslation[1] << ", " << objectTranslation[2] << ", 1\" />\n";
    scene << "   <Transform name=\"model\" matrix=\""
          << meshExport.objectToCommonScale << ", 0, 0, 0, "
          << "0, " << meshExport.objectToCommonScale << ", 0, 0, "
          << "0, 0, " << meshExport.objectToCommonScale << ", 0, "
          << objectTranslation[0] << ", " << objectTranslation[1] << ", " << objectTranslation[2] << ", 1\" />\n";
    scene << "   <Camera eye=\"0, 0, 5\" look_at=\"0, 0, 0\" up=\"0, 1, 0\" fov=\"" << TESTRENDER_FOV_FOR_45_DEGREES << "\" />\n\n";
    if (options.drawEnvironment)
    {
        scene << "   <ShaderGroup>\n";
        scene << "      %environment_shader_parameter_overrides%\n";
        scene << "      shader envmap layer1;\n";
        scene << "   </ShaderGroup>\n";
        scene << "   <Background resolution=\"2048\" />\n\n";
    }
    else
    {
        scene << "   <ShaderGroup>\n";
        scene << "      color Cin color(" << screenColor[0] << ", " << screenColor[1] << ", " << screenColor[2] << ");\n";
        scene << "      shader constant_color layer1;\n";
        scene << "   </ShaderGroup>\n";
        scene << "   <Background resolution=\"32\" />\n\n";
    }

    scene << "   <ShaderGroup>\n";
    scene << "      %input_shader_parameter_overrides%;\n";
    scene << "      shader %input_shader_type% inputShader;\n";
    scene << "      shader %output_shader_type% outputShader;\n";
    scene << "      connect inputShader.%input_shader_output% outputShader.%output_shader_input%;\n";
    scene << "   </ShaderGroup>\n\n";
    scene << "   <Model filename=\"" << xmlEscape(meshExport.objPath.getBaseName()) << "\" />\n";
    scene << "</World>\n";

    return sceneTemplatePath;
}

void initializeRenderer(mx::OslRendererPtr renderer, mx::ImageHandlerPtr imageHandler,
                        const mx::FilePath& utilityPath)
{
    renderer->setOslCompilerExecutable(MATERIALX_OSL_BINARY_OSLC);
    renderer->setOslTestRenderExecutable(MATERIALX_OSL_BINARY_TESTRENDER);
    if (std::string(MATERIALX_OSL_INCLUDE_PATH).length())
    {
        renderer->setOslIncludePath(mx::FileSearchPath(mx::FilePath(MATERIALX_OSL_INCLUDE_PATH)));
    }
    renderer->initialize();
    renderer->setImageHandler(imageHandler);
    renderer->setLightHandler(nullptr);

    if (utilityPath.isEmpty())
    {
        throw std::runtime_error("Could not find OSL utilities directory");
    }

    renderer->setOslOutputFilePath(utilityPath);
    for (const mx::FilePath& filename : utilityPath.getFilesInDirectory("osl"))
    {
        renderer->compileOSL(utilityPath / filename);
    }
    renderer->setOslUtilityOSOPath(utilityPath);
}

mx::ImageHandlerPtr createImageHandler(const mx::FileSearchPath& searchPath)
{
    mx::ImageHandlerPtr imageHandler = mx::ImageHandler::create(mx::StbImageLoader::create());
    imageHandler->setSearchPath(searchPath);
#if defined(MATERIALX_BUILD_OIIO)
    imageHandler->addLoader(mx::OiioImageLoader::create());
#endif
    return imageHandler;
}

void render(const Options& options, char** argv)
{
    if (options.material.isEmpty())
    {
        throw std::runtime_error("--material is required");
    }
    if (options.captureFilename.isEmpty())
    {
        throw std::runtime_error("--captureFilename is required");
    }
    if (std::string(MATERIALX_OSL_BINARY_OSLC).empty() || std::string(MATERIALX_OSL_BINARY_TESTRENDER).empty())
    {
        throw std::runtime_error("OSL compiler or testrender executable was not configured");
    }

    const mx::FilePath materialXRoot = inferMaterialXRoot(argv);
    mx::FileSearchPath searchPath = buildSearchPath(options, materialXRoot);
    const mx::FilePath materialPath = searchPath.find(options.material);
    if (materialPath.isEmpty())
    {
        throw std::runtime_error("Could not find material file: " + options.material.asString());
    }

    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, materialPath, searchPath);
    doc->importLibrary(libraries);

    mx::FileSearchPath imageSearchPath(materialPath.getParentPath());
    imageSearchPath.append(searchPath);
    mx::StringResolverPtr resolver = mx::StringResolver::create();
    resolver->setFilenameSubstitution("\\\\", "/");
    resolver->setFilenameSubstitution("\\", "/");
    mx::flattenFilenames(doc, imageSearchPath, resolver);

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    if (elements.empty())
    {
        throw std::runtime_error("No renderable elements found in: " + materialPath.asString());
    }

    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::ColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create(generator->getTarget());
    colorManagementSystem->loadLibrary(libraries);
    generator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    const mx::FilePath genOslIncludePath = searchPath.find("libraries/stdlib/genosl/include");
    if (!genOslIncludePath.isEmpty())
    {
        context.registerSourceCodeSearchPath(genOslIncludePath);
    }
    context.getOptions().targetColorSpaceOverride = "lin_rec709";
    context.getOptions().fileTextureVerticalFlip = true;
    context.getOptions().oslConnectCiWrapper = true;
    generator->registerShaderMetadata(libraries, context);

    generator->registerImplementation("IM_tangent_vector3_" + mx::OslShaderGenerator::TARGET, TangentOsl::create);
    generator->registerImplementation("IM_bitangent_vector3_" + mx::OslShaderGenerator::TARGET, BitangentOsl::create);

    mx::TypedElementPtr element = elements.front();
    const std::string generatedShaderName = shaderName(element);
    mx::ShaderPtr shader = generator->generate(generatedShaderName, element, context);

    const mx::FilePath utilityPath = findUtilitiesPath(searchPath, materialXRoot);
    mx::ImageHandlerPtr imageHandler = createImageHandler(imageSearchPath);
    mx::OslRendererPtr renderer = mx::OslRenderer::create(options.screenWidth, options.screenHeight);
    initializeRenderer(renderer, imageHandler, utilityPath);

    mx::FilePath outputDir = options.captureFilename.getParentPath() / ".materialx-osl";
    outputDir.createDirectory(true);

    mx::FilePath sceneTemplatePath = utilityPath / "scene_template.xml";
    if (!options.mesh.isEmpty())
    {
        mx::FilePath meshPath = searchPath.find(options.mesh);
        if (meshPath.isEmpty())
        {
            meshPath = imageSearchPath.find(options.mesh);
        }
        if (meshPath.isEmpty())
        {
            throw std::runtime_error("Could not find mesh file: " + options.mesh.asString());
        }

        const NormalizedMeshExport meshExport = exportNormalizedObj(meshPath, outputDir, generatedShaderName);
        sceneTemplatePath = writeSceneTemplate(options, outputDir, generatedShaderName, meshExport);
    }

    renderer->setOslOutputFilePath(outputDir);
    renderer->setOslShaderName(generatedShaderName);
    renderer->setRaysPerPixelLit(4);
    renderer->setRaysPerPixelUnlit(1);
    renderer->setOslShadows(options.shadows);
    renderer->setSize(options.screenWidth, options.screenHeight);

    mx::StringVec envOverrides;
    if (!options.envRadiance.isEmpty())
    {
        mx::FilePath envRadiancePath = imageSearchPath.find(options.envRadiance);
        if (envRadiancePath.isEmpty())
        {
            envRadiancePath = options.envRadiance;
        }
        envOverrides.push_back("string envmap_filename \"" + envRadiancePath.asString() + "\";\n");
    }
    renderer->setEnvShaderParameterOverrides(envOverrides);

    renderer->createProgram(shader);

    const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);
    const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
    if (outputs.size() == 0)
    {
        throw std::runtime_error("Generated OSL shader has no renderable outputs");
    }
    const mx::ShaderPort* output = outputs[0];
    const mx::TypeSyntax& typeSyntax = generator->getSyntax().getTypeSyntax(output->getType());
    const std::string outputType = typeSyntax.getTypeAlias().empty() ? typeSyntax.getName() : typeSyntax.getTypeAlias();
    renderer->setOslShaderOutput(output->getVariable(), outputType);

    renderer->setOslTestRenderSceneTemplateFile(sceneTemplatePath.asString());
    renderer->useOslCommandString(false);
    renderer->render();
    mx::ImagePtr capturedImage = renderer->captureImage();

    const mx::FilePath producedImage = outputDir / (generatedShaderName + "_osl.png");
    const fs::path finalPath(options.captureFilename.asString());
    if (finalPath.has_parent_path())
    {
        fs::create_directories(finalPath.parent_path());
    }

    if (capturedImage)
    {
        imageHandler->saveImage(options.captureFilename, capturedImage, false);
    }
    else if (fs::exists(producedImage.asString()))
    {
        fs::copy_file(producedImage.asString(), finalPath, fs::copy_options::overwrite_existing);
    }
    else
    {
        throw std::runtime_error("OSL renderer did not produce expected image: " + producedImage.asString());
    }
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        Options options = parseOptions(argc, argv);
        if (options.help)
        {
            printUsage();
            return EXIT_SUCCESS;
        }
        render(options, argv);
        return EXIT_SUCCESS;
    }
    catch (mx::ExceptionRenderError& e)
    {
        std::cerr << e.what() << '\n';
        for (const std::string& error : e.errorLog())
        {
            std::cerr << error << '\n';
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return EXIT_FAILURE;
}
