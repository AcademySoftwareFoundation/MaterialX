//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderOsl/OslValidator.h>

#include <MaterialXGenShader/Util.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>

#include <fstream>

namespace MaterialX
{

// Statics
string OslValidator::OSL_CLOSURE_COLOR_STRING("closure color");

//
// Creator
//
OslValidatorPtr OslValidator::create()
{
    return std::shared_ptr<OslValidator>(new OslValidator());
}

OslValidator::OslValidator() :
    ShaderValidator(),
    _useTestRender(true) // By default use testrender
{
}

OslValidator::~OslValidator()
{
}

void OslValidator::initialize()
{
    ShaderValidationErrorList errors;
    const string errorType("OSL initialization error.");
    if (_oslIncludePath.isEmpty())
    {
        errors.push_back("OSL validation include path is empty.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (_oslTestShadeExecutable.isEmpty() && _oslCompilerExecutable.isEmpty())
    {
        errors.push_back("OSL validation executables not set.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::renderOSL(const FilePath& dirPath, const string& shaderName, const string& outputName)
{
    ShaderValidationErrorList errors;
    const string errorType("OSL rendering error.");

    // If command options missing, skip testing.
    if (_oslTestRenderExecutable.isEmpty() || _oslIncludePath.isEmpty() ||
        _oslTestRenderSceneTemplateFile.isEmpty() || _oslUtilityOSOPath.isEmpty())
    {
        errors.push_back("Command input arguments are missing");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    static const StringSet RENDERABLE_TYPES = { "float", "color", "vector", "closure color", "color2", "color4", "vector2", "vector4" };
    static const StringSet REMAPPABLE_TYPES = { "color2", "color4", "vector2", "vector4" };

    // If the output type is not which can be supported for rendering then skip testing.
    if (RENDERABLE_TYPES.count(_oslShaderOutputType) == 0)
    {
        errors.push_back("Output type to render is not supported: " + _oslShaderOutputType);
        throw ExceptionShaderValidationError(errorType, errors);
    }

    const bool isColorClosure = _oslShaderOutputType == "closure color";
    const bool isRemappable = REMAPPABLE_TYPES.count(_oslShaderOutputType) != 0;

    // Determine the shader path from output path and shader name
    FilePath shaderFilePath(dirPath);
    shaderFilePath = shaderFilePath / shaderName;
    string shaderPath = shaderFilePath.asString();

    // Set output image name.
    string outputFileName = shaderPath + "_osl.png";

    // Use a known error file name to check
    string errorFile(shaderPath + "_render_errors.txt");
    const string redirectString(" 2>&1");

    // Read in scene template and replace the applicable tokens to have a valid ShaderGroup.
    // Write to local file to use as input for rendering.
    //
    std::ifstream sceneTemplateStream(_oslTestRenderSceneTemplateFile);
    string sceneTemplateString;
    sceneTemplateString.assign(std::istreambuf_iterator<char>(sceneTemplateStream),
        std::istreambuf_iterator<char>());

    // Get final output to use in the shader
    const string CLOSURE_PASSTHROUGH_SHADER_STRING("closure_passthrough");
    const string CONSTANT_COLOR_SHADER_STRING("constant_color");
    const string CONSTANT_COLOR_SHADER_PREFIX_STRING("constant_");
    string outputShader = isColorClosure ? CLOSURE_PASSTHROUGH_SHADER_STRING :
        (isRemappable ? CONSTANT_COLOR_SHADER_PREFIX_STRING + _oslShaderOutputType : CONSTANT_COLOR_SHADER_STRING);

    // Perform token replacement
    const string OUTPUT_SHADER_TYPE_STRING("%output_shader_type%");
    const string OUTPUT_SHADER_INPUT_STRING("%output_shader_input%");
    const string OUTPUT_SHADER_INPUT_VALUE_STRING("Cin");
    const string INPUT_SHADER_TYPE_STRING("%input_shader_type%");
    const string INPUT_SHADER_PARAMETER_OVERRIDES("%input_shader_parameter_overrides%");
    const string INPUT_SHADER_OUTPUT_STRING("%input_shader_output%");
    const string BACKGROUND_COLOR_STRING("%background_color%");
    const string backgroundColor("0.0 0.0 0.0"); // TODO: Make this a user input

    StringMap replacementMap;
    replacementMap[OUTPUT_SHADER_TYPE_STRING] = outputShader;
    replacementMap[OUTPUT_SHADER_INPUT_STRING] = OUTPUT_SHADER_INPUT_VALUE_STRING;
    replacementMap[INPUT_SHADER_TYPE_STRING] = shaderName;
    string overrideString;
    for (auto param : _oslShaderParameterOverrides)
    {
        overrideString.append(param);
    }
    replacementMap[INPUT_SHADER_PARAMETER_OVERRIDES] = overrideString;
    replacementMap[INPUT_SHADER_OUTPUT_STRING] = outputName;
    replacementMap[BACKGROUND_COLOR_STRING] = backgroundColor;
    string sceneString = replaceSubstrings(sceneTemplateString, replacementMap);
    if ((sceneString == sceneTemplateString) || sceneTemplateString.empty())
    {
        errors.push_back("Scene template file: " + _oslTestRenderSceneTemplateFile.asString() +
                         " does not include proper tokens for rendering.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Write scene file
    const string sceneFileName(shaderPath + "_scene.xml");
    std::ofstream shaderFileStream;
    shaderFileStream.open(sceneFileName);
    if (shaderFileStream.is_open())
    {
        shaderFileStream << sceneString;
        shaderFileStream.close();
    }

    // Set oso file paths
    string osoPaths(_oslUtilityOSOPath);
    osoPaths += ";" + dirPath.asString();

    // Build and run render command
    //
    string command(_oslTestRenderExecutable);
    command += " " + sceneFileName;
    command += " " + outputFileName;
    command += " -r 512 512 --path " + osoPaths;
    if (isColorClosure)
    {
        command += " -aa 4 "; // Images are very noisy without anti-aliasing
    }
    command += " > " + errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    std::ifstream errorStream(errorFile);
    string result;
    result.assign(std::istreambuf_iterator<char>(errorStream),
        std::istreambuf_iterator<char>());

    if (!result.empty())
    {
        errors.push_back("Command string: " + command);
        errors.push_back("Command return code: " + std::to_string(returnValue));
        errors.push_back("Shader failed to render:");
        errors.push_back(result);
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::shadeOSL(const FilePath& dirPath, const string& shaderName, const string& outputName)
{
    // If no command and include path specified then skip checking.
    if (_oslTestShadeExecutable.isEmpty() || _oslIncludePath.isEmpty())
    {
        return;
    }

    FilePath shaderFilePath(dirPath);
    shaderFilePath = shaderFilePath / shaderName;
    string shaderPath = shaderFilePath.asString();

    // Set output image name.
    string outputFileName = shaderPath + ".testshade.png";

    // Use a known error file name to check
    string errorFile(shaderPath + "_shade_errors.txt");
    const string redirectString(" 2>&1");

    string command(_oslTestShadeExecutable);
    command += " " + shaderPath;
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
    string result;
    StringVec results;
    string line;
    string successfulOutputSubString("Output " + outputName + " to " +
                                           outputFileName);
    while (std::getline(errorStream, line))
    {
        if (!line.empty() &&
            line.find(successfulOutputSubString) == string::npos)
        {
            results.push_back(line);
        }
    }

    if (!results.empty())
    {
        const string errorType("OSL rendering error.");
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

void OslValidator::compileOSL(const FilePath& oslFilePath)
{
    // If no command and include path specified then skip checking.
    if (_oslCompilerExecutable.isEmpty() || _oslIncludePath.isEmpty())
    {
        return;
    }

    string oslFileName = oslFilePath.asString();

    // Remove .osl and add .oso extension for output.
    string outputFileName = removeExtension(oslFileName);
    outputFileName += ".oso";

    // Use a known error file name to check
    string errorFile(oslFileName + "_compile_errors.txt");
    const string redirectString(" 2>&1");

    // Run the command and get back the result. If non-empty string throw exception with error
    string command = _oslCompilerExecutable.asString() + " -q -I\"" + _oslIncludePath.asString() + "\" " + oslFileName + " -o " + outputFileName + " > " +
        errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    std::ifstream errorStream(errorFile);
    string result;
    result.assign(std::istreambuf_iterator<char>(errorStream),
                  std::istreambuf_iterator<char>());

    if (!result.empty())
    {
        const string errorType("OSL compilation error.");
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
    StageMap stages = { {Stage::PIXEL, shader->getStage(Stage::PIXEL).getSourceCode()} };
    validateCreation(stages);
}

void OslValidator::validateCreation(const StageMap& stages)
{
    // There is only one stage in an OSL shader so only
    // the first stage is examined.
    ShaderValidationErrorList errors;
    const string errorType("OSL compilation error.");
    if (stages.empty() || stages.begin()->second.empty())
    {
        errors.push_back("No shader code to validate");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    bool haveCompiler = !_oslCompilerExecutable.isEmpty() && !_oslIncludePath.isEmpty();
    if (!haveCompiler)
    {
        errors.push_back("No OSL compiler specified for validation.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Dump string to disk. For OSL assume shader is in stage 0 slot.
    FilePath filePath(_oslOutputFilePath);
    filePath = filePath  / _oslShaderName;
    string fileName = filePath.asString();
    if (fileName.empty())
    {
        fileName = "_osl_temp.osl";
    }
    else
    {
        fileName += ".osl";
    }

    // TODO: Seems testrender will crash currently when trying to convert to "object" space.
    // Thus we replace all instances of "object" with "world" to avoid issues.
    StringMap spaceMap;
    spaceMap["\"object\""] = "\"world\"";
    string oslCode = replaceSubstrings(stages.begin()->second, spaceMap);

    std::ofstream file;
    file.open(fileName);
    file << oslCode;
    file.close();

    // Try compiling the code
    compileOSL(fileName);
}

void OslValidator::validateInputs()
{
    ShaderValidationErrorList errors;
    const string errorType("OSL validation error.");

    errors.push_back("OSL input validation is not supported at this time.");
    throw ExceptionShaderValidationError(errorType, errors);
}

void OslValidator::validateRender(bool /*orthographicView*/)
{
    ShaderValidationErrorList errors;
    const string errorType("OSL rendering error.");

    if (_oslOutputFilePath.isEmpty())
    {
        errors.push_back("OSL output file path string has not been specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (_oslShaderOutputName.empty())
    {
        errors.push_back("OSL shader output name has not been specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Use testshade
    if (!_useTestRender)
    {
        shadeOSL(_oslOutputFilePath, _oslShaderName, _oslShaderOutputName);
    }

    // Use testrender
    else
    {
        if (_oslShaderName.empty())
        {
            errors.push_back("OSL shader name has not been specified.");
            throw ExceptionShaderValidationError(errorType, errors);
        }
        renderOSL(_oslOutputFilePath, _oslShaderName, _oslShaderOutputName);
    }
}

void OslValidator::save(const FilePath& /*filePath*/, bool /*floatingPoint*/)
{
    // No-op: image save is done as part of rendering.
}

}
