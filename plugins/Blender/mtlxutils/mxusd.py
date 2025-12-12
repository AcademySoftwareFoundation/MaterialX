#!/usr/bin/env python
'''
Conversion utilities to cnovert from Usd to MaterialX and MaterialX to Usd
'''
from pxr import Usd
from pxr import UsdShade
from pxr import Sdf
from pxr import Gf

import MaterialX as mx
import mtlxutils.mxfile as mxf

class UsdToMtlx():
    '''
    Sample Usd to MaterialX convertor. Takes a Usd file as input and outputs a MaterialX file.
    For now all materials found in the the Usd stage will be output to the document level in MaterialX
    As material scoping is not preserved the export may fail if duplicate nodes are encountered.     
    '''

    # MaterialX definition libraries
    stdlib = None
    # List of MaterialX library files
    libFiles = []
    # MaterialX working document
    doc = None
    # User search paths
    userPaths = mx.FileSearchPath()
    # User library folders
    userLibraries = []

    # Last error found
    error = ''
    # Nodes skipped
    skippedNodes = []
    
    # Option to insert comments
    insertComments = False    

    def initializeMaterialXDoc(self):
        self.doc, self.libFiles, status = mxf.MtlxFile.createWorkingDocument(
            self.userPaths, self.userLibraries)

    def getMaterials(self, stage):
        ''''''
        # prim = stage.GetPrimAtPath(rootPath)
        materials = [x for x in stage.Traverse() if x.IsA(UsdShade.Material)]
        return materials

    def mapUsdTypeToMtlx(self, usdType):
        '''
        Map a Usd type string to a MaterialX type string.
        Note this is not a complete mapping.
        '''
        error = ''
        usdTypeString = str(usdType)
        mtlxType = 'color3'
        if 'color3' in usdTypeString:
            mtlxType = 'color3'
        elif 'color4' in usdTypeString:
            mtlxType = 'color4'
        elif 'float4' in usdTypeString:
            mtlxType = 'vector4'
        elif 'vector3' in usdTypeString:
            mtlxType = 'vector3'
        elif 'float2' in usdTypeString:
            mtlxType = 'vector2'
        elif 'float' == usdType:
            mtlxType = 'float'
        elif 'string' in usdTypeString:
            mtlxType = 'string'
        elif 'int' in usdTypeString:
            mtlxType = 'integer'
        elif 'bool' in usdTypeString:
            mtlxType = 'boolean'
        elif 'asset' in usdTypeString:
            mtlxType = 'filename'
        elif 'token' in usdTypeString:
            mtlxType = 'token'
        else:
            mtlxType = usdTypeString
            self.error = 'Mapping of Usd type failed %s' % usdTypeString
        return mtlxType

    def mapUsdValueToMtlx(self, mtlxType, usdValue):
        """
        Map a Usd value to a MaterialX value.
        Note this is not a complete mapping. Ideally, if this is a value on a node
        input / output, then the definition can be queried to get the default value.
        """
        mtlxValue = None
        if mtlxType == 'float':
            if not usdValue:
                mtlxValue = '0'
            else:
                mtlxValue = str(usdValue)
        elif mtlxType == 'integer':
            if not usdValue:
                mtlxValue = '0'
            else:
                mtlxValue = str(usdValue)
        elif mtlxType == 'boolean':
            if not usdValue:
                mtlxValue = 'false'
            else:
                mtlxValue = str(usdValue).lower()
        elif mtlxType == 'string' or mtlxType in ['displacementshader', 'surfaceshader', 'volumeshader']:
            if not usdValue:
                mtlxValue = ''
            else:
                mtlxValue = usdValue
        elif mtlxType == 'filename':
            if not usdValue:
                mtlxValue = ''
            else:
                # Remvoe @ prefix and suffix to conver from Usd to MaterialX
                mtlxValue = str(usdValue).removeprefix('@').removesuffix('@')
        elif mtlxType == 'vector2':
            if not usdValue:
                mtlxValue = '0, 0'
            else:
                mtlxValue = str(usdValue[0]) + ',' + str(usdValue[1])
        elif mtlxType == 'color3' or mtlxType == 'vector3':
            if not usdValue:
                mtlxValue = '0, 0, 0'
            else:
                mtlxValue = str(usdValue[0]) + ',' + \
                                str(usdValue[1]) + ',' + str(usdValue[2])
        elif mtlxType == 'color4' or mtlxType == 'vector4':
            if usdValue is None:
                mtlxValue = '0, 0, 0, 0'
            else:
                mtlxValue = str(usdValue[0]) + ',' + str(usdValue[1]) + \
                                ',' + str(usdValue[2]) + ',' + str(usdValue[3])

        error = ''
        if mtlxValue is None:
            self.error = 'Mapping of Usd Value %s failed for MaterialX type %s' % (
                usdValue, mtlxType)

        return mtlxValue

    def mapUsdSdfTypeToMtlx(self, usdType):
        '''
        Mapping from Sdf type to MaterialX type.
        It is possible to map using Sdf type.
        '''
        mtlxUsdMap = dict()
        mtlxUsdMap[Sdf.ValueTypeNames.Asset] = 'filename'
        mtlxUsdMap[Sdf.ValueTypeNames.String] = 'string'
        mtlxUsdMap[Sdf.ValueTypeNames.Bool] = 'boolean'
        mtlxUsdMap[Sdf.ValueTypeNames.Int] = 'integer'
        mtlxUsdMap[Sdf.ValueTypeNames.Color3f] = 'color3'
        mtlxUsdMap[Sdf.ValueTypeNames.Color4f] = 'color4'
        mtlxUsdMap[Sdf.ValueTypeNames.Float] = 'float'
        mtlxUsdMap[Sdf.ValueTypeNames.Float2] = 'vector2'
        mtlxUsdMap[Sdf.ValueTypeNames.Float3] = 'vector3'
        mtlxUsdMap[Sdf.ValueTypeNames.Vector3f] = 'vector3'
        mtlxUsdMap[Sdf.ValueTypeNames.Float4] = 'vector4'

        if usdType in mtlxUsdMap:
            return mtlxUsdMap[usdType]
        return 'string'

    def isMultiOutput(self, prim):
        ''' 
        Test if the Usd prim has multiple outputs
        '''
        outputCount = 0
        if prim.IsA(UsdShade.Material):
            usdMaterial = UsdShade.Material(prim)
            outputCount = len(usdMaterial.GetOutputs())
        elif prim.IsA(UsdShade.NodeGraph):
            usdNodegraph = UsdShade.NodeGraph(prim)
            outputCount = len(usdNodegraph.GetOutputs())
        elif prim.IsA(UsdShade.Shader):
            usdShader = UsdShade.Shader(prim)
            outputCount = len(usdShader.GetOutputs())

        return outputCount > 1

    def mapUsdTokenToType(self, mtlxType, usdBaseName, mtlxPrefix=False):
        """
        Utility to test the base name for a semantic match to a surface, displacement or volume shader
        If found return the appropriate MaterialX type. Otherwise the passed in type is just returned. 

        Note that only types scoped with 'mtlx' are considered to be MaterialX shaders, if the 
        `mtlxPrefix` argument is True. Default is False.
        """
        usdBaseNameSplit = mx.splitString(usdBaseName, ':')
        testName = usdBaseNameSplit[len(usdBaseNameSplit)-1]            
        if not mtlxPrefix or (mtlxPrefix and 'mtlx' in usdBaseNameSplit): 
            if 'displacement' == str(testName) or 'displacementshader' == str(testName):
                mtlxType = 'displacementshader'
            elif 'surface' == str(testName) or 'surfaceshader' == str(testName):
                mtlxType = 'surfaceshader'
            elif 'volume' == str(testName) or 'volumeshader' == str(testName):
                mtlxType = 'volumeshader'
        return mtlxType

    def emitMtxlInputs(self, shader, parent):
        '''
        Emit MaterialX inputs (for a given parent node) corresponding to Usd inputs for a given parent shader

        Currently only surface, volume, and displacement shader types are supported as input on materials.
        
        The different syntax handling for denoting upstream node versus nodegraph and interface inputs connections 
        is handled here. 
        '''
        for input in shader.GetInputs():

            # Only output if there is a value or a connection
            if input:

                # Map Usd type to MaterialX type and create an new input
                usdType = input.GetTypeName()
                mtlxType = self.mapUsdTypeToMtlx(usdType)
                usdBaseName = input.GetBaseName()
                mtlxType = self.mapUsdTokenToType(mtlxType, usdBaseName)
                usdBaseName = usdBaseName.replace(':', '_')    

                # Add a connection if encountered
                if input.HasConnectedSource():
                    newInput = parent.addInput(usdBaseName, mtlxType)

                    # Only consider "valid" inputs.
                    usdSources, invalidSources = input.GetConnectedSources() 
                    if usdSources and usdSources[0]:
                        # Check UsdShadeConnectionSourceInfo to extract
                        # out the upstream information
                        usdSource1 = usdSources
                        sourcePrim = usdSource1[0].source.GetPrim()
                        sourcePort = usdSource1[0].sourceName # e.g. out
                        sourceDirection = usdSource1[0].sourceType # e.g. Input / Output
                        sourceType = usdSource1[0].typeName # e.g. color3f

                        # Handle the complex MaterialX attribute syntax
                        # for specifying a connection.
                        # ---------------------------------------------
                        # Assume a node to input connection to start
                        mtlxConnectString = 'nodename'
                        mtlxConnectItem = sourcePrim.GetName()

                        # An input->input connection is denoted using
                        # "interfacename", but no "node", or "nodegraph"
                        if sourceDirection == UsdShade.AttributeType.Input:
                            mtlxConnectString = 'interfacename'
                            mtlxConnectItem = sourcePort

                            # Set the connection
                            newInput.setAttribute(mtlxConnectString, mtlxConnectItem)

                        else:
                            # A nodegraph to output connection uses "nodegraph" vs "node"                        
                            if sourcePrim.IsA(UsdShade.NodeGraph):
                                mtlxConnectString = 'nodegraph'

                            # Set the connection
                            newInput.setAttribute(mtlxConnectString, mtlxConnectItem)

                            # An output to input connection is denoted using
                            # an additional `output` attribute` if the source is 
                            # does not have multiple outputs
                            if sourceDirection == UsdShade.AttributeType.Output:
                                if self.isMultiOutput(sourcePrim):
                                    newInput.setAttribute('output', sourcePort)                    
    
                # Set value if not connected.
                else:
                    usdVal = input.Get()
                    if usdVal is not None:
                        newInput = parent.addInput(usdBaseName, mtlxType)
                        if newInput:
                            mtlxVal = self.mapUsdValueToMtlx(mtlxType, usdVal)
                            if mtlxVal is not None:
                                newInput.setValueString(mtlxVal)
                                
    def emitMtxlOutputs(self, shader, parent):
        '''
        Emit MaterialX outputs (for a given parent node) corresponding to Usd outputs for a given parent shader        
        Unlike Usd, outputs are not explicitly defined in MaterialX nodes but are on nodegraphs. This
        as well as different syntax handling for denoting upstream node versus nodegraph connections is handled
        here.

        Note that MaterialX materials specify connections as input and NOT as outputs
        as with Usd. Additionally only a subset of shader types are supported: surface, volume, and displacement
        If there is more than one input shader with the same MaterialX type, only the first will be recorded.

        '''
        MTLX_SHADER_TYPES = ['surfaceshader', 'volumeshader', 'displacementshader']

        for output in shader.GetOutputs():
            if output:

                usdType = output.GetTypeName()

                mtlxType = self.mapUsdTypeToMtlx(usdType)
                usdBaseName = output.GetBaseName()
                
                newPort = None
                # Write MaterialX material node inputs for Usd material node outputs.
                # Only keep the first shader of a given type
                if parent.getType() == 'material':
                    mtlxType = self.mapUsdTokenToType(mtlxType, usdBaseName, True)
                    if mtlxType in MTLX_SHADER_TYPES:
                        if parent.getInput(mtlxType):
                            print('Skip connecting > 1 shader of type %s on material %s' % (mtlxType, parent.getNamePath()))
                        else:
                            newPort = parent.addInput(mtlxType, mtlxType)
                
                # Write other MaterialX nodes and and nodgraph outputs as outputs 
                else:
                    mtlxType = self.mapUsdTokenToType(mtlxType, usdBaseName)
                    usdBaseName = usdBaseName.replace(':', '_')    
                    newPort = parent.addOutput(usdBaseName, mtlxType)

                # Translate output connections. Ignore invalid Usd connections
                if newPort and output.HasConnectedSource():
                    usdSources, invalidSources = output.GetConnectedSources() 
                    if usdSources and usdSources[0]:
                        # Check UsdShadeConnectionSourceInfo
                        usdSource1 = usdSources[0]
                        sourcePrim = usdSource1.source.GetPrim()
                        sourcePort = usdSource1.sourceName
                        sourceDirection = usdSource1.sourceType
                        sourceType = usdSource1.typeName

                        mtlxConnectString = 'nodename'
                        mtlxConnectItem = sourcePrim.GetName()

                        # An input->output connection should never occur
                        # and is ignored
                        #if sourceDirection == UsdShade.AttributeType.Input:

                        # Handle adding in node or nodegraph depending on source prim type.                                
                        if sourcePrim.IsA(UsdShade.NodeGraph):
                            mtlxConnectString = 'nodegraph'

                        newPort.setAttribute(mtlxConnectString, mtlxConnectItem)

                        # Handle outputs connected to outputs
                        if sourceDirection == UsdShade.AttributeType.Output:
                            if self.isMultiOutput(sourcePrim):
                                newPort.setAttribute('output', sourcePort)

    def emitMtlxValueElements(self, shader, parent, emitInputs, emitOutputs):
        '''
        Emit MaterialX value elements on a shader. Currently only Inputs and Outputs are handled.
        This is not a complete translation of all value element meta-data attributes (e.g. colorspae is not handled)
        '''
        if emitInputs:
            self.emitMtxlInputs(shader, parent)
        if emitOutputs:
            self.emitMtxlOutputs(shader, parent)

    def emitMaterialX(self, stage, indent, prim, parent):
        """
        Emit MaterialX for a given Usd Stage starting at a given root.
        Currently only nodegraphs, material and shader nodes are supported.
        """
        if prim:
            # Test if it's a material first as a material is a nodegraph
            # Ignore inputs as they have no meaning on a MaterialX material.
            if prim.IsA(UsdShade.Material): 
                if self.insertComments:
                    comment = parent.addChildOfCategory('comment')
                    comment.setDocString(' Usd material: %s ' % str(prim.GetPath()))
                doc = parent.getDocument()
                usdMaterial = UsdShade.Material(prim)
                mtlxName = parent.createValidChildName(prim.GetName())
                mtlxMaterial = parent.addMaterialNode(mtlxName)
                self.emitMtlxValueElements(usdMaterial, mtlxMaterial, False, True)

            elif prim.IsA(UsdShade.NodeGraph):
                if self.insertComments:
                    comment = parent.addChildOfCategory('comment')
                    comment.setDocString(' Usd node graph: %s ' % str(prim.GetPath()))
                doc = parent.getDocument()
                usdNodegraph = UsdShade.NodeGraph(prim)
                mtlxName = parent.createValidChildName(prim.GetName())
                mtlxNodeGraph = parent.addChildOfCategory('nodegraph', mtlxName)
                parent = mtlxNodeGraph
                self.emitMtlxValueElements(usdNodegraph, mtlxNodeGraph, True, True)

            elif prim.IsA(UsdShade.Shader): 
                if self.insertComments:
                    comment = parent.addChildOfCategory('comment')
                    comment.setDocString(' Usd node: %s ' % str(prim.GetPath()))
                usdShader = UsdShade.Shader(prim)
                mtlxNodeDefId = ''
                
                # Note: Only consider when the definition is specified in the identifier
                usdImplAttr = usdShader.GetImplementationSourceAttr()
                if usdImplAttr.Get() == 'id':
                    mtlxNodeDefId = usdShader.GetIdAttr().Get()

                # Do a manual rename for built in UsdPreviewSurface
                # Could be done for other built-ins which have MaterialX
                # definitions.
                if mtlxNodeDefId == 'UsdPreviewSurface':
                    mtlxNodeDefId = 'ND_UsdPreviewSurface_surfaceshader'

                # Look for an existing definition. If found add an instance and populate
                # it's inputs and outputs.
                doc = parent.getDocument()
                mtlxNodeDef = doc.getNodeDef(mtlxNodeDefId)
                if mtlxNodeDef:
                    mtlxShadername = parent.createValidChildName(prim.GetName())
                    shaderNode = parent.addNodeInstance(mtlxNodeDef, mtlxShadername)                
                    self.emitMtlxValueElements(usdShader, shaderNode, True, False)
                else:
                    self.skippedNodes.append(prim.GetName())  

            children = prim.GetChildren()
            for child in children:
                self.emitMaterialX(stage, indent+indent, child, parent)

    def emit(self, stage, librarySearchPaths = None, libFolders = []):

        self.doc = None
        materials = self.getMaterials(stage)
        if not materials:
            error = 'No materials to translate in Usd stage'
            return self.doc
        
        self.userPaths = mx.FileSearchPath()
        if librarySearchPaths:
            for pathList in librarySearchPaths:
                for path in pathList:
                    self.userPaths.append(path)

        self.userLibraries = []
        if libFolders:
            for libraryList in libFolders:
                for library in libraryList:
                    self.userLibraries.append(library)

        self.initializeMaterialXDoc()
        if self.doc:
            # Add a top level comment about generation
            comment = self.doc.addChildOfCategory('comment')
            commentString = '\n    The contents of this document were generated from Usd using the mxusd Python class.'
            if self.userPaths.asString() or self.userLibraries:
                commentString = commentString + '\n\n    - User search paths: ' + self.userPaths.asString() + '\n    - User libraries: ' + ','.join(self.userLibraries) + '\n'
            commentString = commentString + '\n  '
            comment.setDocString(commentString)
        
            # Start at the root and emit child nodes 
            prim = stage.GetPrimAtPath('/')
            self.emitMaterialX(stage, ' ', prim, self.doc)
        
        return self.doc
    
