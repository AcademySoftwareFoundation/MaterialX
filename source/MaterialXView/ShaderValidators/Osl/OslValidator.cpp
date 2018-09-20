#include <MaterialXView/ShaderValidators/Osl/OslValidator.h>
#include <MaterialXView/Handlers/ObjGeometryHandler.h>
#include <MaterialXGenShader/Util.h>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>

namespace MaterialX
{
//
// Creator
//
OslValidatorPtr OslValidator::create()
{
    return std::shared_ptr<OslValidator>(new OslValidator());
}

OslValidator::OslValidator() :
    ShaderValidator(),
    _useTestRender(false) // By default use testshade
{
}

OslValidator::~OslValidator()
{
}

void OslValidator::initialize()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL initialization error.");
    if (_oslIncludePathString.empty())
    {
        errors.push_back("OSL validation include path is empty.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (_oslTestShadeExecutable.empty() && _oslCompilerExecutable.empty())
    {
        errors.push_back("OSL validation executables not set.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::shadeOSL(const std::string& shaderName, const std::string& outputName)
{
    // If no command and include path specified then skip checking.
    if (_oslTestShadeExecutable.empty() || _oslIncludePathString.empty())
    {
        return;
    }

    // Set output image name. 
    std::string outputFileName = shaderName + ".png";

    // Use a known error file name to check
    std::string errorFile(shaderName + "_render_errors.txt");
    const std::string redirectString(" 2>&1");

    std::string command(_oslTestShadeExecutable);
    command += " " + shaderName;
    command += " -o " + outputName + " " + outputFileName;
    command += " -g 256 256";
    command += " > " + errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    // There is no "silent" or "quiet" mode for testshade so we must parse the lines
    // to check if there were any error lines which are not the success line.
    // Note: This is currently hard-coded to a specific value. If testshade
    // modifies this then this hard-coded string must also be modified.
    // The formatted string is "Output <outputName> to <outputFileName>".
    std::ifstream errorStream(errorFile);
    std::string result;
    std::vector<std::string> results;
    std::string line;
    std::string successfulOutputSubString("Output " + outputName + " to " + 
                                           outputFileName);
    while (std::getline(errorStream, line))
    {
        if (!line.empty() &&
            line.find(successfulOutputSubString) == std::string::npos)
        {
            results.push_back(line);
        }
    }

    if (!results.empty())
    {
        const std::string errorType("OSL rendering error.");
        ShaderValidationErrorList errors;
        errors.push_back("Command string: " + command);
        errors.push_back("Command return code: " + std::to_string(returnValue));
        errors.push_back("Shader failed to render:");
        for (auto resultLine : results)
        {
            errors.push_back(resultLine);
        }
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::compileOSL(const std::string& oslFileName)
{
    // If no command and include path specified then skip checking.
    if (_oslCompilerExecutable.empty() || _oslIncludePathString.empty())
    {
        return;
    }

    // Remove .osl and add .oso extension for output. 
    std::string outputFileName = removeExtension(oslFileName);
    outputFileName += ".oso";

    // Use a known error file name to check
    std::string errorFile(oslFileName + "_compile_errors.txt");
    const std::string redirectString(" 2>&1");

    // Run the command and get back the result. If non-empty string throw exception with error
    std::string command = _oslCompilerExecutable + " -q -I\"" + _oslIncludePathString + "\" " + oslFileName + " -o " + outputFileName + " > " +
        errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    std::ifstream errorStream(errorFile);
    std::string result;
    result.assign(std::istreambuf_iterator<char>(errorStream),
                  std::istreambuf_iterator<char>());

    if (!result.empty())
    {
        const std::string errorType("OSL compilation error.");
        ShaderValidationErrorList errors;
        errors.push_back("Command string: " + command);
        errors.push_back("Command return code: " + std::to_string(returnValue));
        errors.push_back("Shader failed to compile:");
        errors.push_back(result);
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::validateCreation(const ShaderPtr shader)
{
    std::vector<std::string> stages;
    stages.push_back(shader->getSourceCode());

    validateCreation(stages);
}

void OslValidator::validateCreation(const std::vector<std::string>& stages)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL compilation error.");
    if (stages.empty() || stages[0].empty())
    {
        errors.push_back("No shader code to validate");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    bool haveCompiler = !_oslCompilerExecutable.empty() && !_oslIncludePathString.empty();
    if (!haveCompiler)
    {
        errors.push_back("No OSL compiler specified for validation.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Dump string to disk. For OSL assume shader is in stage 0 slot.
    std::string fileName = _oslOutputFilePathString;
    if (fileName.empty())
    {
        fileName = "_osl_temp.osl";
    }
    else
    {
        fileName += ".osl";
    }
    std::ofstream file;
    file.open(fileName);
    file << stages[0];
    file.close();

    // Try compiling the code
    compileOSL(fileName);
}

void OslValidator::validateInputs()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL validation error.");

    errors.push_back("OSL input validation is not supported at this time.");
    throw ExceptionShaderValidationError(errorType, errors);
}

void OslValidator::validateRender(bool /*orthographicView*/)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL rendering error.");

    if (_oslOutputFilePathString.empty())
    {
        errors.push_back("OSL output file path string has not been specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (_oslShaderOutputName.empty())
    {
        errors.push_back("OSL shader output name has not been specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Use testshade to render with
    if (!_useTestRender)
    {
        shadeOSL(_oslOutputFilePathString, _oslShaderOutputName);
    }
    else
    {
        // TODO: testrender support has not been added at this time
        errors.push_back("testrender usage is not supported at the current time.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::save(const std::string& /*fileName*/)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL image save error.");

    if (!_imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // No image generation, thus no image save at this time.
    errors.push_back("OSL rendering image save is not supported at this time.");
    throw ExceptionShaderValidationError(errorType, errors);
}

}
