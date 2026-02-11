//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// @file
/// Shader generation utility methods

#include <MaterialXGenShader/Export.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <unordered_set>
#include <vector>
#include <ostream>

MATERIALX_NAMESPACE_BEGIN

class ShaderGenerator;

/// Gaussian kernel weights for different kernel sizes.
/// Shared between the ConvolutionNode implementation and MaterialXRender::Image
extern MX_GENSHADER_API const std::array<float, 3> GAUSSIAN_KERNEL_3;
extern MX_GENSHADER_API const std::array<float, 5> GAUSSIAN_KERNEL_5;
extern MX_GENSHADER_API const std::array<float, 7> GAUSSIAN_KERNEL_7;

/// Information about a transparency-relevant input that deviates from its opaque default.
struct TransparencyInput
{
    string name;       ///< Input name (e.g. "opacity", "transmission", "alpha")
    string valueType;  ///< Value type (e.g. "float", "color3")
    string value;      ///< Current value as string, or "[connected]" if driven by a node
    float opaqueAt;    ///< The value at which this input is considered opaque

    /// Serialize this input to a JSON object string.
    MX_GENSHADER_API string toJson() const;
};

/// Returns true if the given element is a surface shader with the potential
/// of being transparent. This can be used by HW shader generators to determine
/// if a shader will require transparency handling.
///
/// Note: This function will check some common cases for how a surface
/// shader can be transparent. It is not covering all possible cases for
/// how transparency can be done and target applications might need to do
/// additional checks to track transparency correctly. For example, custom
/// surface shader nodes implemented in source code will not be tracked by this
/// function and transparency for such nodes must be tracked separately by the
/// target application.
///
/// @param outInputs If non-null, populated with the transparency-relevant inputs
///                  that caused the surface to be classified as transparent.
///
MX_GENSHADER_API bool isTransparentSurface(ElementPtr element, const string& target = EMPTY_STRING,
                                           vector<TransparencyInput>* outInputs = nullptr);

/// Resolve the surface shader node from a renderable element.
/// Handles material nodes (via getShaderNodes), direct shader nodes,
/// and output elements connected to shader nodes.
/// @return The resolved shader node, or nullptr if not found.
MX_GENSHADER_API NodePtr resolveShaderNode(ElementPtr element, const string& target = EMPTY_STRING);

/// Returns true if the given element is an unlit surface shader
/// (i.e. uses the surface_unlit node, which has no lighting and uses
/// a scalar emission+transmission model).
MX_GENSHADER_API bool isUnlitSurface(ElementPtr element, const string& target = EMPTY_STRING);

/// Maps a value to a four channel color if it is of the appropriate type.
/// Supported types include float, Vector2, Vector3, Vector4,
/// and Color4. If not mapping is possible the color value is
/// set to opaque black.
MX_GENSHADER_API void mapValueToColor(ConstValuePtr value, Color4& color);

/// Return whether a nodedef requires an implementation
MX_GENSHADER_API bool requiresImplementation(ConstNodeDefPtr nodeDef);

/// Determine if a given element requires shading / lighting for rendering
MX_GENSHADER_API bool elementRequiresShading(ConstTypedElementPtr element);

/// Find all renderable material nodes in the given document.
/// @param doc Document to examine
/// @return A vector of renderable material nodes.
MX_GENSHADER_API vector<TypedElementPtr> findRenderableMaterialNodes(ConstDocumentPtr doc);

/// Find all renderable elements in the given document, including material nodes if present,
/// or graph outputs of renderable types if no material nodes are found.
/// @param doc Document to examine
/// @return A vector of renderable elements
MX_GENSHADER_API vector<TypedElementPtr> findRenderableElements(ConstDocumentPtr doc);

/// Analysis of a single renderable element's shader generation decisions.
struct RenderableAnalysis
{
    string path;           ///< Element name path in the document
    string file;           ///< Source file URI
    string type;           ///< Element type (e.g. "material", "surfaceshader")
    string shaderNode;     ///< Shader node category (e.g. "gltf_pbr", "standard_surface", "open_pbr_surface")
    string shaderNodeDef;  ///< NodeDef name (e.g. "ND_gltf_pbr_surfaceshader")
    string shaderNodeDefVersion;  ///< NodeDef version (e.g. "2.0.1" for gltf_pbr, "2.6" for UsdPreviewSurface)

