//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERTRANSLATOR_H
#define MATERIALX_SHADERTRANSLATOR_H

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

/// A shared pointer to a TextureBaker
using ShaderTranslatorPtr = shared_ptr<class ShaderTranslator>;

/// @class ShaderTranslator
class ShaderTranslator
{
  public:
    static ShaderTranslatorPtr create(ConstDocumentPtr doc)
    {
        return ShaderTranslatorPtr(new ShaderTranslator(doc));
    }

    StringSet getAvailableTranslations(string start)
    {
        return _shadingTranslations[start];
    }

    bool translateShader(ShaderRefPtr shaderRef, string destShader);

  protected:
    /// Constructor.
    ShaderTranslator(ConstDocumentPtr doc);
    void loadShadingTranslations();

    StringSet _translationNodes;
    std::unordered_map<string, StringSet> _shadingTranslations;
    ConstDocumentPtr _doc;
    NodePtr _translationNode;
};

} // namespace MaterialX

#endif
