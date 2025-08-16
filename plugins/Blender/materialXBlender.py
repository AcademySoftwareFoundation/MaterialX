import os
import sys

# Add the parent directory of this file to sys.path so mtlxutils can be found
#plugin_dir = os.path.dirname(os.path.abspath(__file__))
#parent_dir = os.path.abspath(os.path.join(plugin_dir, ".."))
#if parent_dir not in sys.path:
#    sys.path.insert(0, parent_dir)

# Now import the rest
import math

# Import blender package
import bpy, bmesh
import MaterialX as mx

plugin_env_var = "MATERIALX_PLUGIN_PATHS"
plugin_path = os.environ.get(plugin_env_var)
print(f"Environment variable {plugin_env_var} = {plugin_path}")
if plugin_path and plugin_path not in sys.path:
    print(f"Adding plugin path from environment variable {plugin_env_var}: {plugin_path}")
    sys.path.insert(0, plugin_path)

# MX Utilities
import mtlxutils.mxbase as mxb
import mtlxutils.mxfile as mxf
import mtlxutils.mxnodegraph as mxg
import mtlxutils.mxtraversal as mxt

# To silence Blender output
import io
from contextlib import redirect_stdout, redirect_stderr

class BlenderMaterialXExporter:

    def __init__(self):
        self.writeSingleMaterialFile = True
        self.writeSingleGeomFile = False
        self.exportMeshMaterials = True
        self.outputPath = mx.FilePath()
        self.outputFileName = mx.FilePath()

    def setWriteSingleMaterialFile(self, val):
        "Set whether to write single MaterialX file. Default is True"
        self.writeSingleMaterialFile = val

    def setWriteSingleGeomFile(self, val):
        "Set whether to write single geometry file. Default is False"
        self.writeSingleGeomFile = val

    def setExportMeshMaterials(self, val):
        self.exportMeshMaterials = val

    def setOutputPath(self, val):
        self.outputPath = val

    def setOutputFileName(self, val):
        self.outputFileName = val

    def floatToStr(self, val):
        """ 
        Emit formatted float value to string
        """
        return f"{val:.4g}"

    def createMtlxInput(self, portName, blenderVal, node, nodedef):
        """ 
        Creat input on shader node based on blender value 
        """
        #print('------- add input: ', portName)
        nodedefInput = nodedef.getInput(portName)
        if not nodedefInput:
            return

        valueLen = dict()
        valueLen['color3'] = 3
        valueLen['color4'] = 4
        valueLen['vector2'] = 2
        valueLen['vector3'] = 3
        valueLen['vector4'] = 4
        valueLen['float'] = 1

        portType = nodedefInput.getType()

        # Check Python type to get string values
        # * Use nodedef port type to clamp vector inputs. For example
        # * Blender colors can be 4 float (rgba) in length, but the MaterialX port is only 3 float (rgb).
        # * Blender float can map to a MaterialX vector. The float is replicated as needed
        valueString = ''
        valueLength = valueLen[portType]
        if isinstance(blenderVal, float):
            if valueLength == 1:
                valueString = self.floatToStr(blenderVal)  
            else:
                blenderValString = []
                for i in range(0,valueLength):
                    blenderValString.append(self.floatToStr(blenderVal))
                valueString = ','.join(blenderValString)
        elif isinstance(blenderVal, int):
            valueString = str(blenderVal)
        elif isinstance(blenderVal, str):
            valueString = str(blenderVal)
        else:
            if len(blenderVal) in (2,3,4):
                blenderValString = []
                for i, c in enumerate(blenderVal):
                    if i < valueLength:                                 
                        blenderValString.append(self.floatToStr(blenderVal[i]))
                valueString = ','.join(blenderValString)

        if len(valueString):        
            newInput = node.addInput(portName, portType)
            if newInput:
                newInput.setValueString(valueString) 
        
        return newInput

    def init_node_dictionary(self, targetBSDF):

        # Manual name mapping from Blender BSDF to USD Preview Surface
        PBSDF_USDPS_map = dict()
        PBSDF_USDPS_map['Base Color'] = 'diffuseColor'
        PBSDF_USDPS_map['Specular'] = 'specularColor'
        PBSDF_USDPS_map['IOR'] = 'ior'
        PBSDF_USDPS_map['Clearcoat'] = 'clearcoat'
        PBSDF_USDPS_map['Clearcoat Roughness'] = 'clearcoatRoughness'
        PBSDF_USDPS_map['Metallic'] = 'metallic'
        PBSDF_USDPS_map['Roughness'] = 'roughness'
        PBSDF_USDPS_map['Alpha'] = 'opacity'
        PBSDF_USDPS_map['Emission'] = 'emissiveColor'  
        PBSDF_USDPS_map['Normal'] = 'normal'  

        IMAGE_map = dict()
        NORMALMAP_map = dict()

        # Mapping from Blender nodes to MaterialX node definitions
        SHADER_NODE_map = dict()
        SHADER_NODE_map['BSDF_PRINCIPLED'] =  targetBSDF
        SHADER_NODE_map['TEX_IMAGE'] =  'ND_image_'
        SHADER_NODE_map['NORMAL_MAP'] =  'ND_normalmap'

        SHADER_NODE_INPUTS_map = dict()
        SHADER_NODE_INPUTS_map['BSDF_PRINCIPLED'] = PBSDF_USDPS_map
        SHADER_NODE_INPUTS_map['TEX_IMAGE'] = IMAGE_map
        SHADER_NODE_INPUTS_map['NORMAL_MAP'] = NORMALMAP_map

        return [ SHADER_NODE_map, SHADER_NODE_INPUTS_map ]

    def createMtlxShaderNode(self, doc, name, shaderNodeDefinition, isMaterial):

        mtlxShadername = name + ('_' + 'Shader' if isMaterial else '')
        mtlxShaderNode = mxg.MtlxNodeGraph.addNode(doc, shaderNodeDefinition, mtlxShadername)
        if not mtlxShaderNode:
            return None

        # Create MaterialX material and shader for each Blender material
        if isMaterial:
            mtlxMaterialNode = mxg.MtlxNodeGraph.addNode(doc, 'ND_surfacematerial', name)
            if mtlxMaterialNode:
                # Connect the material node to the output of the graph
                mxg.MtlxNodeGraph.connectNodeToNode(mtlxMaterialNode, 'surfaceshader', mtlxShaderNode, '')          

        return mtlxShaderNode

    def connectImageNode(self, doc, SHADER_NODE_map, mtlxInput, blenderNode):
        nodeDefinition = SHADER_NODE_map['TEX_IMAGE']
        nodeDefinition = nodeDefinition + mtlxInput.getType() 
        mtxImageNode = self.createMtlxShaderNode(doc, blenderNode.label, nodeDefinition, False)

        # Connect input to new node
        if mtxImageNode:
            imagePath = ''
            if blenderNode.image:
                imagePath = blenderNode.image.filepath_from_user() 
            fileInput = mtxImageNode.addInput('file', 'filename')
            fileInput.setValueString(imagePath)
            mxg.MtlxNodeGraph.connectNodeToNode(mtlxInput.getParent(), mtlxInput.getName(), mtxImageNode, '')
        
        return mtxImageNode

    def connectNormalMapNode(self, doc, SHADER_NODE_map, mtlxInput, blenderNode):
        """ 
        Create a MaterialX normal map node from a Blender node
        Connected the new node to an downstream input 
        """
        nodeDefinition = SHADER_NODE_map['NORMAL_MAP']
        mtxNormalMap = self.createMtlxShaderNode(doc, blenderNode.label, nodeDefinition, False) 
        mxg.MtlxNodeGraph.connectNodeToNode(mtlxInput.getParent(), mtlxInput.getName(), mtxNormalMap, '')                               
        return mtxNormalMap

    def getUpstreamNode(self, blenderInput):
        if not blenderInput:
            return None
        link = blenderInput.links[0] if blenderInput.links else None
        if link and link.is_valid:
            return link.from_node
        return None

    def loadLibraries(self, librarySearchPath): 
        stdlib = mx.createDocument()
        searchPath = mx.getDefaultDataSearchPath()
        libFiles = mx.loadLibraries(mx.getDefaultDataLibraryFolders(), searchPath, stdlib)
        print('Loaded %d library files' % len(libFiles))
        return stdlib

    def createWorkingDocument(self, stdlib):
        doc = mx.createDocument()
        doc.setDataLibrary(stdlib)
        return doc

    def exportMaterials(self, shaderNodeMappings, materialFilter, librarySearchPath):
        """
        Simple Export of a few Blender nodes to MaterialX material nodes + shaders
        """
        docs = dict()

        stdlib = self.loadLibraries(librarySearchPath)
        if len(stdlib.getNodeDefs()) == 0:
            print('Failed to load MaterialX libraries')
            return docs            

        doc = None
        if self.writeSingleMaterialFile:
            doc = self.createWorkingDocument(stdlib)
            docs[self.outputFileName.asString()] = doc

        SHADER_NODE_map = shaderNodeMappings[0]
        SHADER_NODE_INPUTS_map = shaderNodeMappings[1]

        shaderType = 'BSDF_PRINCIPLED'
        for m in bpy.data.materials:
            if not m.node_tree:
                continue

            # Find the default material node type
            materialNode = None
            for node in m.node_tree.nodes:
                if node.type == shaderType:
                    materialNode = node
                    break

            if materialNode: 
                #print('Compare: ', m.name, 'with',  materialFilter)
                if materialFilter and m.name not in materialFilter:
                    print('Skip material: ', m.name)
                    continue
                else:
                    print('Export material: ', m.name)                

                # Creat a corresponding MaterialX material / shader node
                shaderNodeDefinition = SHADER_NODE_map[shaderType]
                if not shaderNodeDefinition:
                    print('Skip handling of node', materialNode.name)
                    continue

                if not self.writeSingleMaterialFile:
                    doc = self.createWorkingDocument(stdlib)

                mtlxShaderNode = self.createMtlxShaderNode(doc, m.name, shaderNodeDefinition, shaderType == 'BSDF_PRINCIPLED')
                if not mtlxShaderNode:
                    continue

                if not self.writeSingleMaterialFile:
                    docs[mtlxShaderNode.getName()] = doc

                mtlxShaderNodeDef = mtlxShaderNode.getNodeDef()

                # Nothing to do with outputs for now
                #for noutput in materialNode.outputs:
                #    print("  - Visit output: ", noutput.name)

                #print('Add inputs to node: ', mtlxShaderNode.getNamePath())
                PBSDF_USDPS_map = SHADER_NODE_INPUTS_map[shaderType]
                for ninput in materialNode.inputs:
                    if not ninput.name in PBSDF_USDPS_map:
                        #print('-- Skip translating input: ', ninput.name)
                        continue                   

                    # Add in inputs
                    val = ninput.default_value
                    portName  = PBSDF_USDPS_map[ninput.name]
                    newInput = None
                    if portName:
                        newInput = self.createMtlxInput(portName, val, mtlxShaderNode, mtlxShaderNodeDef)                         
                        if portName == 'normal':
                            newInput.setValueString('0,0,1') 

                    # Check for upstream connections
                    if not newInput:
                        continue

                    connectedNode = self.getUpstreamNode(ninput)
                    if connectedNode:
                        mtxNormalMap = None
                        # Add a MaterialX normal map node for each Blender normal map node
                        if connectedNode.type == 'NORMAL_MAP':                                
                            colorInput = connectedNode.inputs['Color']
                            connectedNodeUp = None
                            if colorInput:
                                connectedNodeUp = self.getUpstreamNode(colorInput)
                            if connectedNodeUp:
                                mtxNormalMap = self.connectNormalMapNode(doc, SHADER_NODE_map, newInput, connectedNode)
                                # Traverse upstream
                                connectedNode = connectedNodeUp 

                        # Add an MaterialX image node for each Blender texture image node
                        mtxImageNode = None
                        if connectedNode and connectedNode.type == 'TEX_IMAGE':                                
                            mtxImageNode = self.connectImageNode(doc, SHADER_NODE_map, newInput, connectedNode)

                        # Connect normal map and image node if both found
                        if mtxNormalMap and mtxImageNode:
                            mxg.MtlxNodeGraph.connectNodeToNode(mtxNormalMap, 'normal', mtxImageNode, '')  
        return docs

    def writeMaterialXFile(self, doc, filePath):
        """
        Simple utility to write a document to a Markdown section
        and or a to disk.
        """
        if not filePath:
            return

        writeOptions = mx.XmlWriteOptions()
        writeOptions.writeXIncludeEnable = False
        writeOptions.elementPredicate = mxf.MtlxFile.skipLibraryElement
        mx.writeToXmlFile(doc, filePath, writeOptions)

    def writeMaterialXFiles(self, docs):
        filesWritten = []

        for filename in docs:
            doc = docs[filename]
            mtlxFilePath = self.outputPath / filename
            mtlxFilePath.removeExtension()
            mtlxFilePath.addExtension('mtlx')
            self.writeMaterialXFile(doc, mtlxFilePath)
            filesWritten.append(mtlxFilePath.asString())
        
        return filesWritten

    def getMeshesAndMaterials(self, renderableOnly=True, activeOnly=False):
        meshes = []
        materials = []
        scene = bpy.context.scene
        for obj in scene.objects:
            if obj.type == 'MESH' and obj.visible_get():
                if renderableOnly and obj.hide_render == True:
                    continue
                if activeOnly and not obj.select_get():
                    continue
                mat = obj.active_material
                if mat:
                    materials.append(mat.name)
                    meshes.append(obj)    
                else:
                    print('No material for mesh:', obj.name)
            obj.select_set(False)
        return meshes, materials    

    def writeGLTFMesh(self, mesh, outputPath, export_settings):
        """
        Write a Blender mesh out to GLTF format
        """
        exportMaterials = export_settings['write_mesh_material']
        exportNormals = export_settings['export_normals']
        exportColors = export_settings['export_vertex_color']
        exportUv = export_settings['export_uv']
        exportTangent = export_settings['export_tangents']
        exportAnim = export_settings['export_animation']
        outputFormat = export_settings['geometry_format']         

        # Output selected
        outMeshName = outputPath / mx.createValidName(mesh.name) 
        outMeshName.addExtension('gltf')
        if outputFormat == 'GLB':
            outMeshName.addExtension('glb')
        bpy.ops.export_scene.gltf(filepath=outMeshName.asString(),
                                use_visible=True,
                                use_selection=True,
                                export_format=outputFormat,
                                #use_triangles=True,
                                export_cameras=False, 
                                export_lights=False,
                                export_materials=exportMaterials,
                                export_normals=exportNormals,
                                export_texcoords=exportUv,
                                export_all_vertex_colors=exportColors,
                                export_tangents=exportTangent,
                                export_animations=exportAnim,
                                )

        return outMeshName.asString()        

    def writeGLTFMeshes(self, meshes, outputPath, export_settings):
        exportMaterials = export_settings['write_mesh_material']
        exportNormals = export_settings['export_normals']
        exportColors = export_settings['export_vertex_color']
        exportUv = export_settings['export_uv']
        exportTangent = export_settings['export_tangents']
        exportAnim = export_settings['export_animation']
        outputFormat = export_settings['geometry_format'] 

        filesWritten = []

        if self.writeSingleGeomFile:
            print('Write single geometry file...')
            for mesh in meshes:
                mesh.select_set(True)

            outMeshName = outputPath / self.outputFileName
            outMeshName.removeExtension()
            outMeshName.addExtension('glb')
            filesWritten.append(outMeshName.asString())
            bpy.ops.export_scene.gltf(filepath=outMeshName.asString(),
                                use_visible=True,
                                use_selection=True,
                                export_format=outputFormat,
                                #use_triangles=True,
                                export_cameras=False, 
                                export_lights=False,
                                export_materials=exportMaterials,
                                export_normals=exportNormals,
                                export_texcoords=exportUv,
                                export_all_vertex_colors=exportColors,
                                export_tangents=exportTangent,
                                export_animations=exportAnim,
                                )
        else:
            print('Write separate geometry file...')
            for mesh in meshes:
                mesh.select_set(True)
                filesWritten.append( self.writeGLTFMesh(mesh, outputPath, export_settings) )
                mesh.select_set(False)    

        return filesWritten

    def execute(self, export_settings):    
        """
        Perform export.
        """
        docs = dict()

        selected_objects = export_settings['selected_objects']
        separateMtlxFile = export_settings['seperate_materials']
        write_materials_to_file = export_settings['write_materials_to_file']

        outputFileName = mx.FilePath(export_settings['file_path'])
        outputFilePath = outputFileName.getParentPath()
        outputFileName = mx.FilePath(outputFileName.getBaseName())

        self.setOutputPath(outputFilePath)
        self.setOutputFileName(outputFileName)

        shaderNodeMap = self.init_node_dictionary('ND_UsdPreviewSurface_surfaceshader')
        self.setWriteSingleMaterialFile(not separateMtlxFile)
        librarySearchPath = mx.FileSearchPath()

        meshes, materials = self.getMeshesAndMaterials(True, selected_objects)
        if materials:
            docs  = self.exportMaterials(shaderNodeMap, materials, librarySearchPath)
            if docs: 
                if write_materials_to_file:
                    filesWritten = self.writeMaterialXFiles(docs)
                    for f in filesWritten:
                        print('Write MaterialX material file:', f)
            else:
                print('Failed to export materials')

            writeMtlxGraph =  export_settings['diagram']
            if writeMtlxGraph:
                for fileName in docs:
                    # Load in document and create a Mermaid graph
                    doc = docs[fileName]
                    roots = doc.getMaterialNodes()
                    graph = mxt.MtlxMermaid.generateMermaidGraph(roots, 'LR')
                    graphFileName = outputFilePath / mx.FilePath(fileName)
                    graphFileName.addExtension('md')
                    print('Write Mermaid graph file:', graphFileName.asString())
                    graphFile = open(graphFileName.asString(), 'w')
                    if graphFile:                
                        graphFile.write('```mermaid\n')
                        for line in graph:
                            if line:
                                graphFile.write(line + '\n')
                        #graphFile.writelines(graph)
                        graphFile.write('```\n')
                        graphFile.close() 

        # Export meshes as separate pieces
        if meshes:
            separateGeomFile = export_settings['seperate_files']
            if export_settings['write_geometry']:
                print("write meshes....")
                filesWritten = []
                output = io.StringIO()
                with redirect_stdout(output), redirect_stderr(output):        
                    self.setWriteSingleGeomFile(not separateGeomFile)
                    filesWritten = self.writeGLTFMeshes(meshes, outputFilePath, export_settings)
                for fileWritten in filesWritten:
                    print('Write GLTF to file:', fileWritten)

        print(f"----------------- RETURN {len(docs)} documents")
        return docs

