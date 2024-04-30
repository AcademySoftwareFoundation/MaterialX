#!/usr/bin/env python
'''
Construct a MaterialX file from the textures in the given folder, using the standard data libraries
to build a mapping from texture filenames to shader inputs.

By default the standard_surface shading model is assumed, with the --shadingModel option used to
select any other shading model in the data libraries.
'''

import os
import re
import argparse
from difflib import SequenceMatcher

import MaterialX as mx

UDIM_TOKEN = '.<UDIM>.'
UDIM_REGEX = r'\.\d+\.'
TEXTURE_EXTENSIONS = [ "exr", "png", "jpg", "jpeg", "tif", "hdr" ]
INPUT_ALIASES = { "roughness": "specular_roughness" }

class UdimFilePath(mx.FilePath):

    def __init__(self, pathString):
        super().__init__(pathString)

        self._isUdim = False
        self._udimFiles = []
        self._udimRegex = re.compile(UDIM_REGEX)

        textureDir = self.getParentPath()
        textureName = self.getBaseName()
        textureExtension = self.getExtension()

        if not self._udimRegex.search(textureName):
            self._udimFiles = [self]
            return

        self._isUdim = True
        fullNamePattern = self._udimRegex.sub(self._udimRegex.pattern.replace('\\', '\\\\'),
                                              textureName)

        udimFiles = filter(
            lambda f: re.search(fullNamePattern, f.asString()),
            textureDir.getFilesInDirectory(textureExtension)
        )
        self._udimFiles = [textureDir / f for f in udimFiles]
        
    def __str__(self):
        return self.asPattern()

    def asPattern(self):
        if not self._isUdim:
            return self.asString()

        textureDir = self.getParentPath()
        textureName = self.getBaseName()

        pattern = textureDir / mx.FilePath(
            self._udimRegex.sub(UDIM_TOKEN, textureName))
        return pattern.asString()

    def isUdim(self):
        return self._isUdim

    def getUdimFiles(self):
        return self._udimFiles

    def getUdimNumbers(self):
        def _extractUdimNumber(_file):
            pattern = self._udimRegex.search(_file.getBaseName())
            if pattern:
                return re.search(r"\d+", pattern.group()).group()

        return list(map(_extractUdimNumber, self._udimFiles))

    def getNameWithoutExtension(self):
        if self._isUdim:
            name = self._udimRegex.split(self.getBaseName())[0]
        else:
            name = self.getBaseName().rsplit('.', 1)[0]

        return re.sub(r'[^\w\s]+', '_', name)

def listTextures(textureDir, texturePrefix=None):
    '''
    Return a list of texture filenames matching known extensions.
    '''
    
    texturePrefix = texturePrefix or ""
    allTextures = []
    for ext in TEXTURE_EXTENSIONS:
        textures = [textureDir / f for f in textureDir.getFilesInDirectory(ext)
                    if f.asString().lower().startswith(texturePrefix.lower())]
        while textures:
            textureFile = UdimFilePath(textures[0].asString())
            allTextures.append(textureFile)
            for udimFile in textureFile.getUdimFiles():
                textures.remove(udimFile)
    return allTextures

def findBestMatch(textureName, shadingModel):
    '''
    Given a texture name and shading model, return the shader input that is the closest match.
    '''
    
    parts = textureName.rsplit("_")

    baseTexName = parts[-1]
    if baseTexName.lower() == 'color':
        baseTexName = ''.join(parts[-2:])
    if baseTexName in INPUT_ALIASES:
        baseTexName = INPUT_ALIASES.get(baseTexName.lower())

    shaderInputs = shadingModel.getActiveInputs()
    ratios = []
    for shaderInput in shaderInputs:
        inputName = shaderInput.getName()
        inputName = re.sub(r'[^a-zA-Z0-9\s]', '', inputName).lower()
        baseTexName = re.sub(r'[^a-zA-Z0-9\s]', '', baseTexName).lower()

        sequenceScore = SequenceMatcher(None, inputName, baseTexName).ratio()
        ratios.append(sequenceScore * 100)

    highscore = max(ratios)
    if highscore < 50:
        return None

    idx = ratios.index(highscore)
    return shaderInputs[idx]

