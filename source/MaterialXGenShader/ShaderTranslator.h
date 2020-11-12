//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERTRANSLATOR_H
#define MATERIALX_SHADERTRANSLATOR_H

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

/// A shared pointer to a ShaderTranslator
using ShaderTranslatorPtr = shared_ptr<class ShaderTranslator>;

/// @class ShaderTranslator
class ShaderTranslator
{
  public:
    static ShaderTranslatorPtr create()
    {
        return ShaderTranslatorPtr(new ShaderTranslator());
    }

    /// Translate a shader reference to the destination shading model.
    void translateShader(ShaderRefPtr shaderRef, string destShader);

    /// Translate each material in the input document to the destination
    /// shading model.
    void translateAllMaterials(DocumentPtr doc, string destShader);

  protected:
    ShaderTranslator();

    // Connect translation node inputs from the original shaderRef
    void connectTranslationInputs(ShaderRefPtr shaderRef, NodeDefPtr translationNodeDef);

    // Connect translation node outputs to finalize shaderref translation
    void connectTranslationOutputs(ShaderRefPtr shaderRef);

  protected:
    NodeGraphPtr _graph;
    NodePtr _translationNode;
};

} // namespace MaterialX

#endif