    bool transparency = false;   ///< Would HW shader gen enable hwTransparency?
    bool displacement = false;   ///< Is this a displacement shader?
    bool isUnlit = false;        ///< True when shader is surface_unlit (no lighting, scalar emission model)

    /// Alpha mode inferred from the shader graph structure.
    /// "opaque"  - no transparency handling needed (all transparency inputs at opaque defaults)
    /// "mask"    - binary cutout (e.g. gltf_pbr alpha_mode=1 with alpha_cutoff)
    /// "blend"   - smooth alpha blending (e.g. gltf_pbr alpha_mode=2, or non-default opacity)
    string alphaMode = "opaque";

    /// Alpha cutoff threshold, meaningful when alphaMode == "mask".
    float alphaCutoff = 0.0f;

    /// List of transparency-relevant inputs that deviate from opaque defaults.
    vector<TransparencyInput> transparencyInputs;

    /// Serialize this analysis to a JSON object string.
    MX_GENSHADER_API string toJson() const;
};

/// Find renderable elements and return analysis information for each,
/// including transparency mode detection and shader generation decisions.
MX_GENSHADER_API void getRenderableAnalysis(ConstDocumentPtr doc,
                                            const string& target,
                                            vector<RenderableAnalysis>& out);

/// Run the material report pipeline: load the document, validate it,
/// analyze renderables, and write the result to the given output stream.
/// @param materialFilename Path to the MTLX document
/// @param searchPath Search path for resolving libraries and references
/// @param reportFormat Output format: "json" or "text"
/// @param out Output stream (typically std::cerr)
/// @return 0 on success (valid document), 1 on validation failure or load error
MX_GENSHADER_API int runMaterialReport(const string& materialFilename,
                                       const FileSearchPath& searchPath,
                                       const string& reportFormat,
                                       std::ostream& out);

/// Given a node input, return the corresponding input within its matching nodedef.
/// The optional target string can be used to guide the selection of nodedef declarations.
MX_GENSHADER_API InputPtr getNodeDefInput(InputPtr nodeInput, const string& target);

/// Perform token substitutions on the given source string, using the given substitution map.
/// Tokens are required to start with '$' and can only consist of alphanumeric characters.
/// The full token name, including '$' and all following alphanumeric character, will be replaced
/// by the corresponding string in the substitution map, if the token exists in the map.
MX_GENSHADER_API void tokenSubstitution(const StringMap& substitutions, string& source);

/// Compute the UDIM coordinates for a set of UDIM identifiers
/// @return List of UDIM coordinates
MX_GENSHADER_API vector<Vector2> getUdimCoordinates(const StringVec& udimIdentifiers);

/// Get the UV scale and offset to transform uv coordinates from UDIM uv space to
/// 0..1 space.
MX_GENSHADER_API void getUdimScaleAndOffset(const vector<Vector2>& udimCoordinates, Vector2& scaleUV, Vector2& offsetUV);

/// Determine whether the given output is directly connected to a node that
/// generates world-space coordinates (e.g. the "normalmap" node).
/// @param output Output to check
/// @return Return the node if found.
MX_GENSHADER_API NodePtr connectsToWorldSpaceNode(OutputPtr output);

/// Returns true if there is are any value elements with a given set of attributes either on the
/// starting node or any graph upsstream of that node.
/// @param output Starting node
/// @param attributes Attributes to test for
MX_GENSHADER_API bool hasElementAttributes(OutputPtr output, const StringVec& attributes);

//
// These are deprecated wrappers for older versions of the function interfaces in this module.
// Clients using these interfaces should update them to the latest API.
//
[[deprecated]] MX_GENSHADER_API void findRenderableMaterialNodes(ConstDocumentPtr doc, vector<TypedElementPtr>& elements, bool, std::unordered_set<ElementPtr>&);
[[deprecated]] MX_GENSHADER_API void findRenderableElements(ConstDocumentPtr doc, vector<TypedElementPtr>& elements, bool includeReferencedGraphs = false);

MATERIALX_NAMESPACE_END

#endif
