//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GRAPHEDITOROSLGENERATOR_H
#define MATERIALX_GRAPHEDITOROSLGENERATOR_H

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/GenContext.h>

#include <memory>
#include <string>

namespace mx = MaterialX;

/// @class GraphEditorOslGenerator
/// Shared MaterialX-to-OSL shader generation for the Graph Editor render
/// backends.
///
/// The Cycles and testrender pipelines use the same document traversal,
/// flattening, color management, unit management, and reduced shader
/// interface.  Backend-specific source fixups (for example Cycles' UV and
/// tangent remapping) are applied by the caller after generate().
class GraphEditorOslGenerator
{
  public:
    struct Result
    {
        std::string source;
        std::string outputName;

        explicit operator bool() const
        {
            return !source.empty();
        }
    };

    GraphEditorOslGenerator(mx::DocumentPtr document,
                            const mx::FileSearchPath& searchPath);
    ~GraphEditorOslGenerator();

    /// Update the document and reinitialize the generator context.
    void setDocument(mx::DocumentPtr document);

    /// Generate OSL for the given element, or for the first renderable element
    /// in the document when the element is null.  Returns an empty result when
    /// there is nothing to render or generation fails; getLastError() provides
    /// the failure reason.
    Result generate(mx::TypedElementPtr element);

    const std::string& getLastError() const
    {
        return _lastError;
    }

  private:
    void initContext();

    mx::DocumentPtr _document;
    mx::FileSearchPath _searchPath;
    std::unique_ptr<mx::GenContext> _context;
    std::string _lastError;
};

#endif // MATERIALX_GRAPHEDITOROSLGENERATOR_H
