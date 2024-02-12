#!/usr/bin/env python
'''
Construct a MaterialX file from the textures in the given folder, using the standard data libraries
to determine the shading model and inputs they are most likely to reference.
'''

import os
import re
import argparse
from difflib import SequenceMatcher

from typing import Dict, List

import MaterialX as mx

UDIM_TOKEN = '.<UDIM>.'
UDIM_REGEX = r'\.\d+\.'
TEXTURE_EXTENSIONS = [ "exr", "png", "jpg", "jpeg", "tif", "hdr" ]

class UdimFile(mx.FilePath):

    def __init__(self, pathString):
        super().__init__(pathString)

        self._isUdim = False
        self._udimFiles = []
        self._udimRegex = re.compile(UDIM_REGEX)

        self.udimFiles()

    def __str__(self):
        return self.asPattern()

    def udimFiles(self):
        textureDir = self.getParentPath()
        textureName = self.getBaseName()
        textureExtension = self.getExtension()

        if not self._udimRegex.search(textureName):
            # non udims files
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

    def asPattern(self, format=mx.FormatPosix):

        if not self._isUdim:
            return self.asString(format=format)

        textureDir = self.getParentPath()
        textureName = self.getBaseName()

        pattern = textureDir / mx.FilePath(
            self._udimRegex.sub(UDIM_TOKEN, textureName))
        return pattern.asString(format=format)

    def isUdim(self):
        return self._isUdim

    def getNumberOfUdims(self):
        return len(self._udimFiles)

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

def listTextures(textureDir: mx.FilePath, texturePrefix=None) -> List[UdimFile]:
    """
    List all textures that matched extensions in cfg file
    @param textureDir: the directory where the textures exist
    @param texturePrefix: Get only textures that have prefix, if None, will return all textures
    return List(UdimFile)
    """
    texturePrefix = texturePrefix or ""
    allTextures = []
    for ext in TEXTURE_EXTENSIONS:
        textures = [textureDir / f for f in textureDir.getFilesInDirectory(ext)
                    if f.asString().lower().startswith(texturePrefix.lower())]

        while textures:
            textureFile = UdimFile(textures[0].asString())
            allTextures.append(textureFile)
            for udimFile in textureFile.getUdimFiles():
                textures.remove(udimFile)

    return allTextures

def getShadingModels() -> Dict[str, mx.NodeDef]:
    """
    To get all shading models nodeDefs that registers in materialx
    return (dict[str, mx.NodeDef]):  dictionary with nodedef name as a key and value of nodedef itself
    """
    stdlib = mx.createDocument()
    searchPath = mx.getDefaultDataSearchPath()
    libraryFolders = mx.getDefaultDataLibraryFolders()

    shadingModelFiles = mx.loadLibraries(libraryFolders, searchPath, stdlib)

    shadingModels = {}
    for nodeDef in stdlib.getNodeDefs():
        if nodeDef.getInheritsFrom():
            nodeDef = nodeDef.getInheritsFrom()
        shadingModels[nodeDef.getNodeString()] = nodeDef

    return shadingModels

def findBestMatch(textureName: str, shadingModel: mx.NodeDef):
    """
    Get the texture matched pattern and return the materialx plug name and the plug type
    @param textureName: The base texture name
    @param shadingModel: The shading model
    return: list(materialInputName, materialInputType) | None
    """

    parts = textureName.rsplit("_")

    baseTexName = parts[-1]
    if baseTexName.lower() == 'color':
        baseTexName = ''.join(parts[-2:])

    shaderInputs = shadingModel.getInputs()
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

