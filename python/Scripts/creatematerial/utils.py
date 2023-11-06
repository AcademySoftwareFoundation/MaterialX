# -*- coding: utf-8 -*-
"""
Documentation:
"""

import os
import re
import json
import logging
from typing import Dict

import MaterialX

# Configure the logger
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
)

logger = logging.getLogger('creatematerial')


class Constant:
    UDIM_TOKEN = '.<UDIM>.'
    UDIM_REGEX = r'\.\d+\.'

    TEXTURE_PATTERNS_SEARCH_PATH = 'MATERIALX_TEXTURE_PATTERNS_PATH'

    METHOD_KEY = 'method'
    METHOD_APPEND = 'append'
    METHOD_PREPEND = 'prepend'
    METHOD_OVERRIDE = 'override'

class UDIMFile(MaterialX.FilePath):

    def __init__(self, pathstring):
        super().__init__(pathstring)

        self._isudim = False
        self._udim_files = []
        self.regex = re.compile(Constant.UDIM_REGEX)

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

        pattern = texture_dir / MaterialX.FilePath(self.regex.sub(Constant.UDIM_TOKEN, texture_name))
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


def read_json(json_path: str) -> Dict:
    """
    Read a JSON file and return its content as a dictionary.
    If there is a JSON decoding error, return an empty dictionary.
    """
    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except json.JSONDecodeError:
        logger.debug("Configuration JSON file decoder error: `{}`".format(json_path))
        data = {}
    return data


def apply_method(base_item: Dict, item: Dict, method: str) -> None:
    """
    Apply the method (append, prepend, or override) to the given item.

    @param base_item: The base dictionary item to apply the changes in.
    @param item: The dictionary item to get the data from.
    @param method: The methode of combining ["append", "prepend", "override"]

    return: (None) Apply the changes inplace in base_item

    """
    if method == Constant.METHOD_APPEND:

        for key in item:
            sub_item = item[key]
            if isinstance(sub_item, dict):
                sub_method = sub_item.get(Constant.METHOD_KEY, Constant.METHOD_APPEND)

                if not base_item.get(key):
                    base_item[key] = {}

                apply_method(base_item[key], sub_item, sub_method)

            elif isinstance(sub_item, list):
                base_item[key] = list(set(base_item.get(key, []) + sub_item))

            else:
                pass

    if method == Constant.METHOD_PREPEND:

        for key in item:
            sub_item = item[key]
            if isinstance(sub_item, dict):
                sub_method = sub_item.get(Constant.METHOD_KEY, Constant.METHOD_APPEND)

                if not base_item.get(key):
                    base_item[key] = {}

                apply_method(base_item[key], sub_item, sub_method)

            elif isinstance(sub_item, list):
                base_item[key] = list(set(sub_item + base_item.get(key, [])))

            else:
                base_item[key] = sub_item

    if method == Constant.METHOD_OVERRIDE:

        for key in item:
            sub_item = item[key]
            if isinstance(sub_item, dict):
                base_item[key] = sub_item

            elif isinstance(sub_item, list):
                base_item[key] = sub_item

            else:
                base_item[key] = sub_item


def combine_configration(base_config: Dict, added_config: Dict) -> Dict:
    """
    Combine two configurations from two JSON files.

    @param base_config: The base dictionary configration to apply the changes in.
    @param added_config: The dictionary configration to get the data from.

    return: (Dict) the final cooked dictionary
    """
    for key in added_config:
        item = added_config[key]

        if key == Constant.METHOD_KEY:
            continue
        if key not in base_config:
            base_config[key] = item
            continue

        method = item.get(Constant.METHOD_KEY, Constant.METHOD_APPEND)
        apply_method(base_config[key], item, method)

    return base_config


def get_configration() -> Dict:
    """
    Get the configuration from a JSON file combined with all user-specified JSON files.
    """
    base_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'texture_patterns.json')

    if not os.path.exists(base_path):
        raise RuntimeError("Can not find the base configration JSON file")

    base_config = read_json(base_path)

    # search_paths = MaterialX.getEnvironmentPath('MATERIALX_TEXTURE_PATTERN_PATH')
    config_paths = os.getenv(Constant.TEXTURE_PATTERNS_SEARCH_PATH, "")
    logger.debug("Using `{}`=`{}`".format(Constant.TEXTURE_PATTERNS_SEARCH_PATH, config_paths))

    for config_path in config_paths.split(MaterialX.PATH_LIST_SEPARATOR):
        if not os.path.exists(config_path):
            logger.debug("Cannot find the configration file: `{}`".format(config_path))
            continue

        logger.debug("Read Configration from: `{}`".format(config_path))
        combine_configration(base_config, read_json(config_path))

    return base_config


if __name__ == '__main__':
    print(__name__)
