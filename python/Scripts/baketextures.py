#!/usr/bin/env python
'''
Generate a baked version of each material in the input document, using the TextureBaker class in the MaterialXRenderGlsl library.
'''

import sys, os, string
import argparse
import struct
import MaterialX as mx
from MaterialX import PyMaterialXRender as mx_render
from MaterialX import PyMaterialXRenderGlsl as mx_render_glsl
from MaterialX import PyMaterialXGenShader as ms_gen_shader

def main():
    parser = argparse.ArgumentParser(description="Generate a baked version of each material in the input document.")
    parser.add_argument("--width", dest="width", type=int, default=1024, help="Specify the width of baked textures.")
    parser.add_argument("--height", dest="height", type=int, default=1024, help="Specify the height of baked textures.")
    parser.add_argument("--hdr", dest="hdr", action="store_true", help="Save images to hdr format.")
    parser.add_argument("--path", dest="paths", action='append', nargs='+', help="An additional absolute search path location (e.g. '/projects/MaterialX')")
    parser.add_argument("--library", dest="libraries", action='append', nargs='+', help="An additional relative path to a custom data library folder (e.g. 'libraries/custom')")
    parser.add_argument(dest="input_filename", help="Filename of the input document.")
    parser.add_argument(dest="output_filename", help="Filename of the output document.")

    opts = parser.parse_args()

    doc = mx.createDocument()
    mxversion = mx.getVersionString()

    try:
        mx.readFromXmlFile(doc, opts.input_filename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    stdlib = mx.createDocument()
    search_path = mx.FileSearchPath()
    library_folders = [ mx.FilePath("libraries") ]
    if opts.paths:
        for path_list in opts.paths:
            for path in path_list:
                search_path.append(path)
    search_path.append(os.path.join(os.getcwd(), '../..'))
    if opts.libraries:
        for library_list in opts.libraries:
            for library in library_list:
                library_folders.append(library)
    mx.loadLibraries(library_folders, search_path, stdlib, set(), None)
    doc.importLibrary(stdlib)

    valid, msg = doc.validate()
    if (not valid):
        print("Validation warnings for input document:")
        print(msg)

    image_search_path = mx.FileSearchPath(os.path.dirname(opts.input_filename))
    base_type = mx_render.BaseType.FLOAT if opts.hdr else mx_render.BaseType.UINT8
    baker = mx_render_glsl.TextureBaker.create(opts.width, opts.height, base_type)
    baker.bakeAllMaterials(doc, image_search_path, opts.output_filename)

if __name__ == '__main__':
    main()
