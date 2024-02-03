import os
import re
import logging
from difflib import SequenceMatcher

from typing import Dict

import MaterialX
from utils import UdimFile, listTextures

logger = logging.getLogger('createMaterial')


def getShaderModels() -> Dict[str, MaterialX.NodeDef]:
    stdlib = MaterialX.createDocument()
    searchPath = MaterialX.getDefaultDataSearchPath()
    libraryFolders = MaterialX.getDefaultDataLibraryFolders()

    shaderModelFiles = MaterialX.loadLibraries(libraryFolders, searchPath, stdlib)

    shaderModels = {}
    for nodeDef in stdlib.getNodeDefs():
        if nodeDef.getInheritsFrom():
            nodeDef = nodeDef.getInheritsFrom()

        shaderModels[nodeDef.getNodeString()] = nodeDef

    return shaderModels


def findBestMatch(textureFile: UdimFile,
                  shaderModel: MaterialX.NodeDef) -> None | MaterialX.PyMaterialXCore.Input:
    """
    Get the texture matched pattern and return the materialx plug name and the plug type
    @param textureFile: The texture filename
    @param shaderModel: The shader model
    return: list(materialInputName, materialInputType)
    """

    baseTexName = textureFile.getNameWithoutExtension().rsplit("_", 1)[-1]

    shaderInputs = shaderModel.getInputs()
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


def createMtlxDoc(textureDir: MaterialX.FilePath,
                  mtlxFile: MaterialX.FilePath,
                  shaderModel: str,
                  relativePaths: bool = True,
                  colorspace: str = 'srgb_texture',
                  useTileImage: bool = False
                  ) -> None:
    """
    Create a metrical document with uber shader
    @param textureDir: The texture path directory
    @param mtlxFile: The output path of document
    @param relativePaths: Will create relative texture path or not inside document
    @param colorspace: The given color space
    @param useTileImage: use tileImage node or image node
    """

    udimNumbers = set()
    textureFiles = listTextures(textureDir)

    if not textureFiles:
        logger.warning(
            "The directory does not contain any matched texture with given cfg file.. Skipping")
        return

    allShadersModels = getShaderModels()
    shaderModelNodeDef = allShadersModels.get(shaderModel)

    if not shaderModelNodeDef:
        logger.warning(
            f"Shader model {shaderModel} doesn't exist in materialx libraries.. Skipping")
        return

    # Create Document
    doc = MaterialX.createDocument()

    mtlxFilename = mtlxFile.getBaseName().rsplit('.', 1)[0]
    mtlxFilename = doc.createValidChildName(mtlxFilename)

    # Create node graph and material
    graphName = doc.createValidChildName('NG_' + mtlxFilename)
    nodeGraph = doc.getNodeGraph(graphName)
    if not nodeGraph:
        nodeGraph = doc.addNodeGraph(graphName)

    shaderNode = doc.addNode(shaderModel, 'SR_' + mtlxFilename, 'surfaceshader')
    doc.addMaterialNode('M_' + mtlxFilename, shaderNode)

    for textureFile in textureFiles:

        shaderInput = findBestMatch(textureFile, shaderModelNodeDef)

        if not shaderInput:
            logger.debug(
                "Skipping `{}` not matched any {} inputs".format(textureFile.getBaseName(),
                                                                 shaderModel))
            continue

        inputName = shaderInput.getName()
        inputType = shaderInput.getType()

        # Create textures

        # Skip if the plug already created (in the case of UDIMs)
        textureName = textureFile.getNameWithoutExtension()
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

    MaterialX.writeToXmlFile(doc, mtlxFile.asString())
    logger.info("MaterialX file created `{}`".format(mtlxFile.asString()))
