//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALXGENSHADER_EXCEPTION_H
#define MATERIALXGENSHADER_EXCEPTION_H

/// @file
/// Base shader generator exception class

#include <MaterialXGenShader/Export.h>

#include <MaterialXCore/Exception.h>

MATERIALX_NAMESPACE_BEGIN

/// @class ExceptionShaderGenError
/// An exception that is thrown when shader generation fails.
class MX_GENSHADER_API ExceptionShaderGenError : public Exception
{
  public:
    using Exception::Exception;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALXGENSHADER_EXCEPTION_H
