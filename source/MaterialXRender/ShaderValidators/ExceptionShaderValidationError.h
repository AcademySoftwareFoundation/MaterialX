#ifndef MATERIALX__EXCEPTIONSHADERVALIDATIONERROR_H
#define MATERIALX__EXCEPTIONSHADERVALIDATIONERROR_H

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// Error string list type
using ShaderValidationErrorList = vector<string>;

/// @class @ExceptionShaderValidationError
/// An exception that is thrown when shader validation fails.
/// An error log of shader errors is cached as part of the exception.
/// For example, if shader compilation fails, then a list of compilation errors is cached.
class ExceptionShaderValidationError : public Exception
{
  public:
    ExceptionShaderValidationError(const string& msg, const ShaderValidationErrorList& errorList) :
        Exception(msg),
        _errorLog(errorList)
    {
    }

    ExceptionShaderValidationError(const ExceptionShaderValidationError& e) :
        Exception(e)
    {
        _errorLog = e._errorLog;
    }

    ExceptionShaderValidationError& operator=(const ExceptionShaderValidationError& e)         
    {
        Exception::operator=(e);
        _errorLog = e._errorLog;
        return *this;
    }

    virtual ~ExceptionShaderValidationError() throw()
    {
    }

    const ShaderValidationErrorList& errorLog() const
    {
        return _errorLog;
    }

  private:
    ShaderValidationErrorList _errorLog;
};

} // namespace MaterialX

#endif