###################

class BlenderSVGBuilder():
    """
    Utility class to convert SVG to a mesh.
    Uses Blender to convert from SVG to mesh and export to GLTF
    Use MaterialX to create material from SVG color. Option to choose the type
    of shader definition (lit or not). Default is to use a lit material (standard surface).
    """
    
    svgFilePath = mx.FilePath()
    materialDict = dict()
    shaderNodeDefinition = 'ND_standard_surface_surfaceshader'
    extrudeValue = 0.001
    bevelValue = 0.0
    
    def setExtrudeValue(self, val):
        self.extrudeValue = val

    def setBevelValue(self, val):
        self.bevelValue = val

    def setUseUnlitMaterial(self, val):
        if val:
            self.shaderNodeDefinition = 'ND_surface_unlit'
        else:
            self.shaderNodeDefinition = 'ND_standard_surface_surfaceshader'

    def inputColorName(self):
        if self.shaderNodeDefinition == 'ND_standard_surface_surfaceshader':
            return 'base_color'
        else:
            return 'emission_color'

    def setSVGFilePath(self, svgFile):    
        self.svgFilePath = svgFile
        
    def load(self):
        if not os.path.exists(self.svgFilePath.asString()):
            return False
        try:
            bpy.ops.import_curve.svg(filepath=self.svgFilePath.asString())
        except RuntimeError as ex:
            error_report = "\n".join(ex.args)
            print("Error on SVG load:", error_report)
            return False

        return True
        
    def extrudeCurves(self):
        extruded = False
        for curve in bpy.data.curves[:]:
            curve.extrude = self.extrudeValue                    
            curve.bevel_depth = self.bevelValue
            extruded = True
        return extruded
            
    def createMaterial(self, obj):
        material = obj.active_material
        if not material:
            material = bpy.data.materials.new(name=obj.name + '_Material')
            obj.active_material = material
            print('  -- Create material for curve with no material:', material.name)
        if material.name in self.materialDict:
            return
        
        baseColor = material.diffuse_color
        material.use_nodes = True
        bsdfNode = material.node_tree.nodes['Principled BSDF']
        if bsdfNode:
            # Assign the color to the BSDF node
            bsdfNode.inputs[0].default_value = baseColor

    def floatToStr(self, val):
        """ 
        Emit formatted float value to string
        """
        return f"{val:.4g}"

    def createMtlxMaterial(self, doc, obj):
        """
        Create a MaterialX material and shaders.
        Will create a 'standard_surface' BSDF node 

        TODO: Hash unique shader based on material values (base_color etc) instead of name of material
        """
        look = doc.getLook('Default_Look')
        if not look:
            look = doc.addLook('Default_Look')

        blmaterial = obj.active_material
        meshName = mx.createValidName('Mesh_' + obj.name)
        if blmaterial.name in self.materialDict:
            print('---- Assign existing material:', self.materialDict[blmaterial.name], ' to object: ', )
            assign = look.getMaterialAssign(self.materialDict[blmaterial.name])
            if assign:
                curGeom = assign.getGeom()
                assign.setGeom(curGeom + ', ' + meshName)
            return

        mtlxShadername = blmaterial.name + ('_' + 'Shader')
        mtlxShaderNode = mxg.MtlxNodeGraph.addNode(doc, self.shaderNodeDefinition, mtlxShadername)
        if not mtlxShaderNode:
            return None

        # Create MaterialX material and shader for each Blender material
        mtlxMaterialNode = mxg.MtlxNodeGraph.addNode(doc, 'ND_surfacematerial', blmaterial.name + '_Material')
        if mtlxMaterialNode:
            # Connect the material node to the output of the graph
            mxg.MtlxNodeGraph.connectNodeToNode(mtlxMaterialNode, 'surfaceshader', mtlxShaderNode, '')          

        inputName = self.inputColorName()
        mtlxShaderNode.addInputFromNodeDef(inputName)
        if blmaterial:
            baseColor = blmaterial.diffuse_color
            base_color = mtlxShaderNode.getInput(inputName)
            #print('    - Set base color = ', self.floatToStr(baseColor[0]), self.floatToStr(baseColor[1]), self.floatToStr(baseColor[2]))
            base_color.setValue(mx.Color3(baseColor[0], baseColor[1], baseColor[2]))

        newMaterialName = mtlxMaterialNode.getNamePath()
        self.materialDict[blmaterial.name] = newMaterialName

        print('-- Create MTLX material node:', newMaterialName)
        print('---- Assign new material:', newMaterialName, ' to object: ', meshName)
        assign = look.addMaterialAssign(newMaterialName, newMaterialName)
        assign.setGeom(meshName)
        return mtlxMaterialNode

    def createMeshFromCurve(self, context, curve):
        deg = context.evaluated_depsgraph_get()
        mesh = None
        try:
            mesh = bpy.data.meshes.new_from_object(curve.evaluated_get(deg), depsgraph=deg)
        except RuntimeError as ex:
            error_report = "\n".join(ex.args)
            print("Error on SVG load:", error_report)
            return None, None
        
        mesh.name = mx.createValidName('Mesh_' + curve.name + '_geom')

        new_obj = bpy.data.objects.new(mx.createValidName('Mesh_' + curve.name), mesh)
        context.collection.objects.link(new_obj)

        new_obj.matrix_world = curve.matrix_world
        
        # Center the object
        #new_obj.select_set(True)
        #context.view_layer.objects.active = new_obj
        #bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')
        #new_obj.location[0] = 0.0
        #new_obj.location[1] = 0.0
        #new_obj.location[2] = 0.0
        return new_obj, mesh

    def convertToMesh(self, doc):
        newobjs = []
        context = bpy.context
        bpy.ops.object.select_all(action='DESELECT')
        for obj in bpy.data.objects:
            if obj.type == 'MESH':
                obj.select_set(True)
                bpy.ops.object.delete()

        for obj in bpy.data.objects:
            #print('Examine object:', obj.name)
            if obj.type == 'CURVE':
                obj.select_set(True)
                #print('- Examine curve:', obj.name)
                newobj, mesh = self.createMeshFromCurve(context, obj)
                if newobj and mesh:
                    mesh.name = mx.createValidName(mesh.name)
                    print('-- Created mesh:', newobj.name, ',', mesh.name, 'for curve:', obj.name)
                    self.createMaterial(obj)
                    self.createMtlxMaterial(doc, obj)
                    newobjs.append(newobj)

        bpy.ops.object.select_all(action='DESELECT')
        for obj in bpy.data.objects:
            if obj.type == 'CURVE':
                obj.select_set(True)
                bpy.ops.object.delete()

        return newobjs
    
    def writeMaterialXFile(self, doc, filePath):
        if not filePath:
            return

        writeOptions = mx.XmlWriteOptions()
        major, minor, patch = mx.getVersionIntegers()
        # Write predicate does not work prior to 1.38.7
        if major >= 1 and minor >= 38 and patch >= 7:
            writeOptions.writeXIncludeEnable = False
            writeOptions.elementPredicate = mxf.MtlxFile.skipLibraryElement
        else:
            for elem in doc.getChildren():
                    if elem.hasSourceUri():
                        doc.removeChild(elem.getName())

        mx.writeToXmlFile(doc, filePath, writeOptions)

