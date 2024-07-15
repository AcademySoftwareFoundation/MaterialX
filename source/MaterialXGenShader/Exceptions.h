//
// Created by Lee Kerley on 6/11/24.
//

#ifndef MATERIALX_EXCEPTIONS_H
#define MATERIALX_EXCEPTIONS_H


/// @file
/// Exception classes for shader generation

#include <MaterialXGenShader/Export.h>

#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/GenUserData.h>
#include <MaterialXGenShader/ShaderNode.h>

#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

/// @class ExceptionShaderGenError
/// An exception that is thrown when shader generation fails.
class MX_GENSHADER_API ExceptionShaderGenError : public Exception
{
  public:
    using Exception::Exception;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_EXCEPTIONS_H
