#include <MaterialXView/ShaderValidators/Osl/OslValidator.h>
#include <MaterialXView/Handlers/ObjGeometryHandler.h>

#include <fstream>
#include <iostream>
#include <algorithm>

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
    ShaderValidator()
{
}

OslValidator::~OslValidator()
{
}

void OslValidator::initialize()
{
}

void OslValidator::compileOSL(const std::string& oslFileName)
{
    // If no command and include path specified then skip checking.
    if (_oslCompilerExecutable.empty() || _oslIncludePathString.empty())
    {
        return;
    }

    // Use a known error file name to check
    std::string errorFile(oslFileName + "_errors.txt");
    const std::string redirectString(" 2>&1");

    // Run the command and get back the result. If non-empty string throw exception with error
    std::string command = _oslCompilerExecutable + " -q -I\"" + _oslIncludePathString + "\" " + oslFileName + " > " +
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
    const std::string fileName("_osl_temp.osl");
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

    errors.push_back("OSL rendering is not supported at this time.");
    throw ExceptionShaderValidationError(errorType, errors);
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
