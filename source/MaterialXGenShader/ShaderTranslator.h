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
    static ShaderTranslatorPtr create(ConstDocumentPtr doc)
    {
        return ShaderTranslatorPtr(new ShaderTranslator(doc));
    }

    /// Translates shaderRef to the destShader shading model
    void translateShader(ShaderRefPtr shaderRef, string destShader);

    /// Translates all the materials to the destShader shading model if translation exists.
    static bool translateAllMaterials(DocumentPtr doc, string destShader);

    /// Returns set of all the available potential translations
    StringSet getAvailableTranslations(string start)
    {
        return _shadingTranslations[start];
    }

  protected:
    ShaderTranslator(ConstDocumentPtr doc);

    /// Reads shading translation nodes from the document
    void loadShadingTranslations();

    /// Connects translation node inputs from the original shaderRef
    void connectToTranslationInputs(ShaderRefPtr shaderRef);

    /// Copies translation nodegraph upstream node dependencies over to the working nodegraph.
    /// Used when normals need to be baked in tangent space but shaderref expects normals to 
    /// be in world space.
    void insertUpstreamDependencies(OutputPtr translatedOutput, OutputPtr ngOutput);

    /// Connects translation node outputs to finalize shaderRef translation
    void connectTranslationOutputs(ShaderRefPtr shaderRef);

    /// Set that stores all the translation nodes in the document library
    StringSet _translationNodes;

    /// Map that stores all the potential destination shading models for given shading model
    std::unordered_map<string, StringSet> _shadingTranslations;

    /// Saved document that contains library for shading translation
    ConstDocumentPtr _doc;

    /// The inserted translation node
    NodePtr _translationNode;

    /// The nodegraph where translation node will be inserted
    NodeGraphPtr _graph;
};

} // namespace MaterialX

#endif
