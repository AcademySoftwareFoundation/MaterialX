//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GENSHADERLIBRARY_H
#define MATERIALX_GENSHADERLIBRARY_H

/// @file
/// Library-wide includes and types.  This file should be the first include for
/// any public header in the MaterialXGenShader library.

#include <MaterialXCore/Library.h>

namespace MaterialX
{

template<class T>
using CreatorFunction = shared_ptr<T>(*)();

/// @class @ExceptionShaderGenError
/// An exception that is thrown when shader generation fails.
class ExceptionShaderGenError : public Exception
{
public:
    ExceptionShaderGenError(const string& msg) :
        Exception(msg)
    {
    }

    ExceptionShaderGenError(const ExceptionShaderGenError& e) :
        Exception(e)
    {
    }

    virtual ~ExceptionShaderGenError() throw()
    {
    }
};

} // namespace MaterialX

#endif
