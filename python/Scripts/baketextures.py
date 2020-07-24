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
    parser.add_argument("--incl", "--i", dest="include", default = "", help="Txt document with each library path on each line")
    parser.add_argument("--tx", "--texResX", "--texresx", dest="tex_res_x", type=int, default=1024, help="X Resolution at which to save textures.")
    parser.add_argument("--ty", "--texResY", "--texresy", dest="tex_res_y", type=int, default=1024, help="Y Resolution at which to save textures.")
    parser.add_argument(dest="filename", help="Input mtlx filename.")

    opts = parser.parse_args()

    doc = mx.createDocument()
    mxversion = mx.getVersionString()

    try:
        mx.readFromXmlFile(doc, opts.filename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    stdlib = mx.createDocument()
    library_folder = [
        mx.FilePath("../../libraries/stdlib"),
        mx.FilePath("../../libraries/pbrlib"),
        mx.FilePath("../../libraries/bxdf"), 
        mx.FilePath("../../libraries/pbrlib/genglsl")
        ]
    if (opts.include != ""):
        with open(opts.include) as f:
            for line in f:
                library_folder.append(mx.FilePath(line.rstrip()))
    searchPath = mx.FileSearchPath(os.getcwd())
    mx.loadLibraries(library_folder, searchPath, stdlib, set(), None)
    doc.importLibrary(stdlib)
    rc = doc.validate()

    if (len(rc) == 0 or not rc[0]):
        print("%s is not a valid MaterialX %s document:" % (opts.filename, mxversion))
        print(rc[1])
        sys.exit(0)

    mx_render_glsl.TextureBaker.bakeAllShaders(doc, opts.filename, opts.hdr, opts.tex_res_x, opts.tex_res_y)

if __name__ == '__main__':
    main()

