import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
import MaterialX.PyMaterialXGenGlsl as mx_gen_glsl
import MaterialX.PyMaterialXRender as mx_render
import MaterialX.PyMaterialXRenderGlsl as mx_render_glsl
from mtlxutils import mxshadergen
import inspect, os, sys, math

def buildUnitDict(doc):
    '''
    Sample code to examine unit types and unit name information
    '''
    unitdict = {}

    for ud in doc.getUnitDefs():
        unittype = ud.getAttribute('unittype')
        unitinfo = {}
        for unit in ud.getChildren():
            unitinfo[unit.getName()] = unit.getAttribute('scale')

        unitdict[unittype] = unitinfo
    return unitdict

def buildColorTransformDict(doc):
    colordict = {}
    targetdict = {}
    for cmnode in doc.getNodeDefs():
        if cmnode.getNodeGroup() == 'colortransform':
            name = cmnode.getName()
            name = name.removeprefix('ND_')
            namesplit = name.split('_to_')
            type = 'color3'
            if 'color4' in namesplit[1]:
                continue
            else:
                namesplit[1] = namesplit[1].removesuffix('_color3')

            sourceSpace = namesplit[0]
            targetSpace = namesplit[1]

            if sourceSpace in colordict:
                sourceItem = colordict[sourceSpace]
                sourceItem.append(targetSpace)
            else:
                colordict[sourceSpace] = [targetSpace]

            if targetSpace in targetdict:
                taregetItem = targetdict[targetSpace]
                taregetItem.append(sourceSpace)
            else:
                targetdict[targetSpace] = [sourceSpace]

    
    return colordict, targetdict

