//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_GENCONTEXTCREATE_H
#define MATERIALX_GENSHADER2_GENCONTEXTCREATE_H

/// @file
/// Phase 2 entry point for shader graph creation via IShaderSource.
///
/// GenContextCreate owns an IShaderSource and a GenContext, and provides
/// buildGraph() to construct a ShaderGraph equivalent to what
/// ShaderGraph::create() would produce from the same material.
///
/// Phase 3 will add a buildShader() method that drives code emission through
/// a GenContextEmit, completing the two-phase pipeline.

#include <MaterialXGenShader2/Export.h>
#include <MaterialXGenShader2/IShaderSource.h>
#include <MaterialXGenShader2/ShaderGraph2.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <memory>

MATERIALX_NAMESPACE_BEGIN

/// @class GenContextCreate
/// Owns an IShaderSource and drives ShaderGraphBuilder to produce a ShaderGraph.
///
/// Usage:
/// @code
///   auto adapter = std::make_unique<MxElementAdapter>(doc, element);
///   GenContextCreate ctx(GlslShaderGenerator::create(), std::move(adapter));
///   ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
///
///   ShaderGraph2Ptr graph = ctx.buildGraph("myShader");
/// @endcode
class MX_GENSHADER2_API GenContextCreate
{
  public:
    /// Constructs a GenContextCreate with the given generator and source.
    /// @param generator  The shader generator to use (e.g. GlslShaderGenerator::create()).
    /// @param source     The abstract data source; ownership is transferred here.
    GenContextCreate(ShaderGeneratorPtr generator, std::unique_ptr<IShaderSource> source);

    /// Returns a mutable reference to the underlying GenContext.
    /// Use this to register search paths, set options, etc.
    GenContext& getGenContext() { return _genContext; }

    /// Returns the IShaderSource provided at construction.
    const IShaderSource& getSource() const { return *_source; }

    /// Constructs and returns a fully-finalized ShaderGraph built from the
    /// IShaderSource, equivalent to what ShaderGraph::create() produces from
    /// the same material element.
    ShaderGraph2Ptr buildGraph(const string& name);

    /// Builds the ShaderGraph and emits shader source code, returning the
    /// completed Shader equivalent to ShaderGenerator::generate().
    ///
    /// For MX-backed sources the root element is resolved via getMxDocument()
    /// and forwarded to the generator's existing generate() method.
    /// @see TODO in GenContextCreate.cpp for the path to full MX independence.
    ShaderPtr buildShader(const string& name);

  private:
    std::unique_ptr<IShaderSource> _source;
    GenContext                     _genContext;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GENSHADER2_GENCONTEXTCREATE_H
