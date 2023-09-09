#!/usr/bin/env python
'''
MaterialX glTF bi-directional conversion utility.

If a MaterialX document is provided as input, the utility will convert to glTF,
otherwise if a glTF file is provided as input, the utility will convert to MaterialX.

Copyright 2022 - 2023 Bernard Kwok

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

import sys, os, io, argparse
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_gltf

def skipLibraryElement(elem):
    return not elem.hasSourceUri()

def main():
    parser = argparse.ArgumentParser(description="MaterialX glTF converter")
    parser.add_argument(dest="inputFilename", help="Filename of the input document (glTF or MaterialX).")
    opts = parser.parse_args()

    inputFilename = opts.inputFilename
    if not os.path.isfile(inputFilename):
        print('Input file not found: ' + inputFilename)
        return

    inputFilePath = mx.FilePath(inputFilename)
    
    toGLTF = inputFilePath.getExtension() == 'mtlx'
    toMaterialX = inputFilePath.getExtension() == 'gltf'

    if not toGLTF and not toMaterialX:
        print('Input file must be glTF or MaterialX')
        return

    handler = mx_gltf.GltfMaterialHandler()
    if not handler:
        print('Failed to create GltfMaterialHandler')
        return
    
    log = []
        
    # Convert from Materialx to GLTF
    if toGLTF:
        doc = mx.createDocument()
        mx.readFromXmlFile(doc, inputFilePath)
        outputFilePath = inputFilePath
        print('Converting MaterialX file:%s to glTF file: %s' % 
              (inputFilePath.asString(), outputFilePath.asString() + '.gltf'))
        handler.setMaterials(doc);
        converted = handler.save(outputFilePath.asString() + '.gltf', log);    
        print("- Converted: " + str(converted))
        if not converted:
            print("- Log: " + '\n'.join(log))

    # Convert from GLTF to MaterialX
    elif toMaterialX:
        stdlib = mx.createDocument()
        searchPath = mx.getDefaultDataSearchPath()
        libraryFolders = []
        libraryFolders.extend(mx.getDefaultDataLibraryFolders())
        try:
            mx.loadLibraries(libraryFolders, searchPath, stdlib)
        except mx.Exception as err:
            print('Failed to load standard libraries: "', err, '"')
            sys.exit(-1)

        outputFilePath = mx.FilePath()
        outputFilePath = inputFilePath
        print('Converting glTF file:%s to MaterialX file: %s' % 
              (inputFilePath.asString(), inputFilePath.asString()+".mtlx"))
        
        createAssignments = False
        fullDefinition = False
        handler.setDefinitions(stdlib)
        handler.setGenerateAssignments(createAssignments)
        handler.setGenerateFullDefinitions(fullDefinition)
    
        loadedMaterial = handler.load(inputFilePath, log)
        print("- Loaded GLTF file: " + str(loadedMaterial))
        doc = None
        if loadedMaterial:
            doc = handler.getMaterials() 
        if doc:
            # Filter out standard library elements
            writeOptions = mx.XmlWriteOptions()
            writeOptions.writeXIncludeEnable = False
            writeOptions.elementPredicate = skipLibraryElement
            mx.writeToXmlFile(doc, inputFilePath.asString()+".mtlx", writeOptions)

        print("- Converted: " + str(doc != None))

    if log:
        print("Log" + '\n'.join(log))


if __name__ == '__main__':
    main()