class GlslRenderer():
    '''
    Wrapper for GLSL sample renderer.

    Handles setup of image, geometry and light handlers as well as GLSL code and 
    program generation. 

    Calls into sample renderer to render and capture images as desired.
    '''
    
    def __init__(self, desiredRenderSize):
        # Renderer
        if desiredRenderSize[0] * desiredRenderSize[1] < 4:
            desiredRenderSize = self.getDefaultRenderSize()        
        self.renderSize = desiredRenderSize
        self.renderer = None

        # Code Generator
        self.mxgen = None 
        self.activeShader = None
        self.activeShaderErrors = ''
        self.sourceCode = {}

        # Image Handling
        self.capturedImage = None
        self.haveOIIOImageHandler = False
        mxrenderMembers = inspect.getmembers(sys.modules['MaterialX.PyMaterialXRender'])
        for className, classObject in mxrenderMembers:
            if className == 'OiioImageLoader' and inspect.isclass(classObject):
                self.haveOIIOImageHandler = True
                break

        # Geometry loading
        self.haveCGLTFLoader = False
        # Note: TODO: Test for existence of GLTF loader in Python module. This does not exist in a release currently.
        for className, classObject in mxrenderMembers:
            if className == 'CgltfLoader' and inspect.isclass(classObject):
                self.haveCGLTFLoader = True
                break

        # Light setup
        self.lightHandler = None

        # Units dictionary
        self.unitDict = None

        # Colorspace dictionaries
        self.sourceColorDict = None
        self.targetColorDict = None

        # Camera
        self.camera = mx_render.Camera.create()

        # Rendering log
        self.renderLog = []

    def getRenderer(self):
        return self.renderer
    
    def getDefaultRenderSize(self):
        return [512,512]
    
    def getCodeGenerator(self):
        return self.mxgen
    
    def getActiveShader(self):
        return self.activeShader

    def getActiveShaderErrors(self):
        return self.activeShaderErrors
    
    def setActiveShaderErrors(self, errors):
        self.activeShaderErrors = errors
    
    def getSourceCode(self):
        return self.sourceCode
    
    def haveGLTFLoader(self):
        return self.haveCGLTFLoader

    def haveOIIOLoader(self):
        return self.haveOIIOImageHandler
        
    def getLightHandler(self):
        return self.lightHandler
    
    def getRenderLog(self):
        return self.renderLog
    
    def addToRenderLog(self, msg):
        self.renderLog.append(msg)        
    
    def clearRenderLog(self):
        self.renderLog = []        

    def initialize(self, bufferFormat=mx_render.BaseType.UINT8):
        '''
        Setup sample renderer with a given frame buffer size.
        Initialize image and geometry handlers.
        '''
        self.renderer = mx_render_glsl.GlslRenderer.create(self.renderSize[0], self.renderSize[1], bufferFormat)
        if self.renderer:
            self.renderer.initialize()
            self.initializeGeometryHandler()

    def resize(self, w, h):
        '''
        Resize frame buffer. 
        Clears any cached captured image.
        '''
        if not self.renderer:
            return False
        
        self.renderer.setSize(w, h)
        self.capturedImage = None

    def updateCamera(self):
        '''
        Update camera with current renderer state.
        '''
        if not self.renderer:
            return False
        
        self.camera = self.renderer.getCamera()
        #self.camera.setViewport(mx_render.Viewport(0, 0, self.renderer.getWidth(), self.renderer.getHeight()))  

        DEFAULT_EYE_POSITION = mx.Vector3(0.0, 0.0, 3.0)
        DEFAULT_TARGET_POSITION = mx.Vector3(0.0, 0.0, 0.0)
        DEFAULT_UP_VECTOR = mx.Vector3(0.0, 1.0, 0.0)
        DEFAULT_FIELD_OF_VIEW = 45.0
        DEFAULT_NEAR_PLANE = 0.05
        DEFAULT_FAR_PLANE = 100.0

        # Compute bounding box geometry
        geometryHandler = self.renderer.getGeometryHandler()
        if geometryHandler:
            # TODO: This interface is missing from the Python API
            #geometryHandler.computeBounds()
            boxMax = geometryHandler.getMaximumBounds() 
            boxMin = geometryHandler.getMinimumBounds()
            sphereCenter = (boxMax + boxMin) * 0.5

            meshRotation = mx.Matrix44.createRotationY(0)
            meshScaleVal = mx.Vector3()
            maxVal = boxMax[0] - boxMin[0]
            if boxMax[1] - boxMin[1] > maxVal:
                maxVal = boxMax[1] - boxMin[1]
            if boxMax[2] - boxMin[2] > maxVal:
                maxVal = boxMax[2] - boxMin[2]
            if maxVal < 0.0001:
                maxVal = 1.0
            meshScaleVal[0] = 2.0 / maxVal
            meshScaleVal[1] = 2.0 / maxVal
            meshScaleVal[2] = 2.0 / maxVal
            meshScale = mx.Matrix44.createScale(meshScaleVal)
            meshTranslationValue = meshRotation.transformPoint(sphereCenter)
            meshTranslationValue[0] = -meshTranslationValue[0]
            meshTranslationValue[1] = -meshTranslationValue[1]
            meshTranslationValue[2] = -meshTranslationValue[2]
            meshTranslation = mx.Matrix44.createTranslation(meshTranslationValue)

            self.camera.setWorldMatrix(meshRotation * meshTranslation * meshScale)         

            fH = math.tan(DEFAULT_FIELD_OF_VIEW / 360.0 * math.pi) * DEFAULT_NEAR_PLANE;
            fW = fH * 1.0
            self.camera.setViewMatrix(mx_render.Camera.createViewMatrix(DEFAULT_EYE_POSITION, DEFAULT_TARGET_POSITION, DEFAULT_UP_VECTOR))
            self.camera.setProjectionMatrix(mx_render.Camera.createPerspectiveMatrix(-fW, fW, -fH, fH, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE));
            
            self.renderer.setCamera(self.camera)

        #aspectRatio = float(self.renderSize[0]) / float(self.renderSize[1])
        #self.camera.setNearDistance(0.01)
        #self.camera.setFarDistance(1000.0)
        #viewMatrix = mx.Matrix44.createLookAt(mx.Vector3(0.0, 0.0, 5.0), mx.Vector3(0.0, 0.0, 0.0), mx.Vector3(0.0, 1.0, 0.0))
        #self.renderer.updateCamera(self.camera)

        return True            
        

    def initializeImageHandler(self, searchPath):   
        '''
        Initialize image handler. 
        ''' 
        if self.renderer.getImageHandler():
            return
            
        # TODO: Missing fom the Python API for createImageHandler() 
        #imageHandler = renderer.createImageHandler()
        imageLoader = mx_render.StbImageLoader.create()
        imageHandler = mx_render_glsl.GLTextureHandler.create(imageLoader)
        # Add OIIO handler if it exists
        if self.haveOIIOImageHandler:
            imageHandler.addLoader(mx_render.OIIOHandler.create())

        if imageHandler:
            imageSearchPath = mx.FileSearchPath()
            imageSearchPath.append(searchPath)            
            imageHandler.setSearchPath(imageSearchPath)
            #self.addToRenderLog('- Create image loader with path: %s' % imageHandler.getSearchPath().asString())
            self.renderer.setImageHandler(imageHandler)

    def initializeGeometryHandler(self):        
        # renderer has a geometry handler created by
        # default so not need to call: mx_render.GeometryHandler.create()
        geometryHandler = self.renderer.getGeometryHandler()
        # TODO: Currently missing gltf loader from Python API
        if self.haveCGLTFLoader:
            gltfLoader = mx_render.CgltfLoader.create()
            geometryHandler.addLoader(gltfLoader)

    def loadGeometry(self, fileName):
        geometryHandler = self.renderer.getGeometryHandler()
        if geometryHandler:
            texcoordVerticalFlip = False # True if mx.FilePath(fileName).getExtension() == 'obj' else False
            if not geometryHandler.hasGeometry(fileName):
                geometryHandler.loadGeometry(fileName, texcoordVerticalFlip)

    def getGeometyHandler(self):
        return self.renderer.getGeometryHandler()

    def initializeLights(self, doc, enableDirectLighting, radianceIBLPath, irradianceIBLPath, enableReferenceQuality):
        if self.lightHandler:
            return
        
        # Create a light handler
        self.lightHandler = mx_render.LightHandler.create()

        # Scan for lights
        if enableDirectLighting:
            lights = []
            self.lightHandler.findLights(doc, lights)
            mxcontext = self.mxgen.getContext()
            self.lightHandler.registerLights(doc, lights, mxcontext)

            # Set the list of lights on the with the generator
            self.lightHandler.setLightSources(lights)

        # Load environment lights.
        imageHandler = self.renderer.getImageHandler()
        envRadiance = imageHandler.acquireImage(radianceIBLPath)
        envIrradiance = imageHandler.acquireImage(irradianceIBLPath)

        # Apply light settings for render tests.
        self.lightHandler.setEnvRadianceMap(envRadiance)
        self.lightHandler.setEnvIrradianceMap(envIrradiance)
        self.lightHandler.setEnvSampleCount(4096 if enableReferenceQuality else 16)
        # TODO: Check for 1.38.8
        self.lightHandler.setRefractionTwoSided(True)

    def captureImage(self):
        '''
        Capture the framebuffer contents to an image
        '''
        self.capturedImage = self.renderer.captureImage(self.capturedImage)

    def clearCaptureImage(self):
        '''
        Clear out any captured image
        '''
        self.captureImage = None

    def saveCapture(self, filePath, verticalFlip=True): 
        '''
        Save captured image to a file.
        Vertical flip image as needed.
        '''
        if not self.capturedImage:
            self.captureImage()
        
        imageHandler = self.renderer.getImageHandler()
        if imageHandler:
            imageHandler.saveImage(filePath, self.capturedImage, verticalFlip)            

    def getImageHandler(self):
        return self.renderer.getImageHandler()

    def getCapturedImage(self):
        return self.capturedImage

    def setupGenerator(self, stdlib, searchPath):
        '''
        Setup code generation. Returns the generator instantiated.
        Note: It is important to set up the source code path so that
        file implementations can be found.
        '''
        self.mxgen = mxshadergen.MtlxShaderGen(stdlib)
        self.mxgen.setup()

        # Check generator and generator options
        mxgenerator = None
        mxcontext = self.mxgen.setGeneratorForTarget('genglsl')
        if mxcontext:
            mxgenerator = mxcontext.getShaderGenerator()

        # Set source code path
        self.mxgen.registerSourceCodeSearchPath(searchPath)

        return mxgenerator

    def findRenderableElements(self, doc):
        # Generate shader for a given node
        self.nodes = self.mxgen.findRenderableElements(doc)
        return self.nodes

    def generateShader(self, node, targetColorSpaceOverride='lin_rec709', targetDistanceUnit='meter'):
        '''
        Generate new GLSL shader.
        - Inspects node to check if it requires lighting and / or is transparent.
        - Sets target colorspace and real-world units
        - Generates code and caches it
        - Caches the "active" Shader node
        '''
        self.activeShader = None
        if not node:
            return None
        
        # Set up generation options.
        # Detect requirement for shading and transparency.
        mxcontext = self.mxgen.getContext()
        mxoptions = mxcontext.getOptions()
        mxgenerator = mxcontext.getShaderGenerator()
        if not mx_gen_shader.elementRequiresShading(node):
            mxoptions.hwMaxActiveLightSources = 0
        else:
            mxoptions.hwMaxActiveLightSources = 0
        mxoptions.hwTransparency = mx_gen_shader.isTransparentSurface(node, mxgenerator.getTarget())

        # Check support of units and working color space
        doc = node.getDocument()
        if doc:
            buildUnitDict(doc)
            if self.unitDict:
                units = self.unitDict['distance']
                if targetDistanceUnit not in units:
                    targetDistanceUnit = 'meter'

            sdict, tdict = buildColorTransformDict(doc)
            if tdict:
                if targetColorSpaceOverride not in tdict:
                    targetColorSpaceOverride = 'lin_rec709'
        else:
            targetDistanceUnit = 'meter'
            targetColorSpaceOverride = 'lin_rec709'

        mxoptions.targetDistanceUnit = targetDistanceUnit
        mxoptions.targetColorSpaceOverride = targetColorSpaceOverride

        self.activeShader, self.activeShaderErrors = self.mxgen.generateShader(node)        
        if self.activeShader:
            self.sourceCode[mx_gen_shader.VERTEX_STAGE] = self.activeShader.getSourceCode(mx_gen_shader.VERTEX_STAGE)
            self.sourceCode[mx_gen_shader.PIXEL_STAGE] = self.activeShader.getSourceCode(mx_gen_shader.PIXEL_STAGE)

        return self.activeShader

    def createProgram(self):
        '''
        Create a GLSL program from the active shader node and validates it's inputs.
        Note: A light handler **must** be set to for validation to work properly.
        '''
        if not self.activeShader:
            return False
        
        self.renderer.setLightHandler(self.lightHandler)
        self.renderer.createProgram(self.activeShader)
        #self.renderer.validateInputs()

        program = self.renderer.getProgram()
        if program:
            return True
        else:
            return False

    def getProgram(self):
        if self.renderer:
            return self.renderer.getProgram() 

    def render(self):
        '''
        Render a frame.
        - Note: LookupError's are returned if any failure occurs.
        - Status and and any errors are returned. 
        '''
        if not self.renderer:
            return False, 'No renderer'
        
        # Render
        try:
            self.renderer.render()
        except mx.Exception as err:
            print('- Render error:', err)
            return False, err
        except LookupError as err:
            return False, err
        
        return True, ''

