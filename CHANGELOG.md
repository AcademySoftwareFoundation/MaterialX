# Change Log

## [1.37.2] - Development

- ILM : General
  - Load standard libraries for MaterialXView regardless of build folder location (#922)
- ADSK : General
  - Add definition searching for shader nodes for <material> instances (#919)
- ILM : Baking
  - Add support for floating-point render targets (#922)
- ADSK : ShaderGen general changes
  - Fix regression with variable naming. (#927)
  - Support for thin-film. (#923)
  - Fix handling of included functions for convolution nodes (#909)
  - Support for vertical layering through a layer operator node. (#892)
- ADSK : OSL / GLSL ShaderGen layout support
  - Fix GLSL regression with non-FIS environment sampling. (#930)
  - Face forward GLSL fix to sheen brdf (#926)
  - Fix regression in OSL generation. (#928)
  - Fix regressions with layout bindings. (#906)
  - Improve layering in OSL (#901)
  - Fix regression with search paths in MaterialXView (#894)
  - Test case and python bindings (#898)
  - Enable GlslShaderGenerator to add `layout` in generated code. (#896) 
  - Use mod() vs fmod() logic for OSL and MDL codegen (#871). Only for 1.38
- ADSK : Render general changes
  - Disable C++20 Warning for TinyExrLoader (#922)
  - Fix binding issue if empty filename given to bind with. Remove skipping binding in that condition. (#915)
  - Enhance upstream shader search for <surfacematerial> node. Enhance MaterialXView shader failure handling. (#911)
- ADSK Definition publishing support:
  - Makes it so that on read when an interface name has a matching node (#905)
  - Initial Node Definition Version support code. (#899)
  - Fix input traversal so "output" attribute is optional if nodename/nodegraph specified. (#882)
  - Add in nodedef from nodegraph utilities (#848)
- ADSK : Javascript Bindings
  - Fix JavaScript security issues from the Codacy scan (#925)
  - Part I : Basic setup MaterialX and parts of MaterialXFormat modules. CI on Travis. (#879)

## [1.37.1] - 2020-06-04

### Added
- Added command-line options for mesh, light, and camera transforms to the viewer.
- Added command-line options for screen dimensions and background color to the viewer.
- Added a Light Rotation slider to the Advanced Settings panel of the viewer.
- Added utility methods Backdrop\:\:setContainsElements and Backdrop\:\:getContainsElements.
- Added backwards compatibility for OpenImageIO 1.x.
- Added support for GCC 10.

### Changed
- Improved energy conservation and preservation computations in generated GLSL.
- Upgraded Smith masking-shadowing to height-correlated form in generated GLSL.
- Improved the robustness of tangent frame computations in MaterialXRender.
- Renamed Backdrop\:\:setContains and getContains to Backdrop\:\:setContainsString and getContainsString for consistency.
- Added `applyFutureUpdates` to XmlReadOptions. Updates current applied include:
  - Upgrade from Material Element to Material Node
  - Fix for arguments of from `atan2(x,y)` to `atan2(y,x)`. (in1 and in2 arguments are swapped)

### Fixed
- Fixed the GLSL implementation of Burley diffuse for punctual lights.
- Fixed the upgrade path for compare nodes in v1.36 documents.

## [1.37.0] - 2020-03-20

Updated the MaterialX library to the v1.37 specification.  See the [v1.37 changelist](http://www.materialx.org/assets/MaterialX.v1.37REV2.Changelist.pdf) for full details.

### Added
- Added a Shadow Map option to the viewer, supported by shadowing functionality in GLSL code generation.
- Added support for the 'uisoftmin', 'uisoftmax', and 'uistep' attributes, updating Autodesk Standard Surface to leverage these features.
- Added support for LookGroup elements.
- Added support for Clang 9.

### Changed
- Updated the set of standard nodes to match the v1.37 specification.
- Unified the rules for NodeDef outputs, with all NodeDefs defining their output set through Output child elements rather than 'type' attributes.
- Replaced GeomAttr elements with GeomProp elements.
- Replaced backdrop nodes with Backdrop elements.
- Aligned Matrix33 and Matrix44 with the row-vector convention, for improved consistency with Imath, USD, and other libraries.
- Updated the stb_image library to version 2.23.

## [1.36.5] - 2020-01-11

### Added
- Added a Load Environment option to the viewer, allowing arbitrary latitude-longitude HDR environments to be loaded at run-time.
- Added an initial TextureBaker class, supporting baking of procedural content to textures.
- Added initial support for units, including the MaterialX\:\:Unit, MaterialX\:\:UnitDef, and MaterialX\:\:UnitTypeDef classes.
- Added support for unit conversion in shader code generation.
- Added support for Visual Studio 2019.

### Changed
- Updated Autodesk Standard Surface to the latest interface and graph.
- Updated the PyBind11 library to version 2.4.3.

## [1.36.4] - 2019-09-26

### Added
- Added a Save Material option to the viewer.
- Added property accessors to PropertyAssign and PropertySetAssign
- Added Python bindings for TypeDesc and array-type Values.
- Added Python functions getTypeString, getValueString, and createValueFromStrings.
- Added support for GCC 9 and Clang 8.

### Changed
- Updated the interface of readFromXmlFile and writeToXmlFile to support FilePath and FileSearchPath arguments.
- Extended Python bindings for FilePath and FileSearchPath.

### Removed
- Deprecated Python functions typeToName, valueToString, and stringToValue.
- Removed deprecated Python functions objectToString and stringToObject.

### Fixed
- Fixed the OSL implementation of roughness_dual.

## [1.36.3] - 2019-08-02

Merged shader code generation and physically-based shading nodes from Autodesk's ShaderX extensions.  Added a default MaterialX viewer based on GLSL shader generation.

### Added
- Added the MaterialXGenShader library, supporting shader code generation in GLSL and OSL.
- Added the MaterialXRender library, providing helper functionality for rendering MaterialX content.
- Added the MaterialXView library, providing a default MaterialX viewer.
- Added the physically-based shading node library (libraries/pbrlib).
- Added a root-level 'cmake' folder, including a standard FindMaterialX module.
- Added a root-level 'resources' folder, including example materials and meshes.
- Added documents for the 1.37 specification.

### Changed
- Moved the MaterialX data libraries from 'documents/Libraries' to 'libraries'.
- Updated MaterialX node definitions to the 1.37 specification.
- Updated the PyBind11 library to version 2.2.4.

### Removed
- Removed customizations of PyBind11 to support Python 2.6.  Only Python versions 2.7 and 3.x are now supported.

## [1.36.2] - 2019-03-05

### Added
- Added support for 'nodedef' attributes on MaterialX\:\:Node, integrating this usage into GraphElement\:\:addNodeInstance.
- Added the MaterialX\:\:GeomPropDef class for geometric input declarations.
- Added the Document\:\:getGeomAttrValue method.
- Added the ValueElement\:\:getResolvedValue method.
- Added support for the MATERIALX_SEARCH_PATH environment variable.
- Added support for GCC 8 and Clang 7.

### Changed
- Added callbacks Observer\:\:onCopyContent and Observer\:\:onClearContent, and removed callback Observer::onInitialize.
- Moved the standard document library to the 'documents/Libraries/stdlib' folder.

## [1.36.1] - 2018-12-18

### Added
- Added support for interface tokens, including the MaterialX\:\:BindToken class and '[TOKEN]' syntax in filenames.
- Added support for Clang 6.

### Changed
- Updated geometry token syntax from '%TOKEN' to '\<TOKEN\>'.
- Replaced readXIncludes boolean with a readXIncludeFunction callback in the XmlReadOptions structure.
- Combined individual options into an XmlWriteOptions argument for the XML write functions.
- Extended functionality of the vector and matrix classes.
- Updated the PyBind11 library to version 2.2.3.
- Updated the PugiXML library to version 1.9.

### Fixed
- Fixed graph implementations of range, extract, tiledimage, and ramp4 nodes.

## [1.36.0] - 2018-07-23

Updated the MaterialX library to the v1.36 specification.  See the [v1.36 changelist](http://www.materialx.org/assets/MaterialX.v1.36.Changelist.pdf) for full details.

### Added
- Added support for Element namespaces.
- Added support for NodeDef inheritance.
- Added support for root-level node elements.
- Added support for inheritance attributes on MaterialX\:\:Material and MaterialX\:\:Look.
- Added support for include and exclude attributes on MaterialX\:\:Collection.
- Added the MaterialX\:\:Token class for string substitutions.
- Added the MaterialX\:\:Variant, MaterialX\:\:VariantSet, and MaterialX\:\:VariantAssign classes.
- Added the MaterialX\:\:GeomPath class for geometry name comparisons.
- Added the Collection\:\:matchesGeomString method, for testing matches between collections and geometries.
- Added the Material\:\:getGeometryBindings method, for finding the bindings of a material to specific geometries.

### Removed
- Removed the MaterialX\:\:MaterialInherit and MaterialX\:\:LookInherit classes.
- Removed the MaterialX\:\:CollectionAdd and MaterialX\:\:CollectionRemove classes.
- Removed the MaterialX\:\:Override class and support for public names.
- Removed the 'channels' attribute from MaterialX\:\:InterfaceElement.
- Removed the Material::getReferencingMaterialAssigns method (deprecated in Python).

## [1.35.5] - 2018-05-07

### Added
- Added material inheritance support to graph traversal and the high-level Material API.
- Added Material methods getActiveShaderRefs and getActiveOverrides.
- Added PropertySet methods setPropertyValue and getPropertyValue.
- Added Element methods setInheritsFrom, getInheritsFrom, traverseInheritance, hasInheritanceCycle, and getDescendant.
- Added function templates MaterialX\:\:fromValueString and MaterialX\:\:toValueString.
- Added math functionality to the vector and matrix classes.
- Added support for Visual Studio 2017, GCC 7, and Clang 5.

### Changed
- Renamed Matrix3x3 to Matrix33 and Matrix4x4 to Matrix44.
- Renamed VectorN\:\:length to VectorN\:\:numElements.
- Updated the PyBind11 library to version 2.2.1.

## [1.35.4] - 2017-12-18

### Added
- Added high-level Material API, including getPrimaryShaderParameters, getPrimaryShaderInputs, getBoundGeomStrings, and getBoundGeomCollections.
- Added methods ValueElement\:\:getBoundValue and ValueElement\:\:getDefaultValue.
- Added support for multi-output nodes.
- Added support for TypeDef members.
- Added StringResolver class, for applying substring modifiers to data values.
- Added example interfaces for the Disney BRDF, Disney BSDF, and alSurface shaders.

### Changed
- Renamed method Material\:\:getReferencedShaderDefs to Material\:\:getShaderNodeDefs.
- Renamed method ShaderRef\:\:getReferencedShaderDef to ShaderRef\:\:getNodeDef.
- Renamed method Node\:\:getReferencedNodeDef to Node\:\:getNodeDef.
- Added a 'string' suffix to all accessors for 'node', 'nodedef', and 'collection' strings.
- Combined individual booleans into an XmlReadOptions argument for the XML read functions.

### Removed
- Removed method Document\:\:applyStringSubstitutions (deprecated in Python).
- Removed method InterfaceElement\:\:getParameterValueString (deprecated in Python).

## [1.35.3] - 2017-10-11

### Added
- Added support for Python 3.
- Added support for standard TypeDef attributes.
- Added support for values of type 'stringarray'.
- Added method Element\:\:setName.
- Extended Python bindings for Document, NodeGraph, MaterialAssign, and Collection.

### Changed
- Modified NodeGraph\:\:topologicalSort to return elements in a more intuitive top-down order, with upstream elements preceding downstream elements.
- Removed special cases for string return values in MaterialX Python, with all strings now returned as 'unicode' in Python 2 and 'str' in Python 3.
- Updated OSL reference implementations.

### Fixed
- Fixed handling of empty names in Element\:\:addChildOfCategory.
- Fixed an edge case in Document\:\:upgradeVersion.

## [1.35.2] - 2017-07-03

### Added
- Added OSL source files for the standard nodes.
- Added example document 'PostShaderComposite.mtlx'.
- Added method MaterialX\:\:prependXInclude.

### Changed
- Argument 'writeXIncludes' defaults to true in MaterialX\:\:writeToXmlStream and MaterialX\:\:writeToXmlString.

### Fixed
- Fixed handling of BindInput elements with missing connections.

## [1.35.1] - 2017-06-23

### Added
- Added a 'viewercollection' attribute to MaterialX\:\:Visibility.
- Added Python support for visibility and source URI methods.

### Changed
- Changed naming convention from 'ColorSpace' to 'ColorManagement' in Document methods.
- Split library document 'mx_stdlib.mtlx' into 'mx_stdlib_defs.mtlx' and 'mx_stdlib_osl_impl.mtlx'.

## [1.35.0] - 2017-06-20

Updated the MaterialX library to the v1.35 specification.  See the [v1.35 changelist](http://www.materialx.org/assets/MaterialX.v1.35.Changelist.pdf) for full details.

### Added
- Added the MaterialX\:\:Visibility class.
- Added 'file', 'function', and 'language' attributes to MaterialX\:\:Implementation.
- Added 'node' and 'nodedef' attributes to MaterialX\:\:ShaderRef.  In v1.35, these attributes define which NodeDef is referenced by a ShaderRef.
- Added a 'material' attribute to MaterialX\:\:MaterialAssign.  In v1.35, this attribute defines which Material is referenced by a MaterialAssign.

### Changed
- Removed the MaterialX\:\:LightAssign and MaterialX\:\:Light classes.  In v1.35, this functionality is now handled by the MaterialX\:\:Visibility class.
- Removed the 'default' attribute from MaterialX\:\:ValueElement.  In v1.35, this functionality is now handled by the 'value' attribute.
- Replaced the 'matrix' type with 'matrix33' and 'matrix44', and replaced the MaterialX\:\:Matrix16 class with MaterialX\:\:Matrix3x3 and MaterialX\:\:Matrix4x4.
- Renamed Material\:\:getMaterialAssigns to Material\:\:getReferencingMaterialAssigns.
- Changed the argument type for MaterialAssign\:\:setExclusive and MaterialAssign\:\:getExclusive to boolean.

## [1.34.4] - 2017-06-09

### Added
- Added support for graph-based implementations of nodes.
- Added support for subtree/subgraph pruning in traversals.
- Added NodeGraph\:\:topologicalSort and MaterialX\:\:printGraphDot methods.
- Added a File module to MaterialXFormat and MaterialXTest.

### Changed
- Extended NodeGraph::flattenSubgraphs to support subgraph recursion.
- Added a searchPath argument to MaterialX\:\:readFromXmlFile.

### Fixed
- Fixed an issue where connecting elements were not returned in graph traversal edges.

## [1.34.3] - 2017-05-16

### Added
- Added support for document validation, including the Document\:\:validate and Element\:\:validate methods.
- Added helper methods ValueElement\:\:getResolvedValueString and Element\:\:getNamePath.
- Added standard library document.
