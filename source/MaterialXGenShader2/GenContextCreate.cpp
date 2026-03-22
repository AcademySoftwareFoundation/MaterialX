//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader2/GenContextCreate.h>
#include <MaterialXGenShader2/ShaderGraphBuilder.h>

#include <MaterialXGenShader/Exception.h>

MATERIALX_NAMESPACE_BEGIN

GenContextCreate::GenContextCreate(ShaderGeneratorPtr generator,
                                   std::unique_ptr<IShaderSource> source)
    : _source(std::move(source))
    , _genContext(generator)
{
}

ShaderGraph2Ptr GenContextCreate::buildGraph(const string& name)
{
    ShaderGraphBuilder builder(*_source, _genContext);
    return builder.build(name);
}

ShaderPtr GenContextCreate::buildShader(const string& name)
{
    // Resolve the MaterialX root element via the MX compatibility bridge.
    // TODO: once emission is decoupled from ElementPtr, drive this step
    //       directly from the IShaderSource + pre-built ShaderGraph2.
    ConstDocumentPtr doc = _source->getMxDocument();
    if (!doc)
    {
        throw ExceptionShaderGenError("GenContextCreate::buildShader: getMxDocument() returned nullptr. "
                                      "Non-MX backends must provide their own buildShader() implementation.");
    }

    DataHandle root = _source->getRootElement();
    ElementPtr mxElem = doc->getDescendant(_source->getElementPath(root));
    if (!mxElem)
    {
        throw ExceptionShaderGenError("GenContextCreate::buildShader: could not resolve root element '" +
                                      _source->getElementPath(root) + "' in document.");
    }

    return _genContext.getShaderGenerator().generate(name, mxElem, _genContext);
}

MATERIALX_NAMESPACE_END
