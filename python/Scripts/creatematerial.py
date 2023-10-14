# -*- coding: utf-8 -*-
__doc__ = 'Create materialX file conatin standard surface uber shader with input texture for given directory'

import os
import re
import json
import argparse

import logging
from typing import List, Dict

import MaterialX as mx

# Configure the logger
logging.basicConfig(
    level=logging.INFO,  # Set DEBUD for more logs
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
)

# Create a logger instance
logger = logging.getLogger('creatematerial')


def get_cfg() -> Dict:
    """
    Get the patterns and settings from cfg file
    return: dict of resultant settings
    """
    json_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'textures_patterns.json')
    # TODO: Add option here to take the user overrides customization file
    with open(json_path, 'r') as f:
        data = json.load(f)
    return data


def texture_type_from_name(texture_name: str) -> List[str]:
    """
    Get the texture matched pattern and return the materialx plug name and the plug type
    @param texture_name: The texture filename
    return: list(material_input_name, material_input_type)
    """
    settings = get_cfg()
    texture_patterns = settings.get('patterns')

    # reversed to take the color before weight e.g. specular_color before specular
    for plug_name in reversed(list(texture_patterns.keys())):
        regx_list = texture_patterns[plug_name]['values']
        if not isinstance(regx_list, list):
            continue

        for regex in regx_list:
            if re.search(regex, texture_name):
                return [plug_name, texture_patterns[plug_name]['type']]


def get_udim(texture_name: str) -> str:
    """
    Check and get the UDIM number like 1001, 1002 in case of UDIM
    @param texture_name: The texture filename
    return UDIM number if UDIM pattern detected and None if not UDIM
    """
    pattern = re.search(r"[.-_]\d+\.", texture_name)
    if pattern:
        return re.search(r"\d+", pattern.group()).group()


def list_textures(directory: str) -> List[str]:
    """
    List all textures that matched extensions in cfg file
    @param directory: the directory where the textures exist
    return List(texture_names)
    """
    img_extensions = get_cfg().get('texture_extensions')

    return [
        x for x in os.listdir(directory) if
        (os.path.isfile(os.path.join(directory, x)))
        and (x.split('.')[-1].lower() in img_extensions)
    ]


def create_mtlx_doc(texture_dir: str, mtlx_file: str, relative_paths: bool = True,
                    colorspace: str = 'srgb_texture') -> None:
    """
    Create a metrical document with uber shader
    @param texture_dir: The texture path directory
    @param mtlx_file: The output path of document
    @param relative_paths: Will create relative texture path or not inside document
    @param colorspace: The given colorspace
    """
    material_type = 'standard_surface'
    # Create document
    doc = mx.createDocument()

    mtlx_filename = os.path.basename(mtlx_file).rsplit('.', 1)[0]

    # Create node graph and material
    graph_name = 'NG_' + mtlx_filename
    node_graph = doc.getNodeGraph(graph_name)
    if not node_graph:
        node_graph = doc.addNodeGraph(graph_name)

    shader_node = doc.addNode(material_type, 'SR_' + mtlx_filename, 'surfaceshader')
    doc.addMaterialNode('M_' + mtlx_filename, shader_node)

    udim_numbers = set()
    texture_names =  list_textures(texture_dir)

    if not texture_names:
        logger.warning("The directory does not contain any matched texture with given cfg file.. Skipping")
        return

    for tex_file_name in texture_names:
        tex_path = os.path.join(texture_dir, tex_file_name)
        tex_name = tex_file_name.rsplit('.', 1)[0]

        # set relative path
        if relative_paths:
            tex_path = os.path.relpath(tex_path, os.path.dirname(mtlx_file))

        # Check UDIM
        udim_num = get_udim(tex_file_name)
        if udim_num:
            logger.debug("Texture UDIM: {} {}".format(udim_num, tex_file_name))
            tex_path = re.sub(udim_num, "<UDIM>", tex_path)
            udim_numbers.add(udim_num)
            tex_name = tex_name.rsplit('.', 1)[0].replace('.', '_')

        mtl_plug = texture_type_from_name(tex_name)
        if not mtl_plug:
            logger.debug("Skipping `{}` not matched pattern detected".format(tex_file_name))
            continue

        # Create texture
        plug_name, plug_type = mtl_plug

        # Skip if the plug already created (in case of udim)
        if shader_node.getInput(plug_name):
            continue

        mtl_input = shader_node.addInput(plug_name)
        image_node = node_graph.addNode('image', tex_name, plug_type)

        # set path
        image_node.setInputValue('file', tex_path, 'filename')

        # set colorspace
        if 'color' in plug_type:
            image_node.setColorSpace(colorspace)

        conn_node = image_node
        if plug_name == 'normal':
            normal_map = node_graph.addNode('normalmap', tex_name + '_normal', plug_type)
            normal_map.setConnectedNode('in', image_node)
            conn_node = normal_map

        output_node = node_graph.addOutput(tex_name + '_output', plug_type)
        output_node.setConnectedNode(conn_node)
        mtl_input.setConnectedOutput(output_node)
        mtl_input.setType(plug_type)

    # Create udim set
    if udim_numbers:
        geom_info = doc.addGeomInfo('GI_' + mtlx_filename)
        geom_info.setGeomPropValue('udimset', list(udim_numbers), "stringarray")

    # Write mtlx file
    if not os.path.isdir(os.path.dirname(mtlx_file)):
        os.makedirs(os.path.dirname(mtlx_file))

    mx.writeToXmlFile(doc, mtlx_file)
    logger.info("MaterialX file created `{}`".format(mtlx_file))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-dir', '--directory', dest='dir', help='The directory where the textures in')
    parser.add_argument('-c', '--colorSpace', dest='colorSpace', help='Colorsapce to set (default to `srgb_texture`)')
    parser.add_argument('-a', '--absolutePaths', dest='absolutePaths', action="store_true",help='Make the texture paths absolute inside the materialX file')

    parser.add_argument(dest='inputDirectory', nargs='?', help='Directory that contain textures (default to current working directory).')
    parser.add_argument(dest='outputFilename', nargs='?', help='Filename of the output materialX document (default material.mtlx)')

    options = parser.parse_args()

    texture_dir = os.getcwd()
    if options.inputDirectory:
        texture_dir = options.inputDirectory
        if not os.path.isdir(texture_dir):
            logger.error("The texture directory does not exist `{}`".format(texture_dir))
            return

    mtlx_file = os.path.join(texture_dir, 'standard_surface.mtlx')
    if options.outputFilename:
        filename = options.outputFilename
        if os.path.abspath(filename):
            mtlx_file = filename
        else:
            mtlx_file = os.path.join(texture_dir, filename)

    # Colorspace
    colorspace = 'srgb_texture'
    if options.colorSpace:
        colorspace = options.colorSpace

    create_mtlx_doc(
        texture_dir,
        mtlx_file,
        relative_paths=not options.absolutePaths,
        colorspace=colorspace,
    )


if __name__ == '__main__':
    main()
