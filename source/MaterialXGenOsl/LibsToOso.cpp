//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <fstream>
#include <iostream>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXGenShader/ShaderStage.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXRenderOsl/OslRenderer.h>

namespace mx = MaterialX;

const std::string setCiOslNetworkSource = R"(

#include "mx_funcs.h"

#define true 1
#define false 0
struct textureresource { string filename; string colorspace; };
#define BSDF closure color
#define EDF closure color
#define VDF closure color
struct surfaceshader { closure color bsdf; closure color edf; float opacity; };
#define volumeshader closure color
#define displacementshader vector
#define lightshader closure color
#define MATERIAL closure color

#define M_FLOAT_EPS 1e-8
closure color null_closure() { closure color null_closure = 0; return null_closure; }

shader setCi (
    float float_input = 0,
    color color3_input = 0,
    color4 color4_input = {0,0},
    vector2 vector2_input = {0,0},
    vector vector3_input = 0,
    vector4 vector4_input = {0,0},
    surfaceshader surfaceshader_input = {0,0,0},
    BSDF BSDF_input = 0,
    EDF EDF_input = 0,
    MATERIAL material_input = 0,

    output closure color Out_Ci = 0
)
{
    color c = 0;
    float a = 1;

    if (isconnected(surfaceshader_input)) {

        float opacity_weight = clamp(surfaceshader_input.opacity, 0.0, 1.0);
        Out_Ci =  (surfaceshader_input.bsdf + surfaceshader_input.edf) * opacity_weight + transparent() * (1.0 - opacity_weight);

    } else if (isconnected(material_input)) {
        Out_Ci = material_input;
    } else if (isconnected(BSDF_input)) {
        Out_Ci = BSDF_input;
    } else if (isconnected(EDF_input)) {
        Out_Ci = EDF_input;
    } else {
        if (isconnected(float_input)) {
            c = float_input;
        } else if (isconnected(color3_input)) {
            c = color3_input;
        } else if (isconnected(color4_input)) {
            c = color4_input.rgb;
            a = color4_input.a;
        } else if (isconnected(vector2_input)) {
            c = color(vector2_input.x, vector2_input.y, 0);
        } else if (isconnected(vector3_input)) {
            c = color(vector3_input);
        } else if (isconnected(vector4_input)) {
            c = color(vector4_input.x, vector4_input.y, vector4_input.z);
            a = vector4_input.w;
        }
        Out_Ci = c * a * emission() + (1-a) * transparent();
    }

    Ci = Out_Ci;
}
)";

const std::string options =
    "    Options: \n"
    "        --outputOsoPath [DIRPATH]       TODO\n"
    "        --libraryRelativeOsoPath [DIRPATH]       TODO\n"
    "        --outputMtlxPath [DIRPATH]      TODO\n"
    "        --oslCompilerPath [FILEPATH]    TODO\n"
    "        --oslIncludePath [DIRPATH]      TODO\n"
    "        --libraries [STRING]            TODO\n"
    "        --removeNdPrefix [BOOLEAN]      TODO\n"
    "        --prefix [STRING]               TODO\n"
    "        --help                          Display the complete list of command-line options\n";

