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

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>

#ifdef USE_OSLCOMP
#include <OSL/oslcomp.h>
#endif

namespace mx = MaterialX;

const std::string argOptions =
    " Options: \n"
    "    --outputOsoPath [DIRPATH]            Directory where compiled OSO files will be written\n"
    "    --libraryRelativeOsoPath [DIRPATH]   Relative path used in MaterialX implementation elements to reference OSO files\n"
    "    --outputMtlxPath [DIRPATH]           Directory where MaterialX implementation documents will be written\n"
    "    --oslCompilerPath [FILEPATH]         Path to the OSL compiler executable (oslc)\n"
#ifdef USE_OSLCOMP
    "    --useOslC                            Use external OSL compiler instead of liboslcomp (when available)\n"
#endif
    "    --skipWritingOSLSource               Skip writing OSL source files to disk during compilation\n"
    "    --skipWritingMtlxDoc                 Skip writing MaterialX implementation documents\n"
    "    --skipConvertingNodegraphs           Skip converting nodegraph implementations to OSL shaders\n"
    "    --oslIncludePath [DIRPATH]           Directory containing OSL include files\n"
    "    --path [FILEPATH]                    Specify an additional data search path location (e.g. '/projects/MaterialX').  This absolute path will be queried when locating data libraries, XInclude references, and referenced images.\n"
    "    --library [FILEPATH]                 Specify an additional data library folder (e.g. 'vendorlib', 'studiolib').  This relative path will be appended to each location in the data search path when loading data libraries.\n"
    "    --osoNameStrategy [STRING]           Naming strategy for OSO files - either 'implementation' or 'nodedef' (default:'implementation')\n"
    "    --help                               Display the complete list of command-line options\n";


void createMtlxImplementationElementForOsoNode(mx::ConstNodeDefPtr nodeDef, mx::ShaderPtr oslShader, const std::string& oslShaderName, mx::DocumentPtr mtlxDoc, const std::string& osoPath)
{
    mx::ImplementationPtr impl = mtlxDoc->addImplementation(oslShaderName);
    impl->setNodeDef(nodeDef);
    impl->setFile(osoPath);

    impl->setFunction(oslShaderName);
    impl->setAttribute("sourcecode", "dummy");
    impl->setTarget(mx::OslNetworkShaderGenerator::TARGET);

    // Compare the shader input names in the source (variable)
    // with the names of the corresponding port.
    // If there is a difference then we use the "implname"
    // attribute to record that difference so that the OSLNetwork generator
    // can use it.
    mx::ShaderGraph& graph = oslShader->getGraph();
    for (mx::ShaderGraphInputSocket* inputSocket : graph.getInputSockets())
    {
        std::string name = inputSocket->getName();
        std::string variable = inputSocket->getVariable();

        if (inputSocket->getType() == mx::Type::FILENAME)
        {
            // We suffix the inputs of type FILENAME with "_resource" so we need to
            // remove that here for the implName comparison
            variable = std::string(variable, 0, variable.size() - std::string("_resource").size());
        }

        if (name != variable)
        {
            mx::InputPtr implInput = impl->addInput(name, inputSocket->getType().getName());
            implInput->setImplementationName(variable);
        }
    }
}

