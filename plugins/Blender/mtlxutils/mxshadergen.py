
import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
import MaterialX.PyMaterialXGenGlsl as mx_gen_glsl
import MaterialX.PyMaterialXGenOsl as mx_gen_osl
import MaterialX.PyMaterialXGenMdl as mx_gen_mdl

# Version check
from mtlxutils.mxbase import *
supportsMSL = haveVersion(1, 38, 7)
if supportsMSL:
    import MaterialX.PyMaterialXGenMsl as mx_gen_msl

from collections import OrderedDict

class MtlxShaderGen:

    def __init__(self, stdlib):
        self.implementations = None
        self.targets = None
        self.implcount = None

        self.generatordict = {}
        self.generator = None
        self.context = None

        self.registry = None

        self.shader = None
        self.source = []

        self.stdlib = stdlib

    def setup(self):
        self.implmentations = self.stdlib.getImplementations()
        self.targets = self.getTargetDefs(self.stdlib)

        # Set up generators
        self.initializeGenerators()

        # Setup units
        self.setupUnits(self.stdlib)

    def getTargetDefs(self, doc):
        targets = []
        for element in doc.getChildren():
            if element.getCategory() == 'targetdef':
                targets.append(element.getName())
        return targets

    def getShaderGenTarget(self, target):

        for target in self.targets:
            self.implcount = 0
            # Find out how many implementations we have 
            for impl in self.implmentations:
                gentarget = target
                if target == 'essl':
                    gentarget = 'genglsl'
                if impl.getTarget() == gentarget:
                    self.implcount = self.implcount + 1
        #print('Found target identifier:', target, 'with', implcount, 'source implementations.')      


    def initializeGenerators(self):
        # Create all generators
        generators = []
        generators.append(mx_gen_osl.OslShaderGenerator.create())
        generators.append(mx_gen_mdl.MdlShaderGenerator.create())
        generators.append(mx_gen_glsl.GlslShaderGenerator.create())
        #generators.append(mx_gen_glsl.EsslShaderGenerator.create())
        #generators.append(mx_gen_glsl.VkShaderGenerator.create())
        if supportsMSL:
            generators.append(mx_gen_msl.MslShaderGenerator.create())

        # Create a dictionary based on target identifier
        self.generatordict.clear()
        for gen in generators:
            self.generatordict[gen.getTarget()] = gen

    def setGeneratorForLanguage(self, language='glsl', wantCM=True):
        target = 'genglsl'
        if language == 'osl':
            target = 'genosl'
        elif language == 'mdl':
            target = 'genmdl'
        elif language == 'essl':
            target = 'essl'
        elif language == 'msl':
            target = 'genmsl'
        elif language in ['glsl', 'vulkan']:
            target = 'genglsl'

        return self.setGeneratorForTarget(target)

    def setGeneratorForTarget(self, target, wantCM=True):
        if target:
            self.generator = self.generatordict[target]
            if self.generator: 
                self.context = mx_gen_shader.GenContext(self.generator)
                # Set up CM and Units per generator
                if wantCM:
                    self.setupColorManagement(self.generator, self.stdlib)
                self.setUnitSystem(self.generator, self.stdlib)
        
        return self.context

    def getContext(self):
        return self.context

    def registerSourceCodeSearchPath(self, searchPath):
        # Register a path to where implmentations can be found.
        if self.context:
            self.context.registerSourceCodeSearchPath(searchPath)

    def setupColorManagement(self, generator, doc):
        # Create default CMS
        cms = mx_gen_shader.DefaultColorManagementSystem.create(generator.getTarget())  
        # Indicate to the CMS where definitions can be found
        cms.loadLibrary(doc)
        # Indicate to the code generator to use this CMS
        generator.setColorManagementSystem(cms)

    def getColorManagement(self):
        cms = self.generator.getColorManagementSystem()
        return cms

    def setupUnits(self, doc):

        # Create unit registry
        self.registry = mx.UnitConverterRegistry.create()
        if self.registry:
            # Get distance and angle unit type definitions and create a linear converter for each
            distanceTypeDef = doc.getUnitTypeDef('distance')
            if distanceTypeDef:
                self.registry.addUnitConverter(distanceTypeDef, mx.LinearUnitConverter.create(distanceTypeDef))
            angleTypeDef = doc.getUnitTypeDef('angle')
            if angleTypeDef:
                self.registry.addUnitConverter(angleTypeDef, mx.LinearUnitConverter.create(angleTypeDef))

    def setUnitSystem(self, generator, stdlib):

        # Create unit system, set where definitions come from, and
        # set up what registry to use, and set on the given generator
        unitsystem = mx_gen_shader.UnitSystem.create(generator.getTarget())
        if unitsystem:
            unitsystem.loadLibrary(stdlib)
            unitsystem.setUnitConverterRegistry(self.registry)
            generator.setUnitSystem(unitsystem)

    def setTargetDistanceUnit(self, unit='meter'):
        # Set the target scene unit to be `meter` on the context options
        genoptions = self.context.getOptions()
        genoptions.targetDistanceUnit = unit

    def getShaderNodes(self, doc):
        # Look for shader nodes in a document
        shaderNodes = []
        for node in doc.getNodes():
            if node.getType() == mx.SURFACE_SHADER_TYPE_STRING:
                shaderNodes.append(node)
        return shaderNodes

    def findRenderableElements(self, doc):
        # Look for renderable nodes
        self.nodes = mx_gen_shader.findRenderableElements(doc, False)
        if not self.nodes:
            self.nodes = doc.getMaterialNodes()
            if not self.nodes:
                self.nodes = self.getShaderNodes(doc)

        return self.nodes

    def generateShader(self, node):
        self.shader = None
        errors = ''
        nodeName = node.getName() if node else ''
        if nodeName:
            shaderName = mx.createValidName(nodeName)
            try:
                self.shader = self.generator.generate(shaderName, node, self.context)
            except LookupError as err:
                errors = err
        
        return self.shader, errors

    def getSourceCode(self, shader):
        self.source.clear()
        if shader:
            self.source.append(shader.getSourceCode(mx_gen_shader.VERTEX_STAGE))
            self.source.append(shader.getSourceCode(mx_gen_shader.PIXEL_STAGE))

        return self.source

    ##############################################################################
    ## For later....
    def getDownstreamPorts(self, doc, nodeName):
        downstreamPorts = []
        for port in doc.getMatchingPorts(nodeName):
            #print('- check port:', port)
            #print('- Compare: ', port.getConnectedNode().getName(), ' vs ', nodeName)
            #if port.getConnectedNode().getName() == nodeName:
            downstreamPorts.append(port)
        return downstreamPorts


    def getDownstreamNodes(self, doc, node, foundPorts, foundNodes, renderableElements, 
                        renderableTypes = ['material', 'surfaceshader', 'volumeshader']):
        """
        For a given "node", traverse downstream connections until there are none to be found.
        Along the way collect a list of ports and corresponding nodes visited (in order), and
        a list of "renderable" elements. 
        """
        testPaths = set()
        testPaths.add(node.getNamePath())

        while testPaths:
            nextPaths = set()
            for path in testPaths:
                testNode = doc.getDescendant(path)
                #print('test node:', testNode.getName())
                ports = []
                if testNode.isA(mx.Node):
                    ports = testNode.getDownstreamPorts()
                else:
                    ports = self.getDownstreamPorts(doc, testNode.getName())
                for port in ports:
                    downNode = port.getParent()
                    downNodePath = downNode.getNamePath()
                    if downNode and downNodePath not in nextPaths: #and downNode.isA(mx.Node):
                        foundPorts.append(port.getNamePath())
                        if port.isA(mx.Output):
                            renderableElements.append(port.getNamePath())
                        nodedef = downNode.getNodeDef()
                        if nodedef:
                            nodetype = nodedef.getType()
                            if nodetype in renderableTypes:
                                renderableElements.append(port.getNamePath())
                        foundNodes.append(downNode.getNamePath())
                        nextPaths.add(downNodePath)

            testPaths = nextPaths    

    def examineNodes(self, nodes):
        """
        Traverse downstream for a set of nodes to find information
        Returns the set of common ports, nodes, and renderables found 
        """
        commonPorts = []
        commonNodes = []
        commonRenderables = []
        for node in nodes:
            foundPorts = []
            foundNodes = []
            renderableElements = []
            self.getDownstreamNodes(node, foundPorts, foundNodes, renderableElements)

            foundPorts = list(OrderedDict.fromkeys(foundPorts))
            foundNodes = list(OrderedDict.fromkeys(foundNodes))
            renderableElements = list(OrderedDict.fromkeys(renderableElements))
            print('Traverse downstream from node: ', node.getNamePath())
            print('- Downstream ports:', ', '.join(foundPorts))
            print('- Downstream nodes:', ', '.join(foundNodes))
            print('- Renderable elements:', ', '.join(renderableElements))
            commonPorts.extend(foundPorts)
            commonNodes.extend(foundNodes)
            commonRenderables.extend(renderableElements)

        commonPorts = list(OrderedDict.fromkeys(commonPorts))
        commonNodes = list(OrderedDict.fromkeys(commonNodes))
        commonRenderables = list(OrderedDict.fromkeys(commonRenderables))

        return commonPorts, commonNodes, commonRenderables