def createMtlxDoc(textureFiles: List[mx.FilePath],
                  mtlxFile: mx.FilePath,
                  shadingModel: str,
                  relativePaths: bool = True,
                  colorspace: str = 'srgb_texture',
                  useTileImage: bool = False
                  ) -> mx.FilePath:
    """
    Create a MaterialX document with uber shader
    @param textureFiles: List of all textures
    @param mtlxFile: The output path of document
    @param relativePaths: Will create relative texture path or not inside document
    @param colorspace: The given color space
    @param useTileImage: use tileImage node or image node
    """

    udimNumbers = set()

    allShadingModels = getShadingModels()
    shadingModelNodeDef = allShadingModels.get(shadingModel)

    if not shadingModelNodeDef:
        print('Shading model', shadingModel, 'not found in the MaterialX data libraries')
        return

    # Create Document
    doc = mx.createDocument()

    mtlxFilename = mtlxFile.getBaseName().rsplit('.', 1)[0]
    mtlxFilename = doc.createValidChildName(mtlxFilename)

    # Create node graph and material
    graphName = doc.createValidChildName('NG_' + mtlxFilename)
    nodeGraph = doc.getNodeGraph(graphName)
    if not nodeGraph:
        nodeGraph = doc.addNodeGraph(graphName)

    shaderNode = doc.addNode(shadingModel, 'SR_' + mtlxFilename, 'surfaceshader')
    doc.addMaterialNode('M_' + mtlxFilename, shaderNode)

    for textureFile in textureFiles:
        textureName = textureFile.getNameWithoutExtension()
        shaderInput = findBestMatch(textureName, shadingModelNodeDef)

        if not shaderInput:
            print('Skipping', textureFile.getBaseName(), 'which does not match any', shadingModel, 'input')
            continue

        inputName = shaderInput.getName()
        inputType = shaderInput.getType()

        # Create textures
        # Skip if the plug already created (in the case of UDIMs)
        if shaderNode.getInput(inputName) or nodeGraph.getChild(textureName):
            continue

        # TODO :: create displacement shader

        plugName = shaderNode.createValidChildName(inputName)
        mtlInput = shaderNode.addInput(plugName)
        textureName = nodeGraph.createValidChildName(textureName)

        imageType = 'tileimage' if useTileImage else 'image'
        imageNode = nodeGraph.addNode(imageType, textureName, inputType)

        # set color space
        if 'color' in inputType.lower():
            imageNode.setColorSpace(colorspace)

        # set file path
        filePathString = textureFile.asPattern()

        # set relative path
        if relativePaths:
            filePathString = os.path.relpath(filePathString, mtlxFile.getParentPath().asString())

        imageNode.setInputValue('file', filePathString, 'filename')

        # in-between nodes
        inputNode = imageNode
        connNode = imageNode

        inBetweenNodes = []
        if re.search(r'(?i)normal', inputName):
            inBetweenNodes = ["normalmap"]

        for inNodeName in inBetweenNodes:
            connNode = nodeGraph.addNode(inNodeName, textureName + '_' + inNodeName,
                                         inputType)
            connNode.setConnectedNode('in', inputNode)
            inputNode = connNode

        # outputs
        outputNode = nodeGraph.addOutput(textureName + '_output', inputType)

        outputNode.setConnectedNode(connNode)
        mtlInput.setConnectedOutput(outputNode)
        mtlInput.setType(inputType)

        if textureFile.isUdim():
            udimNumbers.update(set(textureFile.getUdimNumbers()))

    # Create udim set
    if udimNumbers:
        geomInfoName = doc.createValidChildName('GI_' + mtlxFilename)
        geomInfo = doc.addGeomInfo(geomInfoName)
        geomInfo.setGeomPropValue('udimset', list(udimNumbers), "stringarray")

    # Write mtlx file
    if not mtlxFile.getParentPath().exists():
        mtlxFile.getParentPath().createDirectory()

    mx.writeToXmlFile(doc, mtlxFile.asString())
    print('Created MaterialX file:', mtlxFile.asString())
    return mtlxFile

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-o', '--outputFilename', dest='outputFilename', type=str, help='Filename of the output materialX document (default material.mtlx).')
    parser.add_argument('-s', '--shadingModel', dest='shadingModel', type=str, default="standard_surface", help='The shader model to use like standard_surface, UsdPreviewSurface, gltf_pbr, and open_pbr_surface (standard_surface).')
    parser.add_argument('-c', '--colorSpace', dest='colorSpace', type=str, help='Colorsapce to set (default to `srgb_texture`).')
    parser.add_argument('-p', '--texturePrefix', dest='texturePrefix', type=str, help='Use textures that have the prefix.')
    parser.add_argument('-a', '--absolutePaths', dest='absolutePaths', action="store_true", help='Make the texture paths absolute inside the materialX file.')
    parser.add_argument('-t', '--tileimage', dest='tileimage', action="store_true", help='Use tileimage node instead of image node.')
    parser.add_argument(dest='inputDirectory', nargs='?', help='Directory that contain textures (default to current working directory).')
    # TODO : Flag for SG names to be created in mtlx file. default, djed SG pattern or first match.
    # TODO : Flag to seperate each SG for mtlx with the name of each shading group, default combined in one mtlx file name by output file name.
    # TODO : Flag for forcing texture extension if there are multiple extensions in directory.

    options = parser.parse_args()

    texturePath = mx.FilePath.getCurrentPath()

    if options.inputDirectory:
        texturePath = mx.FilePath(options.inputDirectory)
        if not texturePath.isDirectory():
            print('Input folder not found:', texturePath)
            return

    default_doc_name = mx.FilePath('material.mtlx')
    mtlxFile = texturePath / default_doc_name
    if options.outputFilename:
        mtlxFile = mx.FilePath(options.outputFilename)

    textureFiles = listTextures(texturePath, texturePrefix=options.texturePrefix)
    if not textureFiles:
        print('No matching textures found in input folder.')
        return

    # Get shader model
    shadingModel = 'standard_surface'
    if options.shadingModel:
        shadingModel = options.shadingModel

    # Colorspace
    colorspace = 'srgb_texture'
    if options.colorSpace:
        colorspace = options.colorSpace

    createMtlxDoc(
        textureFiles,
        mtlxFile,
        shadingModel,
        relativePaths=not options.absolutePaths,
        colorspace=colorspace,
        useTileImage=options.tileimage)

if __name__ == '__main__':
    main()
