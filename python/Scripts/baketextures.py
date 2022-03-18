#!/usr/bin/env python
'''
Generate a baked version of each material in the input document, using the TextureBaker class in the MaterialXRenderGlsl library.
'''

import sys, os, argparse
import MaterialX as mx
from MaterialX import PyMaterialXGenShader
from MaterialX import PyMaterialXGenGlsl
from MaterialX import PyMaterialXRender as mx_render
from MaterialX import PyMaterialXRenderGlsl as mx_render_glsl

def main():
    parser = argparse.ArgumentParser(description="Generate a baked version of each material in the input document.")
    parser.add_argument("--width", dest="width", type=int, default=1024, help="Specify the width of baked textures.")
    parser.add_argument("--height", dest="height", type=int, default=1024, help="Specify the height of baked textures.")
    parser.add_argument("--hdr", dest="hdr", action="store_true", help="Save images to hdr format.")
    parser.add_argument("--average", dest="average", action="store_true", help="Average baked images to generate constant values.")
    parser.add_argument("--path", dest="paths", action='append', nargs='+', help="An additional absolute search path location (e.g. '/projects/MaterialX')")
    parser.add_argument("--library", dest="libraries", action='append', nargs='+', help="An additional relative path to a custom data library folder (e.g. 'libraries/custom')")
    parser.add_argument(dest="inputFilename", help="Filename of the input document.")
    parser.add_argument(dest="outputFilename", help="Filename of the output document.")
    opts = parser.parse_args()

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    stdlib = mx.createDocument()
    filePath = os.path.dirname(os.path.abspath(__file__))
    searchPath = mx.FileSearchPath(os.path.join(filePath, '..', '..'))
    searchPath.append(os.path.dirname(opts.inputFilename))
    libraryFolders = []
    if opts.paths:
        for pathList in opts.paths:
            for path in pathList:
                searchPath.append(path)
    if opts.libraries:
        for libraryList in opts.libraries:
            for library in libraryList:
                libraryFolders.append(library)
    libraryFolders.append("libraries")
    mx.loadLibraries(libraryFolders, searchPath, stdlib)
    doc.importLibrary(stdlib)

    valid, msg = doc.validate()
    if not valid:
        print("Validation warnings for input document:")
        print(msg)

    baseType = mx_render.BaseType.FLOAT if opts.hdr else mx_render.BaseType.UINT8
    baker = mx_render_glsl.TextureBaker.create(opts.width, opts.height, baseType)
    if opts.average:
        baker.setAverageImages(True)
    baker.bakeAllMaterials(doc, searchPath, opts.outputFilename)

if __name__ == '__main__':
    main()
