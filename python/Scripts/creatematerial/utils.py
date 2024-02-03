# -*- coding: utf-8 -*-
"""
Documentation:
"""

import os
import re
import logging
from typing import List

import MaterialX

# Configure the logger
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
)

logger = logging.getLogger('createMaterial')


class Constant:
    UdimToken = '.<UDIM>.'
    UdimRegex = r'\.\d+\.'
    TextureExtensions = [
        "exr",
        "png",
        "jpg",
        "jpeg",
        "tif",
        "hdr"
    ]


class UdimFile(MaterialX.FilePath):

    def __init__(self, pathString):
        super().__init__(pathString)

        self._isUdim = False
        self._udimFiles = []
        self._udimRegex = re.compile(Constant.UdimRegex)

        self.udimFiles()

    def udimFiles(self):
        textureDir = self.getParentPath()
        textureName = self.getBaseName()
        textureExtension = self.getExtension()

        if not self._udimRegex.search(textureName):
            # non udims files
            self._udimFiles = [self]

        self._isUdim = True
        fullNamePattern = self._udimRegex.sub(self._udimRegex.pattern.replace('\\', '\\\\'),
                                              textureName)

        udimFiles = filter(
            lambda f: re.search(fullNamePattern, f.asString()),
            textureDir.getFilesInDirectory(textureExtension)
        )
        self._udimFiles = [textureDir / f for f in udimFiles]

    def asPattern(self, format=MaterialX.FormatPosix):

        if not self._isUdim:
            return self.asPattern()

        textureDir = self.getParentPath()
        textureName = self.getBaseName()

        pattern = textureDir / MaterialX.FilePath(
            self._udimRegex.sub(Constant.UdimToken, textureName))
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


def listTextures(textureDir: MaterialX.FilePath) -> List[UdimFile]:
    """
    List all textures that matched extensions in cfg file
    @param textureDir: the directory where the textures exist
    return List(udimutils.UDIMFile)
    """

    allTextures = []
    for ext in Constant.TextureExtensions:
        textures = [textureDir / f for f in textureDir.getFilesInDirectory(ext)]

        while textures:
            textureFile = UdimFile(textures[0].asString())
            allTextures.append(textureFile)
            for udimFile in textureFile.getUdimFiles():
                textures.remove(udimFile)

    return allTextures


if __name__ == '__main__':
    print(__name__)