class imageToMeshBuilder():
    """
    Utility class to convert image to a mesh.
    Uses Blender to create a mesh with the dimensions of an image
    Use MaterialX to create material with image as base color input. Option to choose the type
    of shader definition (lit or not). Default is to use a lit material (standard surface).
    """
   
    shaderNodeDefinition = 'ND_standard_surface_surfaceshader'
    imageFilePath = ''
    imageFilePathId = ''

    def setImageFilePath(self, val):
        self.imageFilePath = val
        path = mx.FilePath(val)
        path.removeExtension()
        path = path.getBaseName()
        self.imageFilePathId = mx.createValidName(path)

    def setUseUnlitMaterial(self, val):
        if val:
            self.shaderNodeDefinition = 'ND_surface_unlit'
        else:
            self.shaderNodeDefinition = 'ND_standard_surface_surfaceshader'

    def loadImage(self):

        yimage = ximage = -1        
        image_src = bpy.data.images.load(self.imageFilePath)
        if image_src:
            yimage = image_src.size[1]
            ximage = image_src.size[0]
        return image_src, yimage, ximage

    def createMaterial(self, obj, image_src, materialName):
        mat = obj.active_material
        if not mat:
            mat = bpy.data.materials.new(name=obj.name + '_Material')
            obj.active_material = mat
        
        mat.use_nodes = True
        
        #mat = ob.active_material
        # Get the nodes
        nodes = mat.node_tree.nodes
        nodes.clear()

        bsdfNode = nodes.new(type='ShaderNodeBsdfPrincipled')
        #bsdfNode.location = 0,0

        imageNode = nodes.new('ShaderNodeTexImage')
        imageNode.image = image_src 
        #node_tex.location = -400,0

        materialNode = nodes.new(type='ShaderNodeOutputMaterial')   
        #node_output.location = 400,0

        # Link all nodes
        links = mat.node_tree.links
        link = links.new(imageNode.outputs["Color"], bsdfNode.inputs["Base Color"])
        link = links.new(bsdfNode.outputs["BSDF"], materialNode.inputs["Surface"])    

    def floatToStr(self, val):
        """ 
        Emit formatted float value to string
        """
        return f"{val:.4g}"

    def inputColorName(self):
        if self.shaderNodeDefinition == 'ND_standard_surface_surfaceshader':
            return 'base_color'
        else:
            return 'emission_color'

    def createMtlxMaterial(self, doc, meshName, materialName):
        """
        Create a MaterialX material and shaders.
        """
        look = doc.getLook(materialName + '_look')
        if not look:
            look = doc.addLook(materialName + '_look')

        #meshName = mx.createValidName('Mesh_' + obj.name)

        mtlxShadername = materialName + ('_' + 'Shader')
        mtlxShaderNode = mxg.MtlxNodeGraph.addNode(doc, self.shaderNodeDefinition, mtlxShadername)
        if not mtlxShaderNode:
            return None

        mtlxMaterialNode = mxg.MtlxNodeGraph.addNode(doc, 'ND_surfacematerial', materialName + '_Material')
        if mtlxMaterialNode:
            # Connect the material node to the output of the graph
            mxg.MtlxNodeGraph.connectNodeToNode(mtlxMaterialNode, 'surfaceshader', mtlxShaderNode, '')          

        inputName = self.inputColorName()
        mtlxShaderNode.addInputFromNodeDef(inputName)

        mtlxImageNode = mxg.MtlxNodeGraph.addNode(doc, 'ND_tiledimage_color3', materialName + '_Image')
        input = mtlxImageNode.addInputFromNodeDef('file')
        input.setValueString(self.imageFilePath)
        mxg.MtlxNodeGraph.connectNodeToNode(mtlxShaderNode, inputName, mtlxImageNode, '')

        newMaterialName = mtlxMaterialNode.getNamePath()
        #self.materialDict[materialName] = newMaterialName

        #print('-- Create MTLX material node:', newMaterialName)
        #print('---- Assign new material:', newMaterialName, ' to object: ', meshName)
        assign = look.addMaterialAssign(newMaterialName, newMaterialName)
        assign.setGeom(meshName)
        return mtlxMaterialNode

    def setMeshUvs(self, obj, sideTexel = [0.05, 0.05]):
        #obj = bpy.context.active_object
        bm = bmesh.from_edit_mesh(obj.data)

        uv_layer = bm.loops.layers.uv.verify()
        loop_uvs = [ [0,0], [1,0], [1,1], [0,1] ] 
        loop_uvs2 = [ sideTexel, sideTexel, sideTexel, sideTexel ] 
        facei = 0
        for f in bm.faces:
            index = 0;      
            for l in f.loops:
                if facei == 5 or facei == 4:
                    l[uv_layer].uv = (loop_uvs[index][0], loop_uvs[index][1])
                else:
                    l[uv_layer].uv = (loop_uvs2[index][0], loop_uvs2[index][1])
                index = index + 1
            facei = facei + 1

        bmesh.update_edit_mesh(obj.data)

    def deleteAllObjects(self):
        deleteListObjects = ['MESH', 'CURVE', 'SURFACE', 'META', 'FONT', 'HAIR', 'POINTCLOUD', 'VOLUME', 'GPENCIL',
                         'ARMATURE', 'LATTICE', 'EMPTY', 'LIGHT', 'LIGHT_PROBE', 'CAMERA', 'SPEAKER']

        for o in bpy.context.scene.objects:
            for i in deleteListObjects:
                if o.type == i:
                    o.select_set(False)
                else:
                    o.select_set(True)
        bpy.ops.object.delete() 

    #bpy.ops.object.mode_set( mode = 'OBJECT' )

    def execute(self, width=1.0, height = 0.1, sideTexel = [0.05, 0.05]):
        # Clear the scene
        self.deleteAllObjects()

        # Load input image
        image_src, yimage, ximage = self.loadImage()
        if not image_src:
            return
        
        # Create a cube and size it to match the image x,y dimenesions but normalized so that x is length 1
        cubex = width
        cubey = width * (yimage / ximage)
        bpy.ops.mesh.primitive_cube_add(size=1.0, calc_uvs=True, enter_editmode=False, align='WORLD', location=(0.0, 0.0, 0.0), rotation=(math.pi, 0.0, 0.0), scale=(cubex, cubey, height))
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

        selected = [obj for obj in bpy.context.selected_objects]
        geom = selected[0]
        geom.name = self.imageFilePathId
        self.createMaterial(geom, image_src, 'material')

        #bpy.ops.screen.info_log_show()

        # Update texture coordinates
        bpy.ops.object.mode_set( mode = 'EDIT' )
        bpy.ops.mesh.select_mode( type = 'FACE' )
        bpy.ops.mesh.select_all( action = 'SELECT' ) 
        self.setMeshUvs(selected[0], sideTexel)
        bpy.ops.object.mode_set( mode = 'OBJECT' )

    def writeMaterialXFile(self, doc, filePath):
        if not filePath:
            return

        writeOptions = mx.XmlWriteOptions()
        major, minor, patch = mx.getVersionIntegers()
        # Write predicate does not work prior to 1.38.7
        if major >= 1 and minor >= 38 and patch >= 7:
            writeOptions.writeXIncludeEnable = False
            writeOptions.elementPredicate = mxf.MtlxFile.skipLibraryElement
        else:
            for elem in doc.getChildren():
                    if elem.hasSourceUri():
                        doc.removeChild(elem.getName())

        mx.writeToXmlFile(doc, filePath, writeOptions)