###########################################################################################################
class MtlxToUsd():
    '''
    Sample convert from MaterialX to Usd
    '''
  
    def getUsdTypes(self):
        types = []
        for t in dir(Sdf.ValueTypeNames):
            if t.startswith('__'):
                continue
            types.append(str(t))

    def mapMtxToUsdType(self, mtlxType):
        '''
        Map a MaterialX type to an Usd Sdf type. The mapping is easier from MaterialX as
        the number of type variations is much less. Note that one Usd type is chosen
        with no options for choosing things like precision.
        '''
        mtlxUsdMap = dict()
        mtlxUsdMap['filename'] = Sdf.ValueTypeNames.Asset
        mtlxUsdMap['string'] = Sdf.ValueTypeNames.String
        mtlxUsdMap['boolean'] = Sdf.ValueTypeNames.Bool
        mtlxUsdMap['integer'] = Sdf.ValueTypeNames.Int
        mtlxUsdMap['float'] = Sdf.ValueTypeNames.Float
        mtlxUsdMap['color3'] = Sdf.ValueTypeNames.Color3f
        mtlxUsdMap['color4'] = Sdf.ValueTypeNames.Color4f
        mtlxUsdMap['vector2'] = Sdf.ValueTypeNames.Float2    
        mtlxUsdMap['vector3'] = Sdf.ValueTypeNames.Vector3f    
        mtlxUsdMap['vector4'] = Sdf.ValueTypeNames.Float4    
        mtlxUsdMap['surfaceshader'] = Sdf.ValueTypeNames.Token
        mtlxUsdMap['volumeshader'] = Sdf.ValueTypeNames.Token
        mtlxUsdMap['displacementshader'] = Sdf.ValueTypeNames.Token

        if mtlxType in mtlxUsdMap:
            return mtlxUsdMap[mtlxType]
        return Sdf.ValueTypeNames.Token

    def mapMtxToUsdValue(mtlxType, mtlxValue):
        '''
        Map a MaterialX value of a given type to a Usd value.
        Note: Not all types are included here.
        '''
        usdValue = '__'
        if mtlxType == 'float':
            usdValue = mtlxValue
        elif mtlxType == 'integer':
            usdValue = mtlxValue
        elif mtlxType == 'boolean':                    
            usdValue = mtlxValue
        elif mtlxType == 'string':                    
            usdValue = mtlxValue
        elif mtlxType == 'filename':  
            usdValue = mtlxValue
        elif mtlxType == 'vector2':
            usdValue = Gf.Vec2f( mtlxValue[0], mtlxValue[1] )
        elif mtlxType == 'color3' or mtlxType == 'vector3':
            usdValue = Gf.Vec3f( mtlxValue[0], mtlxValue[1], mtlxValue[2] )
        elif mtlxType == 'color4' or mtlxType == 'vector4':
            usdValue = Gf.Vec4f( mtlxValue[0], mtlxValue[1], mtlxValue[2], mtlxValue[3] )

        return usdValue

    def mapMtlxToUsdShaderNotation(self, name):
        '''
        Utility to map from MaterialX shader notation to Usd notation
        '''
        if name == 'surfaceshader': 
            name = 'surface'
        elif name == 'displacementshader':
            name = 'displacement'
        elif name == 'volumshader':
            name = 'volume'
        return name

    def emitUsdConnections(node, stage, rootPath):
        ''' 
        Emit connections between MaterialX elements as Usd connections for 
        a given MaterialX node.

        Parameters:
        - node: 
            MaterialX node to examine
        - stage:
            Usd stage to write connection to
        - rootPath:
            root path for connections
        '''
        if not node:
            return
        
        materialPath = None
        if node.getType() == 'material':
            materialPath = node.getName()

        for valueElement in node.getActiveValueElements():
            isInput = valueElement.isA(mx.Input) 
            isOutput = valueElement.isA(mx.Output)
            if  isInput or isOutput:

                interfacename = ''

                # Find out what type of element is connected to upstream:
                # node, nodegraph, or interface input.
                mtlxConnection = valueElement.getAttribute('nodename')
                if not mtlxConnection:
                    mtlxConnection = valueElement.getAttribute('nodegraph')
                if not isOutput:
                    if not mtlxConnection:
                        mtlxConnection = valueElement.getAttribute('interfacename')
                        interfacename = mtlxConnection 

                connectionPath = ''
                if mtlxConnection:

                    # Handle input connection by searching for the appropriate parent node.
                    # - If it's an interface input we want the parent nodegraph. Otherwise
                    # we want the node or nodegraph specified above.
                    # - If the parent path is the root (getNamePath() is empty), then this is to 
                    # nodes at the root document level. 
                    if isInput:
                        parent = node.getParent()
                        if parent.getNamePath():
                            if interfacename:
                                connectionPath = rootPath + parent.getNamePath()
                            else:
                                connectionPath = rootPath + parent.getNamePath() + '/' + mtlxConnection
                        else:
                            # The connectio is to a prim at the root level so insert a '/' identifier
                            # as getNamePath() will return an empty string at the root Document level.
                            if interfacename:
                                connectionPath = rootPath
                            else:
                                connectionPath = rootPath + mtlxConnection

                    # Handle output connection by looking for sibling elements
                    else:
                        parent = node.getParent()                    
                        
                        # Connection is to sibling under the same nodegraph
                        if node.isA(mx.NodeGraph):
                            connectionPath = rootPath + node.getNamePath() + '/' + mtlxConnection
                        else:
                            # Connection is to a nodegraph parent of the current node 
                            if parent.getNamePath():
                                connectionPath = rootPath + parent.getNamePath() + '/' + mtlxConnection
                            # Connection is to the root document.
                            else:
                                connectionPath = rootPath + mtlxConnection

                    # Find the source prim
                    # Assumes that the source is either a nodegraph, a material or a shader
                    connectionPath = connectionPath.removesuffix('/')
                    sourcePrim = None
                    sourcePort = 'out'
                    source = stage.GetPrimAtPath(connectionPath)
                    if not source:
                        if materialPath:
                            connectionPath = '/' + materialPath + connectionPath
                            source = stage.GetPrimAtPath(connectionPath)
                            if not source:
                                source = stage.GetPrimAtPath = '/' + materialPath
                    if source:
                        if source.IsA(UsdShade.Material): 
                            sourcePrim = UsdShade.Material(source)
                        elif source.IsA(UsdShade.NodeGraph):
                            sourcePrim = UsdShade.NodeGraph(source)
                        elif source.IsA(UsdShade.Shader): 
                            sourcePrim = UsdShade.Shader(source)

                        # Special case handle interface input vs an output
                        if interfacename:
                            sourcePort =  interfacename
                        else:                          
                            sourcePort = valueElement.getAttribute('output')
                            if not sourcePort:
                                sourcePort = 'out'
                        if sourcePort:
                            mtlxConnection = mtlxConnection + '. Port:' + sourcePort

                    else:
                        print('> Failed to find source at path:', connectionPath)

                    # Find destination prim and port and make the appropriate connection.
                    # Assumes that the destination is either a nodegraph, a material or a shader
                    destInput = None
                    if sourcePrim:
                        dest = stage.GetPrimAtPath(rootPath + node.getNamePath())
                        if not dest:
                            print('> Failed to find dest at path:', rootPath + node.getNamePath())
                        else:
                            destPort = None
                            portName = valueElement.getName()
                            destNode = None
                            if dest.IsA(UsdShade.Material): 
                                destNode = UsdShade.Material(dest)
                            elif dest.IsA(UsdShade.NodeGraph):
                                destNode = UsdShade.NodeGraph(dest)
                            elif dest.IsA(UsdShade.Shader): 
                                destNode = UsdShade.Shader(dest)
                            else:
                                print('> Encountered unsupport destinion type')

                            # Find downstream port (input or output)
                            if destNode:
                                if isInput:
                                    # Map from MaterialX to Usd connection syntax
                                    if dest.IsA(UsdShade.Material):
                                        portName = mapMtlxToUsdShaderNotation(portName)
                                        portName = 'mtlx:' + portName
                                        destPort = destNode.GetOutput(portName) 
                                    else:
                                        destPort = destNode.GetInput(portName) 
                                else:
                                    destPort = destNode.GetOutput(portName)                                

                            # Make connection to interface input, or node/nodegraph output
                            if destPort:
                                if interfacename:
                                    interfaceInput = sourcePrim.GetInput(sourcePort) 
                                    if interfaceInput:
                                        if not destPort.ConnectToSource(interfaceInput):
                                            print('> Failed to connect: ', source.GetPrimPath(), '-->', destPort.GetFullName())
                                else:
                                    sourcePrimAPI = sourcePrim.ConnectableAPI()
                                    if not destPort.ConnectToSource(sourcePrimAPI, sourcePort):
                                        print('> Failed to connect: ', source.GetPrimPath(), '-->', destPort.GetFullName())
                            else:
                                print('> Failed to find destination port:', portName)

    def emitUsdValueElements(self, node, usdNode, emitAllValueElements):
        '''
        Emit MaterialX value elements in Usd.

        Parameters
        ------------    
        node: 
            MaterialX node with value elements to scan
        usdNode:
            UsdShade node to create value elements on.
        emitAllValueElements: bool
            Emit value elements based on node definition, even if not specified on node instance.      
        '''
        if not node:
            return    
        
        isMaterial = node.getType() == 'material'
    
        # Instantiate with all the nodedef inputs (if emitAllValueELements is True).
        # Note that outputs are always created.
        nodedef = node.getNodeDef()
        if nodedef and not isMaterial:
            for valueElement in nodedef.getActiveValueElements():
                if valueElement.isA(mx.Input):
                    if emitAllValueElements:
                        mtlxType = valueElement.getType()
                        usdType = self.mapMtxToUsdType(mtlxType)

                        portName = valueElement.getName()
                        usdInput = usdNode.CreateInput(portName, usdType)

                        if len(valueElement.getValueString()) > 0:
                            mtlxValue = valueElement.getValue()
                            usdValue = self.mapMtxToUsdValue(mtlxType, mtlxValue)
                            if usdValue != '__':
                                usdInput.Set(usdValue)

                elif not isMaterial and valueElement.isA(mx.Output):
                    usdOutput = usdNode.CreateOutput(valueElement.getName(), self.mapMtxToUsdType(valueElement.getType()))

                else:
                    print('- Skip mapping of definition element: ', valueElement.getName(), '. Type: ', valueElement.getCategory())

        # From the given instance add inputs and outputs and set values.
        # This may override the default value specified on the definition.
        for valueElement in node.getActiveValueElements():
            if valueElement.isA(mx.Input):
                mtlxType = valueElement.getType()
                usdType = self.mapMtxToUsdType(mtlxType)
                portName = valueElement.getName()
                if isMaterial:
                    # Map from Materials to Usd notation
                    portName = self.mapMtlxToUsdShaderNotation(portName)    
                    usdInput = usdNode.CreateOutput('mtlx:' + portName, usdType)
                else:            
                    usdInput = usdNode.CreateInput(portName, usdType)

                # Set value. Note that we check the length of the value string
                # instead of getValue() as a 0 value will be skipped.
                if len(valueElement.getValueString()) > 0:
                    mtlxValue = valueElement.getValue()
                    usdValue = self.mapMtxToUsdValue(mtlxType, mtlxValue)
                    if usdValue != '__':
                        usdInput.Set(usdValue)

            elif not isMaterial and valueElement.isA(mx.Output):
                usdOutput = usdNode.GetInput(valueElement.getName())
                if not usdOutput:
                    usdOutput = usdNode.CreateOutput(valueElement.getName(), self.mapMtxToUsdType(valueElement.getType()))

            else:
                print('- Skip mapping of element: ', valueElement.getNamePath(), '. Type: ', valueElement.getCategory())

    def emitUsdShaderGraph(self, doc, stage, mxnodes, emitAllValueElements):
        '''
        Emit Usd shader graph to a given stage from a list of MaterialX nodes.

        Parameters
        ------------    
        doc: 
            MaterialX source document
        stage:
            Usd target stage
        mxnodes:
            MaterialX shader nodes.
        emitAllValueElements: bool
            Emit value elements based on node definition, even if not specified on node instance.      
        '''
        materialPath = None

        for v in mxnodes:
            elem = doc.getDescendant(v)
            if elem.getType() == 'material':    
                materialPath = elem.getName()
                break
                
        # Emit Usd nodes
        for v in mxnodes:
            elem = doc.getDescendant(v)

            # Note that MaterialX does not use absolute path notation while Usd
            # does. This will result in an error when trying set the path
            usdPath = '/' + elem.getNamePath()

            nodeDef = None
            usdNode = None
            if elem.getType() == 'material':
                usdNode = UsdShade.Material.Define(stage, usdPath)                
            elif elem.isA(mx.Node):
                nodeDef = elem.getNodeDef()
                if materialPath:
                    elemPath = '/' + materialPath + usdPath
                else:
                    elemPath = usdPath
                usdNode = UsdShade.Shader.Define(stage, elemPath)
            elif elem.isA(mx.NodeGraph):
                if materialPath:
                    elemPath = '/' + materialPath + usdPath
                else:
                    elemPath = usdPath
                usdNode = UsdShade.NodeGraph.Define(stage, elemPath)

            if usdNode:
                if nodeDef:
                    usdNode.SetShaderId(nodeDef.getName())
                self.emitUsdValueElements(elem, usdNode, emitAllValueElements)

        # Emit connections between Usd nodes
        for v in mxnodes:
            elem = doc.getDescendant(v)
            usdPath = '/' + elem.getNamePath()

            if elem.getType() == 'material':
                self.emitUsdConnections(elem, stage, '/')                
            elif elem.isA(mx.Node):
                self.emitUsdConnections(elem, stage, '/' + materialPath + '/')                
            elif elem.isA(mx.NodeGraph):
                self.emitUsdConnections(elem, stage, '/' + materialPath + '/')                

    def findMaterialXNodes(self, doc):
        '''
        Find all nodes in a MaterialX document
        '''
        visitedNodes = []
        treeIter = doc.traverseTree()
        for elem in treeIter:
            path = elem.getNamePath()
            if path in visitedNodes:
                continue
            visitedNodes.append(path)
        return visitedNodes

    def emit(self, mtlxFileName, emitAllValueElements):
        '''
        Read in a MaterialX file and emit it to a new Usd Stage
        Dump results for display and save to usda file.

        Parameters:
        -----------
        mtlxFileName : string
            Name of file containing MaterialX document. Assumed to end in ".mtlx"
        emitAllValueElements: bool
            Emit value elements based on node definition, even if not specified on node instance.         
        '''
        stage = Usd.Stage.CreateInMemory()
        
        doc = mx.createDocument()
        mtlxFilePath = mx.FilePath(mtlxFileName)
        if not mtlxFilePath.exists():
            print('Failed to read file: ', mtlxFilePath.asString())
            return
        
        # Find nodes to transform before importing the definition library
        mx.readFromXmlFile(doc, mtlxFileName)
        mxnodes = self.findMaterialXNodes(doc)

        stdlib = MtlxToUsd.createLibraryDocument()        
        doc.setDataLibrary(stdlib)
        
        # Translate
        self.emitUsdShaderGraph(doc, stage, mxnodes, emitAllValueElements)        

        usdFile = mtlxFileName.removesuffix('.mtlx')
        usdFile = usdFile + '.usda'
        stage.Export(usdFile, False)

        return stage
