#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// Utility methods

#include <MaterialXCore/Library.h>

namespace MaterialX
{

class SgNodeGraph;

/// Reads the contents of a file into the given string
bool readFile(const string& filename, string& content);

/// Returns the extension of the given filename
string getFileExtension(const string& filename);

/// Print the given nodegraph in a DOT language syntax.  This can be used to
/// visualise the graph using GraphViz (http://www.graphviz.org).
string printGraphDot(const SgNodeGraph& graph);

} // namespace MaterialX

#endif