def buildDocument(textureFiles, mtlxFile, shadingModel, colorspace, useTiledImage):
    '''
    Build a MaterialX document from the given textures and shading model.
    '''

    # Find the default library nodedef, if any, for the requested shading model.
    stdlib = mx.createDocument()
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), mx.getDefaultDataSearchPath(), stdlib)
    matchingNodeDefs = stdlib.getMatchingNodeDefs(shadingModel)
    if not matchingNodeDefs:
        print('Shading model', shadingModel, 'not found in the MaterialX data libraries')
        return None
    shadingModelNodeDef = matchingNodeDefs[0]
    for nodeDef in matchingNodeDefs:
        if nodeDef.getAttribute('isdefaultversion') == 'true':
            shadingModelNodeDef = nodeDef

    # Create content document.
    doc = mx.createDocument()
    materialName = mx.createValidName(mtlxFile.getBaseName().rsplit('.', 1)[0])
    nodeGraph = doc.addNodeGraph('NG_' + materialName)
    shaderNode = doc.addNode(shadingModel, 'SR_' + materialName, 'surfaceshader')
    doc.addMaterialNode('M_' + materialName, shaderNode)

    # Iterate over texture files.
    imageNodeCategory = 'tiledimage' if useTiledImage else 'image'
    udimNumbers = set()
    for textureFile in textureFiles:
        textureName = textureFile.getNameWithoutExtension()
        shaderInput = findBestMatch(textureName, shadingModelNodeDef)

        if not shaderInput:
            print('Skipping', textureFile.getBaseName(), 'which does not match any', shadingModel, 'input')
            continue

        inputName = shaderInput.getName()
        inputType = shaderInput.getType()

        # Skip inputs that have already been created, e.g. in multi-UDIM materials.
        if shaderNode.getInput(inputName) or nodeGraph.getChild(textureName):
            continue

        mtlInput = shaderNode.addInput(inputName)
        textureName = nodeGraph.createValidChildName(textureName)
        imageNode = nodeGraph.addNode(imageNodeCategory, textureName, inputType)

        # Set color space.
        if shaderInput.isColorType():
            imageNode.setColorSpace(colorspace)

        # Set file path.
        filePathString = os.path.relpath(textureFile.asPattern(), mtlxFile.getParentPath().asString())
        imageNode.setInputValue('file', filePathString, 'filename')

        # Apply special cases for normal maps.
        inputNode = imageNode
        connNode = imageNode
        inBetweenNodes = []
        if inputName.endswith('normal') and shadingModel == 'standard_surface':
            inBetweenNodes = ["normalmap"]
        for inNodeName in inBetweenNodes:
            connNode = nodeGraph.addNode(inNodeName, textureName + '_' + inNodeName, inputType)
            connNode.setConnectedNode('in', inputNode)
            inputNode = connNode

        # Create output.
        outputNode = nodeGraph.addOutput(textureName + '_output', inputType)
        outputNode.setConnectedNode(connNode)
        mtlInput.setConnectedOutput(outputNode)
        mtlInput.setType(inputType)

        if textureFile.isUdim():
            udimNumbers.update(set(textureFile.getUdimNumbers()))

    # Create udim set
    if udimNumbers:
        geomInfoName = doc.createValidChildName('GI_' + materialName)
        geomInfo = doc.addGeomInfo(geomInfoName)
        geomInfo.setGeomPropValue('udimset', list(udimNumbers), "stringarray")

    # Return the new document
    return doc

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--outputFilename', dest='outputFilename', type=str, help='Filename of the output MaterialX document.')
    parser.add_argument('--shadingModel', dest='shadingModel', type=str, default="standard_surface", help='The shading model used in analyzing input textures.')
    parser.add_argument('--colorSpace', dest='colorSpace', type=str, help='The colorspace in which input textures should be interpreted, defaulting to srgb_texture.')
    parser.add_argument('--texturePrefix', dest='texturePrefix', type=str, help='Filter input textures by the given prefix.')
    parser.add_argument('--tiledImage', dest='tiledImage', action="store_true", help='Request tiledimage nodes instead of image nodes.')
    parser.add_argument(dest='inputDirectory', nargs='?', help='Input folder that will be scanned for textures, defaulting to the current working directory.')

    options = parser.parse_args()

    texturePath = mx.FilePath.getCurrentPath()
    if options.inputDirectory:
        texturePath = mx.FilePath(options.inputDirectory)
        if not texturePath.isDirectory():
            print('Input folder not found:', texturePath)
            return

    mtlxFile = texturePath / mx.FilePath('material.mtlx')
    if options.outputFilename:
        mtlxFile = mx.FilePath(options.outputFilename)

    textureFiles = listTextures(texturePath, texturePrefix=options.texturePrefix)
    if not textureFiles:
        print('No matching textures found in input folder.')
        return

    # Get shading model and color space.
    shadingModel = 'standard_surface'
    colorspace = 'srgb_texture'
    if options.shadingModel:
        shadingModel = options.shadingModel
    if options.colorSpace:
        colorspace = options.colorSpace
    print('Analyzing textures in the', texturePath.asString(), 'folder for the', shadingModel, 'shading model.')

    # Create the MaterialX document.
    doc = buildDocument(textureFiles, mtlxFile, shadingModel, colorspace, options.tiledImage)
    if not doc:
        return

    if options.outputFilename:
        # Write the document to disk.
        if not mtlxFile.getParentPath().exists():
            mtlxFile.getParentPath().createDirectory()
        mx.writeToXmlFile(doc, mtlxFile.asString())
        print('Wrote MaterialX document to disk:', mtlxFile.asString())
    else:
        # Print the document to the standard output.
        print('Generated MaterialX document:')
        print(mx.writeToXmlString(doc))

if __name__ == '__main__':
    main()
