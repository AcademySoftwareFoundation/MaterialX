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
#include <MaterialXGenOsl/OslSyntax.h>

#ifdef USE_OSLCOMP
#include <OSL/oslcomp.h>
#endif

namespace mx = MaterialX;

const std::string argOptions =
    " Options: \n"
    "    --outputOsoPath [DIRPATH]       TODO\n"
    "    --libraryRelativeOsoPath [DIRPATH]       TODO\n"
    "    --outputMtlxPath [DIRPATH]      TODO\n"
    "    --oslCompilerPath [FILEPATH]    TODO\n"
#ifdef USE_OSLCOMP
    "    --useOslC                       TODO\n"
#endif
    "    --skipWritingOSLSource          TODO\n"
    "    --skipWritingMtlxDoc            TODO\n"
    "    --oslIncludePath [DIRPATH]      TODO\n"
    "    --path [FILEPATH]              Specify an additional data search path location (e.g. '/projects/MaterialX').  This absolute path will be queried when locating data libraries, XInclude references, and referenced images.\n"
    "    --library [FILEPATH]           Specify an additional data library folder (e.g. 'vendorlib', 'studiolib').  This relative path will be appended to each location in the data search path when loading data libraries.\n"
    "    --osoNameStrategy [STRING]      TODO - either 'implementation' or 'nodedef' (default:'implementation')\n"
    "    --help                          Display the complete list of command-line options\n";

class ExceptionCompileError : public mx::Exception
{
public:
    ExceptionCompileError(const std::string& msg, const mx::StringVec& errorLog = mx::StringVec()) :
        Exception(msg),
        _errorLog(errorLog)
    {
    }

    ExceptionCompileError(const ExceptionCompileError& e) :
        Exception(e),
        _errorLog(e._errorLog)
    {
    }

    ExceptionCompileError& operator=(const ExceptionCompileError& e)
    {
        Exception::operator=(e);
        _errorLog = e._errorLog;
        return *this;
    }

    const mx::StringVec& errorLog() const
    {
        return _errorLog;
    }

private:
    mx::StringVec _errorLog;
};

// A set of options for controlling the behavior of OSL compilation.
class OslCompileOptions
{
public:
    OslCompileOptions() = default;
    ~OslCompileOptions() = default;

    mx::FilePath oslCompilerPath;
    mx::FileSearchPath oslIncludePath;
    bool useOslComp = false;
    bool writeSourceToDisk = true;
};

