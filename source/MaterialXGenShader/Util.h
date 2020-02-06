//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// @file
/// Shader generation utility methods

#include <MaterialXGenShader/Library.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

#include <unordered_set>

namespace MaterialX
{

class ShaderGenerator;

/// Removes the extension from the provided filename
string removeExtension(const string& filename);

/// Reads the contents of a file into the given string
bool readFile(const string& filename, string& content);

/// Scans for all documents under a root path and returns documents which can be loaded
void loadDocuments(const FilePath& rootPath, const FileSearchPath& searchPath, const StringSet& skipFiles,
                   const StringSet& includeFiles, vector<DocumentPtr>& documents, StringVec& documentsPaths,
                   const XmlReadOptions& readOptions, StringVec& errors);

/// Load a given MaterialX library into a document
void loadLibrary(const FilePath& file, DocumentPtr doc, const FileSearchPath* searchPath = nullptr);

/// Load all MaterialX files with given library names in given search paths.
/// Note that all library files will have a URI set on them.
StringVec loadLibraries(const StringVec& libraryNames,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles = nullptr);

/// Load all MaterialX files with given library names in a given path.
StringVec loadLibraries(const StringVec& libraryNames,
                        const FilePath& filePath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles = nullptr);

/// Returns true if the given element is a surface shader with the potential
/// of beeing transparent. This can be used by HW shader generators to determine
/// if a shader will require transparency handling.
///
/// Note: This function will check some common cases for how a surface
/// shader can be transparent. It is not covering all possible cases for
/// how transparency can be done and target applications might need to do
/// additional checks to track transparency correctly. For example, custom
/// surface shader nodes implemented in source code will not be tracked by this
/// function and transprency for such nodes must be tracked separately by the
/// target application.
///
bool isTransparentSurface(ElementPtr element, const ShaderGenerator& shadergen);

/// Maps a value to a four channel color if it is of the appropriate type.
/// Supported types include float, Vector2, Vector3, Vector4,
/// Color2, and Color4. Note that for Color2 the second channel
/// maps to alpha. If not mapping is possible the color value is
/// set to opaque black.
void mapValueToColor(ConstValuePtr value, Color4& color);

/// Return whether a nodedef requires an implementation
bool requiresImplementation(ConstNodeDefPtr nodeDef);

/// Determine if a given element requires shading / lighting for rendering
bool elementRequiresShading(ConstTypedElementPtr element);

/// Return a vector of all Shader nodes for a Material node.
/// @param materialNode Node to example
/// @param shaderType Type of shader to return. If an empty string is specified then
///        all shader node types are returned. The default argument value is an empetyr
///        string which inidates to include shaders which match any type.
/// @param target Target attribute of shader to return. The default argument value is an empty string
///        which indicates to include shaders which match any target.
vector<NodePtr> getShaderNodes(const NodePtr materialNode, 
                               const string& shaderType = EMPTY_STRING,
                               const string& target = EMPTY_STRING);

/// Return a vector of all MaterialAssign elements that bind this material node
/// to the given geometry string
/// @param materialNode Node to examine
/// @param geom The geometry for which material bindings should be returned.
///    By default, this argument is the universal geometry string "/", and
///    all material bindings are returned.
/// @return Vector of MaterialAssign elements
vector<MaterialAssignPtr> getGeometryBindings(NodePtr materialNode, const string& geom);

/// Find any material node elements which are renderable (have input shaders)
/// @param doc Document to examine
/// @param elements List of renderable elements (returned)
/// @param includeRefencedGraphs Whether to check for outputs on referenced graphs
/// @param graphOutputs List of outputs examined. Graph outputs are added if they do
///     not already exist
void findRenderableMaterialNodes(ConstDocumentPtr doc, 
                                 vector<TypedElementPtr>& elements, 
                                 bool includeReferencedGraphs,
                                 std::unordered_set<OutputPtr> &processedOutputs);

/// Find any shaderrefs elements which are renderable
/// @param doc Document to examine
/// @param elements List of renderable elements (returned)
/// @param includeRefencedGraphs Whether to check for outputs on referenced graphs
/// @param graphOutputs List of outputs examined. Graph outputs are added if they do
///     not already exist
void findRenderableShaderRefs(ConstDocumentPtr doc,
                              vector<TypedElementPtr>& elements, 
                              bool includeReferencedGraphs,
                              std::unordered_set<OutputPtr> &processedOutputs);

/// Find any elements which may be renderable from within a document.
/// This includes all outputs on node graphs and shader references which are not
/// part of any included library. Light shaders are not considered to be renderable.
/// The option to include node graphs referened by shader references is disabled by default.
void findRenderableElements(ConstDocumentPtr doc, vector<TypedElementPtr>& elements,
                            bool includeReferencedGraphs = false);

/// Given a path to a element, find the corresponding element with the same name
/// on an associated nodedef if it exists. A target string should be provided
/// if the path is to a Node as definitions for Nodes can be target specific.
ValueElementPtr findNodeDefChild(const string& path, DocumentPtr doc, const string& target);

/// Perform token substitutions on the given source string, using the given substituation map.
/// Tokens are required to start with '$' and can only consist of alphanumeric characters.
/// The full token name, including '$' and all following alphanumeric character, will be replaced
/// by the corresponding string in the substitution map, if the token exists in the map.
void tokenSubstitution(const StringMap& substitutions, string& source);

/// Compute the UDIM coordinates for a set of UDIM identifiers
/// @return List of UDIM coordinates
vector<Vector2> getUdimCoordinates(const StringVec& udimIdentifiers);

/// Get the UV scale and offset to transform uv coordinates from UDIM uv space to
/// 0..1 space.
void getUdimScaleAndOffset(const vector<Vector2>& udimCoordinates, Vector2& scaleUV, Vector2& offsetUV);

} // namespace MaterialX

#endif