int main(int argc, char* const argv[])
{
    std::vector<std::string> tokens;

    // Gather the provided arguments.
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    mx::FileSearchPath argSearchPath = mx::getDefaultDataSearchPath();
    mx::FilePathVec argLibraryFolders;
    std::string argOutputOsoPath;
    std::string argOutputMtlxPath;
    std::string argLibraryRelativeOsoPath;
    std::string argOslCompilerPath;
    std::string argOslIncludePath;
    std::string argOsoNameStrategy = "implementation";
    bool argSkipWritingSource = false;
    bool argSkipWritingMtlxDoc = false;
    bool argSkipConvertingNodegraphs = false;
    bool argUseOslC = false;

    // Loop over the provided arguments, and store their associated values.
    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;

        if (token == "--outputOsoPath")
        {
            argOutputOsoPath = nextToken;
        }
        else if (token == "--outputMtlxPath")
        {
            argOutputMtlxPath = nextToken;
        }
        else if (token == "--libraryRelativeOsoPath")
        {
            argLibraryRelativeOsoPath = nextToken;
        }
        else if (token == "--oslCompilerPath")
        {
            argOslCompilerPath = nextToken;
        }
        else if (token == "--oslIncludePath")
        {
            argOslIncludePath = nextToken;
        }
        else if (token == "--path")
        {
            argSearchPath.append(mx::FileSearchPath(nextToken));
        }
        else if (token == "--library")
        {
            argLibraryFolders.push_back(nextToken);
        }
        else if (token == "--osoNameStrategy")
        {
            argOsoNameStrategy = nextToken;
        }
        else if (token == "--skipWritingOSLSource")
        {
            argSkipWritingSource = true;
        }
        else if (token == "--skipWritingMtlxDoc")
        {
            argSkipWritingMtlxDoc = true;
        }
        else if (token == "--skipConvertingNodegraphs")
        {
            argSkipConvertingNodegraphs = true;
        }
        else if (token == "--useOslC")
        {
            argUseOslC = true;
        }
        else if (token == "--help")
        {
            std::cout << "MaterialXGenOslNetwork - LibsToOso version " << mx::getVersionString();
#ifdef USE_OSLCOMP
            std::cout << " - Compiled with liboslcomp";
#endif
            std::cout << std::endl;
            std::cout << argOptions << std::endl;

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

        if (!nextToken.empty())
            i++;
    }

    if (!(argOsoNameStrategy == "implementation" || argOsoNameStrategy == "nodedef"))
    {
        std::cerr << "Unrecognized value for --osoNameStrategy '" << argOsoNameStrategy << "'. Must be 'implementation' or 'nodedef'" << std::endl;
        return 1;
    }

    // Append the standard library folder, giving it a lower precedence than user-supplied libraries.
    argLibraryFolders.push_back("libraries");

    // Ensure we have a valid output path.
    mx::FilePath outputOsoPath(argOutputOsoPath);
    if (!outputOsoPath.exists() || !outputOsoPath.isDirectory())
    {
        outputOsoPath.createDirectory(true);

        if (!outputOsoPath.exists() || !outputOsoPath.isDirectory())
        {
            std::cerr << "Failed to find and/or create the provided output oso "
                         "path: "
                      << outputOsoPath.asString() << std::endl;

            return 1;
        }
    }

    mx::FilePath outputMtlxPath(argOutputMtlxPath);
    if (argSkipWritingMtlxDoc && (!outputMtlxPath.exists() || !outputMtlxPath.isDirectory()))
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
    mx::DocumentPtr librariesDoc = mx::createDocument();
    try
    {
        mx::loadLibraries(argLibraryFolders, argSearchPath, librariesDoc);
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to load standard data libraries: " << e.what() << std::endl;
        return 1;
    }

    const std::string target = "genoslnetwork";
    mx::DocumentPtr implMtlxDoc = nullptr;
    if (!argSkipWritingMtlxDoc)
    {
        implMtlxDoc = mx::createDocument();
    }

    // Build the list of include paths that will be used to compile the shader.
    mx::FileSearchPath oslRendererIncludePaths;
    // Add the provided OSL include path.
    oslRendererIncludePaths.append(oslIncludePath);
    // Add the MaterialX's OSL include path.
    oslRendererIncludePaths.append(argSearchPath.find("libraries/stdlib/genosl/include"));

    // Create the OSL shader generator.
    mx::OslShaderGeneratorPtr oslShaderGen = std::static_pointer_cast<mx::OslShaderGenerator>(mx::OslShaderGenerator::create());

    // Register types from the libraries on the OSL shader generator.
    oslShaderGen->registerTypeDefs(librariesDoc);

    // Setup the context of the OSL shader generator.
    mx::GenContext context(oslShaderGen);
    context.registerSourceCodeSearchPath(argSearchPath);
    // TODO: It might be good to find a way to not hardcode these options, especially the texture flip.
    context.getOptions().addUpstreamDependencies = false;
    context.getOptions().fileTextureVerticalFlip = false;
    context.getOptions().oslImplicitSurfaceShaderConversion = false;

    mx::OslNetworkShaderGenerator::OslCompileOptions options;
    options.oslIncludePath = oslRendererIncludePaths;
    options.writeSourceToDisk = !argSkipWritingSource;
    options.oslCompilerPath = argOslCompilerPath;
#ifdef USE_OSLCOMP
    options.useOslComp = !argUseOslC;
#endif

    // We'll use this boolean to return an error code is one of the `NodeDef` failed to codegen/compile.
    bool hasFailed = false;

    // Loop over all the `NodeDef` gathered in our documents from the provided libraries.
    for (mx::ConstNodeDefPtr nodeDef : librariesDoc->getNodeDefs())
    {
        // Determine whether or not there's a valid implementation of the current `NodeDef` for the type associated
        // to our OSL shader generator, i.e. OSL, and if not, skip it.
        mx::InterfaceElementPtr nodeImpl = nodeDef->getImplementation(oslShaderGen->getTarget(), false);

        if (!nodeImpl)
        {
            std::cout << "The following `NodeDef` does not provide a valid OSL implementation, "
                         "and will be skipped: "
                      << nodeDef->getName() << std::endl;
            continue;
        }

        if (argSkipConvertingNodegraphs)
        {
            // If we've been asked to skip converting nodegraphs - then we need to check
            // what type of implementation we've been given.
            if (nodeImpl->isA<mx::NodeGraph>())
            {
                std::cout << "Skipping " << nodeDef->getName() << " - NodeGraph implementation" << std::endl;
                continue;
            }
        }

        mx::ShaderPtr oslShader = nullptr;
        try
        {
            // Codegen the `Node` to OSL.
            oslShader =  mx::OslNetworkShaderGenerator::generateOSLShader(nodeDef, oslShaderGen, context, argOsoNameStrategy);
        }
        // Catch any codegen/compilation related exceptions.
        catch (mx::ExceptionShaderGenError& exc)
        {
            std::cerr << "Encountered a shader codegen related exception for the "
                         "following node: "
                      << nodeDef->getName() << std::endl;
            std::cerr << exc.what() << std::endl;
        }

        if (!oslShader)
        {
            std::cerr << "Failed to generate shader for the "
                         "following node: "
                      << nodeDef->getName() << std::endl;
            hasFailed = true;
        }

        const std::string oslFileName = oslShader->getName() + ".osl";
        const std::string& oslFilePath = (outputOsoPath / oslFileName).asString();
        std::ofstream oslFile;

        try
        {
            // Compile the `.osl` file to a `.oso` file next to it.
            mx::OslNetworkShaderGenerator::compileOSL(oslShader->getSourceCode(), oslFilePath, options);

            // Create the MaterialX Implementation element
            createMtlxImplementationElementForOsoNode(nodeDef, oslShader, oslShader->getName(), implMtlxDoc, argLibraryRelativeOsoPath);
        }
        // Catch any codegen/compilation related exceptions.
        catch (mx::ExceptionOslCompileError& exc)
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

    if (implMtlxDoc)
    {
        // We only write the MaterialX document containing the implementations out if requested.
        mx::FilePath implMtlxDocFilePath = outputMtlxPath / "genoslnetwork_impl.mtlx";
        mx::writeToXmlFile(implMtlxDoc, implMtlxDocFilePath);
    }

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