def getPortPath(inputPath, doc):
    '''
    Find any upstream interface input which maps to a given path
    '''
    if not inputPath:
        return inputPath, None
    
    input = doc.getDescendant(inputPath)
    if input:
        # Redirect to interface input if it exists.
        # TODO: This should be done during shader generation !
        interfaceInput = input.getInterfaceInput()
        if interfaceInput:
            input = interfaceInput
            return input.getNamePath(), interfaceInput

    return inputPath, None

def debugStages(shader, doc, filter='Public'):
    '''
    Scan through each stage of a shader and get the uniform blocks for each stage.
    For each block, print out list of assocaited ports.
    '''
    if not shader:
        return

    for i in range(0, shader.numStages()):
        stage = shader.getStage(i)
        if stage:
            print('Stage name: "%s"' % stage.getName())
            print('-' * 30)
            if stage.getName():
                for blockName in stage.getUniformBlocks():
                    block = stage.getUniformBlock(blockName)
                    if filter:
                        if filter not in block.getName():
                            continue                        
                    print('- Block: ', block.getName())  

                    for shaderPort in block:
                        variable = shaderPort.getVariable()
                        value = shaderPort.getValue().getValueString() if shaderPort.getValue() else '<NONE>'
                        origPath = shaderPort.getPath()
                        path, interfaceInput = getPortPath(shaderPort.getPath(), doc)                                                
                        if not path:
                            path = '<NONE>'
                        else:
                            if path != origPath:
                                path = origPath + ' --> ' + path
                        type = shaderPort.getType().getName()
                        print('  - Variable: %s. Value: (%s). Type: %s, Path: "%s"' % (variable, value, type, path))

                        unit = shaderPort.getUnit()
                        if interfaceInput:
                            colorspace = interfaceInput.getColorSpace()
                        else:
                            colorspace = shaderPort.getColorSpace() 
                        if unit or colorspace:                            
                            print('   - Unit:%s, ColorSpace:%s' % (unit,colorspace))