template <class T> void parseToken(std::string token, std::string type, T& res)
{
    if (token.empty())
        return;

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

    // Gather the provided arguments.
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    std::string argOutputOsoPath;
    std::string argOutputMtlxPath;
    std::string argLibraryRelativeOsoPath;
    std::string argOslCompilerPath;
    std::string argOslIncludePath;
    std::string argLibraries;
    bool argRemoveNdPrefix = false;
    std::string argPrefix;

    // Loop over the provided arguments, and store their associated values.
    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;

        if (token == "--outputOsoPath")
            argOutputOsoPath = nextToken;
        else if (token == "--outputMtlxPath")
            argOutputMtlxPath = nextToken;
        else if (token == "--libraryRelativeOsoPath")
            argLibraryRelativeOsoPath = nextToken;
        else if (token == "--oslCompilerPath")
            argOslCompilerPath = nextToken;
        else if (token == "--oslIncludePath")
            argOslIncludePath = nextToken;
        else if (token == "--libraries")
            argLibraries = nextToken;
        else if (token == "--removeNdPrefix")
            parseToken(nextToken, "boolean", argRemoveNdPrefix);
        else if (token == "--prefix")
            argPrefix = nextToken;
        else if (token == "--help")
        {
            std::cout << "MaterialXGenOslNetwork - LibsToOso version " << mx::getVersionString() << std::endl;
            std::cout << options << std::endl;

            return 0;
        }
        else
        {
            std::cout << "Unrecognized command-line option: " << token << std::endl;
            std::cout << "Run LibsToOso with '--help' for a complete list of supported "
                         "options."
                      << std::endl;

            continue;
        }

        if (nextToken.empty())
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        else
            i++;
    }

    // Ensure we have a valid output path.
    mx::FilePath outputOsoPath(argOutputOsoPath);

    if (!outputOsoPath.exists() || !outputOsoPath.isDirectory())
    {
        outputOsoPath.createDirectory();

        if (!outputOsoPath.exists() || !outputOsoPath.isDirectory())
        {
            std::cerr << "Failed to find and/or create the provided output oso "
                         "path: "
                      << outputOsoPath.asString() << std::endl;

            return 1;
        }
    }

    mx::FilePath outputMtlxPath(argOutputMtlxPath);
    if (!outputMtlxPath.exists() || !outputMtlxPath.isDirectory())
    {
        outputMtlxPath.createDirectory();

        if (!outputMtlxPath.exists() || !outputMtlxPath.isDirectory())
        {
            std::cerr << "Failed to find and/or create the provided output Mtlx "
                         "path: "
                      << outputMtlxPath.asString() << std::endl;

            return 1;
        }
    }

    // Ensure we have a valid path to the OSL compiler.
    mx::FilePath oslCompilerPath(argOslCompilerPath);

    if (!oslCompilerPath.exists())
    {
        std::cerr << "The provided path to the OSL compiler is not valid: " << oslCompilerPath.asString() << std::endl;

        return 1;
    }

    // Ensure we have a valid path to the OSL includes.
    mx::FilePath oslIncludePath(argOslIncludePath);

    if (!oslIncludePath.exists() || !oslIncludePath.isDirectory())
    {
        std::cerr << "The provided path to the OSL includes is not valid: " << oslIncludePath.asString() << std::endl;

        return 1;
    }

    // Create the libraries search path and document.
    mx::FileSearchPath librariesSearchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr librariesDoc = mx::createDocument();

    // If a list of comma separated libraries was provided, load them individually into our document.
    if (!argLibraries.empty())
    {
        // TODO: Should we check that we actually split something based on the separator, just to be sure?
        const mx::StringVec& librariesVec = mx::splitString(argLibraries, ",");
        mx::FilePathVec librariesPaths{ "libraries/targets" };

        for (const std::string& library : librariesVec)
            librariesPaths.emplace_back("libraries/" + library);

        loadLibraries(librariesPaths, librariesSearchPath, librariesDoc);
    }
    // Otherwise, simply load all the available libraries.
    else
        loadLibraries({ "libraries" }, librariesSearchPath, librariesDoc);

    const std::string target = "genoslnetwork";
    mx::FilePath implMtlxDocFilePath = outputMtlxPath / "genoslnetwork_impl.mtlx";
    mx::DocumentPtr implMtlxDoc = mx::createDocument();

    // Create and setup the `OslRenderer` that will be used to both generate the `.osl` files as well as compile
    // them to `.oso` files.
    mx::OslRendererPtr oslRenderer = mx::OslRenderer::create();
    oslRenderer->setOslCompilerExecutable(oslCompilerPath);

    // Build the list of include paths that will be passed to the `OslRenderer`.
    mx::FileSearchPath oslRendererIncludePaths;

    // Add the provided OSL include path.
    oslRendererIncludePaths.append(oslIncludePath);
    // Add the MaterialX's OSL include path.
    oslRendererIncludePaths.append(librariesSearchPath.find("libraries/stdlib/genosl/include"));

    oslRenderer->setOslIncludePath(oslRendererIncludePaths);

    // Create the OSL shader generator.
    mx::ShaderGeneratorPtr oslShaderGen = mx::OslShaderGenerator::create();

    // Register types from the libraries on the OSL shader generator.
    oslShaderGen->registerTypeDefs(librariesDoc);

    // Setup the context of the OSL shader generator.
    mx::GenContext context(oslShaderGen);
    context.registerSourceCodeSearchPath(librariesSearchPath);
    // TODO: It might be good to find a way to not hardcode these options, especially the texture flip.
    context.getOptions().addUpstreamDependencies = false;
    context.getOptions().fileTextureVerticalFlip = false;
    context.getOptions().oslImplicitSurfaceShaderConversion = false;

    // We'll use this boolean to return an error code is one of the `NodeDef` failed to codegen/compile.
    bool hasFailed = false;
    try
    {
        const std::string& oslFilePath = (outputOsoPath / "setCi.osl").asString();
        std::ofstream oslFile;

        // TODO: Check that we have a valid/opened file descriptor before doing anything with it?
        oslFile.open(oslFilePath);
        // Dump the content of the codegen'd `NodeDef` to our `.osl` file.
        oslFile << setCiOslNetworkSource;
        oslFile.close();

        // Compile the `.osl` file to a `.oso` file next to it.
        oslRenderer->compileOSL(oslFilePath);
    }
    // Catch any codegen/compilation related exceptions.
    catch (mx::ExceptionRenderError& exc)
    {
        std::cout << "Encountered a codegen/compilation related exception for the "
                     "following node: "
                  << std::endl;
        std::cout << exc.what() << std::endl;

        // Dump details about the exception in the log file.
        for (const std::string& error : exc.errorLog())
        {
            std::cout << error << std::endl;
        }

        hasFailed = true;
    }
    // Catch any other exceptions
    catch (mx::Exception& exc)
    {
        std::cout << "Failed to codegen/compile the following node to OSL: " << std::endl;
        std::cout << exc.what() << std::endl;

        hasFailed = true;
    }

    // We create and use a dedicated `NodeGraph` to avoid `NodeDef` names collision.
    mx::NodeGraphPtr librariesDocGraph = librariesDoc->addNodeGraph("librariesDocGraph");

    // Loop over all the `NodeDef` gathered in our documents from the provided libraries.
    for (const mx::NodeDefPtr& nodeDef : librariesDoc->getNodeDefs())
    {
        std::string nodeName = nodeDef->getName();

        // Remove the "ND_" prefix from a valid `NodeDef` name.
        if (argRemoveNdPrefix)
        {
            if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
                nodeName = nodeName.substr(3);

            // Add a prefix to the shader's name, both in the filename as well as inside the shader itself.
            if (!argPrefix.empty())
                nodeName = argPrefix + "_" + nodeName;
        }

        // Determine whether or not there's a valid implementation of the current `NodeDef` for the type associated
        // to our OSL shader generator, i.e. OSL, and if not, skip it.
        mx::InterfaceElementPtr nodeImpl = nodeDef->getImplementation(oslShaderGen->getTarget());

        if (!nodeImpl)
        {
            std::cout << "The following `NodeDef` does not provide a valid OSL implementation, "
                         "and will be skipped: "
                      << nodeDef->getName() << std::endl;

            continue;
        }

        // TODO: Check for the existence/validity of the `Node`?
        mx::NodePtr node = librariesDoc->addNodeInstance(nodeDef, nodeName);

        std::string oslShaderName = node->getName();
        oslShaderGen->getSyntax().makeValidName(oslShaderName);

        const std::string oslFileName = oslShaderName + ".osl";
        const std::string& oslFilePath = (outputOsoPath / oslFileName).asString();
        std::ofstream oslFile;

        // Codegen the `Node` to an `.osl` file.
        try
        {
            // Codegen the `Node` to OSL.
            mx::ShaderPtr oslShader = oslShaderGen->generate(node->getName(), node, context);

            // TODO: Check that we have a valid/opened file descriptor before doing anything with it?
            oslFile.open(oslFilePath);
            // Dump the content of the codegen'd `NodeDef` to our `.osl` file.
            oslFile << oslShader->getSourceCode();
            oslFile.close();
        }
        // Catch any codegen/compilation related exceptions.
        catch (mx::ExceptionShaderGenError& exc)
        {
            std::cerr << "Encountered a shader codegen related exception for the "
                         "following node: "
                      << nodeDef->getName() << std::endl;
            std::cerr << exc.what() << std::endl;

            hasFailed = true;
        }

        // Compile the codegen'd `.osl` file.
        try
        {
            // Compile the `.osl` file to a `.oso` file next to it.
            oslRenderer->compileOSL(oslFilePath);

            {
                std::string implName = "IMPL_" + nodeName + "_" + target;
                auto impl = implMtlxDoc->addImplementation(implName);
                impl->setNodeDef(nodeDef);
                // TODO: stash the the OSO path here.
                // This is writing the absolute path - which we don't want
                impl->setFile(argLibraryRelativeOsoPath);

                impl->setFunction(oslShaderName);
                impl->setAttribute("sourcecode", "dummy");
                impl->setTarget(target);
            }
        }
        // Catch any codegen/compilation related exceptions.
        catch (mx::ExceptionRenderError& exc)
        {
            std::cerr << "Encountered a shader compilation related exception for the "
                         "following node: "
                      << nodeDef->getName() << std::endl;
            std::cerr << exc.what() << std::endl;

            // Dump details about the exception in the log file.
            for (const std::string& error : exc.errorLog())
                std::cerr << error << std::endl;

            hasFailed = true;
        }
    }

    mx::writeToXmlFile(implMtlxDoc, implMtlxDocFilePath);

    // If something went wrong, return an appropriate error code.
    if (hasFailed)
    {
        std::cerr << "Failed to codegen and compile all the OSL shaders associated to the provided MaterialX "
                     "libraries, see the log file for more details."
                  << std::endl;

        return 1;
    }

    return 0;
}
