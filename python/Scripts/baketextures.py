#!/usr/bin/env python
'''
Generate the baked mtlx file and baked textures programmatically.
'''

import sys, os, string
import argparse
import struct
import MaterialX as mx
from MaterialX import PyMaterialXRender as mx_render
from MaterialX import PyMaterialXRenderGlsl as mx_render_glsl
from MaterialX import PyMaterialXGenShader as ms_gen_shader

def main():
    parser = argparse.ArgumentParser(description="Bake out MaterialX sharderref inputs into textures.")
    parser.add_argument("--hdr", "--HDR", dest="hdr", action="store_true", help="Save images to hdr format.")
    parser.add_argument("--t", "--texRes", "--texres", dest="tex_res", type=int, default=1024, help="Resolution at which to save textures.")
    parser.add_argument(dest="filename", help="Input mtlx filename.")

    opts = parser.parse_args()

    doc = mx.createDocument()

    try:
        mx.readFromXmlFile(doc, opts.filename)
    except mx.ExceptionFileMissing as err:
        print(err)

    stdlib = mx.createDocument()
    library_folder = [
        mx.FilePath("../../libraries/stdlib"),
        mx.FilePath("../../libraries/pbrlib"),
        mx.FilePath("../../libraries/bxdf"), 
        mx.FilePath("../../libraries/pbrlib/genglsl")
        ]
    searchPath = mx.FileSearchPath(os.getcwd())
    mx.loadLibraries(library_folder, searchPath, stdlib, set(), None)
    doc.importLibrary(stdlib)

    mx_render_glsl.TextureBaker.bakeAndSave(doc, opts.filename, opts.hdr, opts.tex_res)

if __name__ == '__main__':
    main()