def initializeRenderer(stdlib, searchPath, 
                       radianceMapFileName, irrandianceMapFileName, w, h, desiredGeometry):
    glslRenderer = GlslRenderer([w,h])
    glslRenderer.initialize(mx_render.BaseType.UINT8)
    glslRenderer.addToRenderLog('- Initialized renderer')
    glslRenderer.addToRenderLog('------------------------')
    glslRenderer.addToRenderLog(' - Have OIIO loader support: %s' % glslRenderer.haveOIIOLoader()) 
    glslRenderer.addToRenderLog(' - Have GLTF loader support: %s' % glslRenderer.haveGLTFLoader()) 

    # TODO: This is not exposed in the Pyhon API
    #clearColor = mx.Color3(1.0, 1.0, 1.0)
    #glslRenderer.setScreenColor(clearColor)

    geometryHandler = glslRenderer.getGeometyHandler()
    if geometryHandler:
        glslRenderer.addToRenderLog('- Initialized geometry loader: ' + desiredGeometry)
        glslRenderer.loadGeometry(desiredGeometry)
        for mesh in geometryHandler.getMeshes():
            glslRenderer.addToRenderLog(' - Loaded Mesh: "%s"' % mesh.getName())

    # Update the camera parameters
    glslRenderer.updateCamera()

    # Set up image handler. Make sure to pass in a suitable search path for images
    glslRenderer.initializeImageHandler(searchPath)
    glslRenderer.addToRenderLog(' - Initialize image handler. Search path %s' % glslRenderer.getImageHandler().getSearchPath().asString())

    # Set up lighting
    enableReferenceQuality = False
    enableDirectLighting = False
    lightDocument = None
    glslRenderer.initializeLights(lightDocument, enableDirectLighting, 
                                  radianceMapFileName, irrandianceMapFileName, enableReferenceQuality)    
    lightHandler = glslRenderer.getLightHandler()
    if lightHandler:
        glslRenderer.addToRenderLog('- Setup lighting:')
        radMap = lightHandler.getEnvRadianceMap()
        irradMap = lightHandler.getEnvIrradianceMap()
        glslRenderer.addToRenderLog(' - Loaded radiance map: %d x %d' % (radMap.getWidth(), radMap.getHeight()))
        glslRenderer.addToRenderLog(' - Loaded irradiance map: %d x %d' % (irradMap.getWidth(), irradMap.getHeight()))

    # Set up source code generator. Make sure to set the source code path
    sourceCodeSearchPath = searchPath
    glslRenderer.setupGenerator(stdlib, sourceCodeSearchPath)
    context = glslRenderer.getCodeGenerator().getContext()
    if context:
        generator = context.getShaderGenerator()
        if generator:
            glslRenderer.addToRenderLog('- Iniitialize generator for target: %s.\n - Source path: %s' % 
                (generator.getTarget(), sourceCodeSearchPath.asString()))

    # Set up additional options for generation
    context = glslRenderer.getCodeGenerator().getContext()
    genOptions = context.getOptions()
    genOptions.emitColorTransforms = True # This is True by default
    genOptions.fileTextureVerticalFlip = True
    # TODO: This and a number of other options are not been exposed in the Python API
    #genOptions.addUpstreamDependencies = True
    # 

    return glslRenderer        

