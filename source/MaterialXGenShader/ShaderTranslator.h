//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERTRANSLATOR_H
#define MATERIALX_SHADERTRANSLATOR_H

/// @file
/// Base shader translator class

#include <MaterialXGenShader/Library.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

/// @class ShaderTranslator
/// Base class for shader translation
class ShaderTranslator
{
  public:
    /// Constructor
    ShaderTranslator();

    /// Destructor
    ~ShaderTranslator() { }


} // namespace MaterialX

#endif // MATERIALX_SHADERTRANSLATOR_H
