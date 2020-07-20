#!/usr/bin/env python
'''
Generate the baked mtlx file and baked textures programmatically.
'''

import sys, os, string
import glfw
import struct
import MaterialX as mx
from MaterialX import PyMaterialXRender as r
from MaterialX import PyMaterialXRenderGlsl as rg
from MaterialX import PyMaterialXGenShader as gs

def usage():
    print("baketextures.py: bake out MaterialX shaderref inputs into textures.")
    print("Usage:  baketextures.py [options] <file.mtlx>" )
    print("    -h[elp]         Print usage information")
    print("    -hdr            Save images to hdr format")
    print("    -t[exRes] <res> Resolution at which to save textures")

def main():
    if len(sys.argv) < 2:
        usage()
        sys.exit(0)

    hdr = False
    texres = 1024
    for i in range(1, len(sys.argv)):
        arg = sys.argv[i]
        if arg in ['-h', '-help', '--help']:
            usage()
            sys.exit(0)
        elif arg in ['-hdr', '-HDR']:
            hdr = True
        elif (arg in ['-t', 'texRes', 'texres'] and i + 1 < len(sys.argv)):
            texres = int(sys.argv[i + 1])
            i += 1
        else:
            filename = arg

    doc = mx.createDocument()

    try:
        mx.readFromXmlFile(doc, filename)
    except mx.ExceptionFileMissing as err:
        print(err)

    stdlib = mx.createDocument()
    libraryFolder = list([mx.FilePath("../../libraries/stdlib"), mx.FilePath("../../libraries/pbrlib"), mx.FilePath("../../libraries/bxdf"), mx.FilePath("../../libraries/pbrlib/genglsl")])
    searchPath = mx.FileSearchPath(os.getcwd())
    mx.loadLibraries(libraryFolder, searchPath, stdlib, set(), None)
    doc.importLibrary(stdlib)

    rg.TextureBaker.bakeAndSave(doc, filename, hdr, texres)

if __name__ == '__main__':
    glfw.init()
    main()