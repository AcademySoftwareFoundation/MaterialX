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

const std::string options =
    "    Options: \n"
    "        --outputOsoPath [DIRPATH]       TODO\n"
    "        --libraryRelativeOsoPath [DIRPATH]       TODO\n"
    "        --outputMtlxPath [DIRPATH]      TODO\n"
    "        --oslCompilerPath [FILEPATH]    TODO\n"
    "        --oslIncludePath [DIRPATH]      TODO\n"
    "        --libraries [STRING]            TODO\n"
    "        --osoNameStrategy [STRING]      TODO - either 'implementation' or 'nodedef' (default:'implementation')\n"
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
    std::string argOsoNameStrategy = "implementation";

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
        else if (token == "--osoNameStrategy")
            argOsoNameStrategy = nextToken;
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

    if (!(argOsoNameStrategy == "implementation" || argOsoNameStrategy == "nodedef"))
    {
        std::cerr << "Unrecognized value for --osoNameStrategy '" << argOsoNameStrategy <<
            "'. Must be 'implementation' or 'nodedef'" << std::endl;
        return 1;
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

    // We create and use a dedicated `NodeGraph` to avoid `NodeDef` names collision.
    mx::NodeGraphPtr librariesDocGraph = librariesDoc->addNodeGraph("librariesDocGraph");

    // Loop over all the `NodeDef` gathered in our documents from the provided libraries.
    for (mx::NodeDefPtr nodeDef : librariesDoc->getNodeDefs())
    {
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

        // Intention is here is to name the new node the same as the genosl implementation name
        // but replacing "_genosl" with "_genoslnetwork"
        std::string nodeName;
        if (argOsoNameStrategy == "implementation")
        {
            // Name the node the same as the implementation with _genoslnetwork added as a suffix.
            // NOTE : If the implementation currently has _genosl as a suffix then we remove it.
            nodeName = nodeImpl->getName();
            nodeName = mx::replaceSubstrings(nodeName, {{"_genosl", ""}});
            nodeName += "_genoslnetwork";
        }
        else
        {
            // Name the node the same as the node definition
            nodeName = nodeDef->getName();
        }

        mx::NodePtr node = librariesDocGraph->addNodeInstance(nodeDef, nodeName);
        if (!node)
        {
            std::cerr << "Unable to create Node instance for NodeDef - '" << nodeDef->getName() << "'" << std::endl;
            return 1;
        }

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
