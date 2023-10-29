# -*- coding: utf-8 -*-
"""
Documentation:
"""

import re

import MaterialX

UDIM_TOKEN = '.<UDIM>.'

class UDIMFile(MaterialX.FilePath):

    def __init__(self, pathstring):
        super().__init__(pathstring)

        self._isudim = False
        self._udim_files = []
        self.regex = re.compile(r"\.\d+\.")

        self.udimFiles()

    def udimFiles(self):
        texture_dir = self.getParentPath()
        texture_name = self.getBaseName()
        texture_extension = self.getExtension()

        if not self.regex.search(texture_name):
            # non udims files
            self._udim_files = [self]

        self._isudim = True
        fullname_pattern = self.regex.sub(self.regex.pattern.replace('\\', '\\\\'),
                                          texture_name)

        udim_files = filter(
            lambda f: re.search(fullname_pattern, f.asString()),
            texture_dir.getFilesInDirectory(texture_extension)
        )
        self._udim_files = [texture_dir / f for f in udim_files]

    def asPattern(self):

        if not self._isudim:
            return self.asPattern()

        texture_dir = self.getParentPath()
        texture_name = self.getBaseName()

        pattern = texture_dir / MaterialX.FilePath(self.regex.sub(UDIM_TOKEN, texture_name))
        return pattern.asString()

    def isUDIM(self):
        return self._isudim

    def getNumberOfUDIMs(self):
        return len(self._udim_files)

    def getUdimFiles(self):
        return self._udim_files

    def getUdimNumbers(self):
        def _extract_udim_number(_file):
            pattern = self.regex.search(_file.getBaseName())
            if pattern:
                return re.search(r"\d+", pattern.group()).group()

        return list(map(_extract_udim_number, self._udim_files))

    def getNameWithoutExtension(self):
        if self._isudim:
            name = self.regex.split(self.getBaseName())[0]
        else:
            name = self.getBaseName().rsplit('.', 1)[0]

        return re.sub(r'[^\w\s]+', '_', name)


def get_texture_files(texture_dir: MaterialX.FilePath, extension='png'):
    textures = [texture_dir / f for f in texture_dir.getFilesInDirectory(extension)]

    textures_files = []
    while textures:
        texture_file = UDIMFile(textures[0].asString())
        textures_files.append(texture_file)
        for udim_file in texture_file.getUdimFiles():
            textures.remove(udim_file)

    return textures_files


if __name__ == '__main__':
    print(__name__)