def performRender(glslRenderer, doc, inputFilename, outputPath, searchPath) -> ( bool, str ):

    rendered = False
    renderErrors = ''

    generator = glslRenderer.getCodeGenerator()
    context = generator.getContext()
    genOptions = context.getOptions()
    target = context.getShaderGenerator().getTarget()

    # Append to search path for image handler
    imageHandler = glslRenderer.getImageHandler()
    imageSearchPathPrev = imageHandler.getSearchPath()
    imageSearchPath = imageSearchPathPrev
    #imageSearchPath.append(searchPath)
    #imageHandler.setSearchPath(imageSearchPath)
    imageHandler.setSearchPath(searchPath)
    glslRenderer.addToRenderLog(' - Using image search path: %s' % imageHandler.getSearchPath().asString())

    # Append to source code search path
    # TODO: There is no way to get and search the path in the API (C++ or Python)
    # so this keeps on accumating patsh :(
    generator.registerSourceCodeSearchPath(searchPath)

    # Find a renderable and generate the shader for it
    nodes = glslRenderer.findRenderableElements(doc)
    if not nodes:
        return
    printSource = False

    # Set up overrides for color space and units. Color space may come from the document,
    # but units are a property of the application.
    targetColorSpaceOverride = 'lin_rec709'
    targetDistanceUnit = 'centimeter'
    for renderNode in nodes:
        shader = None
        if renderNode.getType() == 'material':
            renderNodes = mx.getShaderNodes(renderNode)
            if not renderNodes:
                glslRenderer.setActiveShaderErrors('- Warning: No surface shader found in material: "%s"' % renderNode.getNamePath())                    
                renderNode = None
        if renderNode:
            glslRenderer.addToRenderLog('------- Render Node: %s --------' % renderNode.getNamePath())
            shader = glslRenderer.generateShader(renderNode, targetColorSpaceOverride, targetDistanceUnit)
        if shader:
            glslRenderer.addToRenderLog('- Generate shader for node: "%s"\n\t- Is Transparent: %s. V-Flip textures: %d.\n\t- Emit Color Xforms: %d. Default input colorspace: "%s".\n\t- Scene Units: "%s"' %
                    (nodes[0].getNamePath(),
                    genOptions.hwTransparency, 
                    genOptions.fileTextureVerticalFlip, 
                    genOptions.emitColorTransforms,
                    genOptions.targetColorSpaceOverride, 
                    genOptions.targetDistanceUnit))
        else:
            glslRenderer.addToRenderLog(
                '- Failed to generate shader for node: "%s". Errors: %s' % (nodes[0].getNamePath(), 
                                                                            glslRenderer.getActiveShaderErrors()))
            continue

        if printSource:
            sourceCode = glslRenderer.getSourceCode()
            for stage in sourceCode:
                glslRenderer.addToRenderLog('-' * 80)
                glslRenderer.addToRenderLog('- "%s" Stage Code:' % stage)
                lines = sourceCode[stage].split('\n')
                for l in lines:
                    if l.startswith('uniform'):
                        print('  ', l)

        createdProgram = False
        if shader:
            createdProgram = glslRenderer.createProgram()

        printAttribs = False
        if createdProgram:
            glslRenderer.addToRenderLog('- Create renderer program from shader')

            program = glslRenderer.getProgram()
            if program:
                if printAttribs:
                    attribs = program.getAttributesList()
                    glslRenderer.addToRenderLog('%d geometry attribs in program' % len(attribs))   
                    for attrib in attribs:
                        glslRenderer.addToRenderLog('- attribute: %s' % attrib)
                        input = attribs[attrib] 
                    
                    uniforms = program.getUniformsList()
                    glslRenderer.addToRenderLog('%d uniforms' % len(uniforms))
                    for uniform in uniforms:
                        glslRenderer.addToRenderLog('- Uniform:', uniform)
                        port = uniforms[uniform]
                        glslRenderer.addToRenderLog('  - Port type:', port.gltype)   

        runRender = True
        if createdProgram and runRender:
            rendered, renderErrors = glslRenderer.render()
            if not rendered:
                glslRenderer.addToRenderLog('- Failed to render, Errors: %s' % renderErrors)
            #else:
            #    glslRenderer.addToRenderLog('- Successfully rendered frame.')

        if rendered:
            glslRenderer.captureImage()
            capturedImage = glslRenderer.getCapturedImage()
            if capturedImage:
                flipImage = True        
                outputString = mx.FilePath(mx.createValidName(renderNode.getNamePath()) + '_' + target)
                outputString.addExtension('png')
                if outputPath.size() > 0 and os.path.isdir(outputPath.asString()):
                    fileName = outputPath / outputString
                else:
                    fileName = mx.FilePath(inputFilename).getParentPath() / outputString
                glslRenderer.addToRenderLog('- Saved rendered image to: %s' % fileName.asString())                             
                glslRenderer.saveCapture(fileName, flipImage)
    
    # Restore image search path
    imageHandler.setSearchPath(imageSearchPathPrev)

    return rendered, renderErrors