bool compileOSL(const std::string& oslSourceCode, const mx::FilePath& oslFilePath, const OslCompileOptions& options)
{
    if (!options.useOslComp && !options.writeSourceToDisk)
    {
        throw mx::Exception("If OslComp library is not being used the source must be written to disk");
    }

    if (options.writeSourceToDisk)
    {
        std::ofstream oslFile;
        oslFile.open(oslFilePath);
        oslFile << oslSourceCode;
        oslFile.close();
    }

    mx::FilePath osoFilePath = oslFilePath;
    osoFilePath.removeExtension();
    osoFilePath.addExtension("oso");

    // build up a vector of compiler arguments that will be
    // used in both compiler modes.
    std::vector<std::string> oslCompilerArgs;
    oslCompilerArgs.emplace_back("-o");
    oslCompilerArgs.emplace_back(osoFilePath);
    for (mx::FilePath p : options.oslIncludePath)
    {
        oslCompilerArgs.emplace_back("-I" + p.asString() + "");
    }

#ifdef USE_OSLCOMP
    if (options.useOslComp)
    {
        // Use OSL::oslcomp to compile the shader - this is significantly faster than using the system
        // call to involke the `oslc` command line tool
        OIIO::ErrorHandler errorHandler;
        ::OSL::OSLCompiler compiler(&errorHandler);
        if (options.writeSourceToDisk)
        {
            // Compile from the source file
            compiler.compile(oslFilePath.asString(), oslCompilerArgs);
        }
        else
        {
            // Compile directly from the string buffer
            std::string osoBuffer;
            compiler.compile_buffer(oslSourceCode, osoBuffer, oslCompilerArgs, std::string_view(), oslFilePath.asString());

            std::ofstream osoFile;
            osoFile.open(osoFilePath.asString());
            osoFile << osoBuffer;
            osoFile.close();
        }
    }
    else
#endif
    {
        // If no command and include path specified then skip checking.
        if (options.oslCompilerPath.isEmpty())
        {
            throw mx::Exception("OSL compiler path missing");
        }
        if (!options.oslCompilerPath.exists())
        {
            throw mx::Exception("OSL compiler doesn't exist at '" + options.oslCompilerPath.asString() + "'");
        }

        // Use a known error file name to check
        std::string errorFile(osoFilePath.asString() + "_compile_errors.txt");
        const std::string redirectString(" 2>&1");

        // Run the command and get back the result. If non-empty string throw exception with error
        std::string command = options.oslCompilerPath.asString() + " -q " + oslFilePath.asString();
        for (const auto& arg : oslCompilerArgs)
        {
            command += " " + arg;
        }
        // command += " > " + errorFile + redirectString;

        int returnValue = std::system(command.c_str());

        std::ifstream errorStream(errorFile);
        std::string result;
        result.assign(std::istreambuf_iterator<char>(errorStream),
                      std::istreambuf_iterator<char>());

        if (!result.empty())
        {
            mx::StringVec errors;
            errors.push_back("Command string: " + command);
            errors.push_back("Command return code: " + std::to_string(returnValue));
            errors.push_back("Shader failed to compile:");
            errors.push_back(result);
            throw ExceptionCompileError("OSL compilation error", errors);
        }
    }

    return true;
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

        if (nextToken.empty())
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        else
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
    mx::ShaderGeneratorPtr oslShaderGen = mx::OslShaderGenerator::create();

    // Register types from the libraries on the OSL shader generator.
    oslShaderGen->registerTypeDefs(librariesDoc);

    // Setup the context of the OSL shader generator.
    mx::GenContext context(oslShaderGen);
    context.registerSourceCodeSearchPath(argSearchPath);
    // TODO: It might be good to find a way to not hardcode these options, especially the texture flip.
    context.getOptions().addUpstreamDependencies = false;
    context.getOptions().fileTextureVerticalFlip = false;
    context.getOptions().oslImplicitSurfaceShaderConversion = false;

    OslCompileOptions options;
    options.oslIncludePath = oslRendererIncludePaths;
    options.writeSourceToDisk = !argSkipWritingSource;
    options.oslCompilerPath = argOslCompilerPath;
#ifdef USE_OSLCOMP
    options.useOslComp = !argUseOslC;
#endif

    // We'll use this boolean to return an error code is one of the `NodeDef` failed to codegen/compile.
    bool hasFailed = false;

    // We create and use a dedicated `NodeGraph` to avoid `NodeDef` names collision.
    mx::NodeGraphPtr librariesDocGraph = librariesDoc->addNodeGraph("librariesDocGraph");

    // Loop over all the `NodeDef` gathered in our documents from the provided libraries.
    for (mx::NodeDefPtr nodeDef : librariesDoc->getNodeDefs())
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

        // Intention is here is to name the new node the same as the genosl implementation name
        // but replacing "_genosl" with "_genoslnetwork"
        std::string nodeName;
        if (argOsoNameStrategy == "implementation")
        {
            // Name the node the same as the implementation with _genoslnetwork added as a suffix.
            // NOTE : If the implementation currently has _genosl as a suffix then we remove it.
            nodeName = nodeImpl->getName();
            nodeName = mx::replaceSubstrings(nodeName, { { "_genosl", "" } });
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
        mx::ShaderPtr oslShader = nullptr;
        try
        {
            // Codegen the `Node` to OSL.
            oslShader = oslShaderGen->generate(node->getName(), node, context);

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
            compileOSL(oslShader->getSourceCode(), oslFilePath, options);

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
        catch (ExceptionCompileError& exc)
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
