import os
import re
import logging

from typing import List

import MaterialX
from utils import UDIMFile, get_configration


SETTINGS = get_configration()

logger = logging.getLogger('creatematerial')

def texture_type_from_name(texture_name: str, shader_model:str) -> List[str]:
    """
    Get the texture matched pattern and return the materialx plug name and the plug type
    @param texture_name: The texture filename
    @param shader_model: The shader model name
    return: list(material_input_name, material_input_type)
    """

    texture_patterns = SETTINGS.get('shader_model').get(shader_model, {}).get('patterns', {})

    # reversed to take the color before weight e.g. specular_color before specular
    for plug_name in reversed(list(texture_patterns.keys())):
        regx_list = texture_patterns[plug_name]['values']
        if not isinstance(regx_list, list):
            continue

        for regex in regx_list:
            if re.search(regex, texture_name):
                return [plug_name, texture_patterns[plug_name]['type']]


def list_textures(texture_dir: MaterialX.FilePath) -> List[UDIMFile]:
    """
    List all textures that matched extensions in cfg file
    @param texture_dir: the directory where the textures exist
    return List(udimutils.UDIMFile)
    """
    tex_extensions = SETTINGS.get('textures').get('extensions', [])
    all_textures = []
    for ext in tex_extensions:
        textures = [texture_dir / f for f in texture_dir.getFilesInDirectory(ext)]

        while textures:
            texture_file = UDIMFile(textures[0].asString())
            all_textures.append(texture_file)
            for udim_file in texture_file.getUdimFiles():
                textures.remove(udim_file)

    return all_textures


def create_mtlx_doc(texture_dir: MaterialX.FilePath,
                    mtlx_file: MaterialX.FilePath,
                    shader_model: str,
                    relative_paths: bool = True,
                    colorspace: str = 'srgb_texture',
                    use_tile_image: bool =False
                    ) -> None:
    """
    Create a metrical document with uber shader
    @param texture_dir: The texture path directory
    @param mtlx_file: The output path of document
    @param relative_paths: Will create relative texture path or not inside document
    @param colorspace: The given colorspace
    @param use_tile_image: use tileimage node or image node
    """

    # Create document
    doc = MaterialX.createDocument()

    mtlx_filename = mtlx_file.getBaseName().rsplit('.', 1)[0]
    mtlx_filename = doc.createValidChildName(mtlx_filename)

    # Create node graph and material
    graph_name = doc.createValidChildName('NG_' + mtlx_filename)
    node_graph = doc.getNodeGraph(graph_name)
    if not node_graph:
        node_graph = doc.addNodeGraph(graph_name)

    shader_node = doc.addNode(shader_model, 'SR_' + mtlx_filename, 'surfaceshader')
    doc.addMaterialNode('M_' + mtlx_filename, shader_node)

    udim_numbers = set()
    texture_files = list_textures(texture_dir)

    if not texture_files:
        logger.warning(
            "The directory does not contain any matched texture with given cfg file.. Skipping")
        return

    for texture_file in texture_files:
        mtl_plug = texture_type_from_name(texture_file.getBaseName(), shader_model)

        if not mtl_plug:
            logger.debug("Skipping `{}` not matched pattern detected".format(texture_file.getBaseName()))
            continue

        # Create textures
        plug_name, plug_type = mtl_plug

        # Skip if the plug already created (in case of UDIMs)
        texture_name = texture_file.getNameWithoutExtension()
        if shader_node.getInput(plug_name) or node_graph.getChild(texture_name):
            continue

        if plug_name == 'displacement':
            # create displacement shader
            continue

        plug_name = shader_node.createValidChildName(plug_name)
        mtl_input = shader_node.addInput(plug_name)
        texture_name = node_graph.createValidChildName(texture_name)

        image_type = 'tileimage' if use_tile_image else 'image'
        image_node = node_graph.addNode(image_type, texture_name, plug_type)

        # set file path
        filepath_string = texture_file.asString(MaterialX.FormatPosix)

        # set relative path
        if relative_paths:
            filepath_string = os.path.relpath(filepath_string, mtlx_file.getParentPath().asString())

        image_node.setInputValue('file', filepath_string, 'filename')

        # set colorspace
        if 'color' in plug_type:
            image_node.setColorSpace(colorspace)

        # inbetween nodes
        input_node = image_node
        conn_node = image_node

        patterns = SETTINGS.get('shader_model').get(shader_model).get('patterns')
        inbetween_nodes = patterns.get(plug_name).get('inbetween_nodes', [])

        for in_node_name in inbetween_nodes:
            conn_node = node_graph.addNode(in_node_name, texture_name+'_'+in_node_name, plug_type)
            conn_node.setConnectedNode('in', input_node)
            input_node = conn_node

        # outputs
        output_node = node_graph.addOutput(texture_name + '_output', plug_type)
        output_node.setConnectedNode(conn_node)
        mtl_input.setConnectedOutput(output_node)
        mtl_input.setType(plug_type)

        if texture_file.isUDIM():
            udim_numbers.update(set(texture_file.getUdimNumbers()))

    # Create udim set
    if udim_numbers:
        geom_info_name = doc.createValidChildName('GI_' + mtlx_filename)
        geom_info = doc.addGeomInfo(geom_info_name)
        geom_info.setGeomPropValue('udimset', list(udim_numbers), "stringarray")

    # Write mtlx file
    if not mtlx_file.getParentPath().exists():
        mtlx_file.getParentPath().createDirectory()

    MaterialX.writeToXmlFile(doc, mtlx_file.asString())
    logger.info("MaterialX file created `{}`".format(mtlx_file.asString()))



