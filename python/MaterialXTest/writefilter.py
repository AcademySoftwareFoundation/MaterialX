#!/usr/bin/env python
'''
Test reading in all libraries and writing out the doc with the libraries filtered out.
'''

import sys, os, argparse
import MaterialX as mx

def skipLibraryElement(elem):
    return not elem.hasSourceUri()

def main():
    parser = argparse.ArgumentParser(description='Generate shader code for each material / shader in a document.')
    parser.add_argument(dest='libraryFolder', help='Root of library folder.')
    opts = parser.parse_args()

    # Create main document
    doc = mx.createDocument()

    # Create libraries document
    stdlib = mx.createDocument()

    # Import libraries. This will set the sourceUri on each element
    libraryFolders = []
    searchPath = mx.FileSearchPath()
    libraryFolders.append(opts.libraryFolder)
    libraryFolders.append("libraries")
    mx.loadLibraries(libraryFolders, searchPath, stdlib)
    doc.importLibrary(stdlib)

    # Write out document. Add in element predication which
    # will exclude all elements with a sourceUri.
    # This will exclude all elements in the document resulting
    # in an empty document
    writeOptions = mx.XmlWriteOptions()
    writeOptions.writeXIncludeEnable = False
    writeOptions.elementPredicate = skipLibraryElement
    result = mx.writeToXmlString(doc, writeOptions)

    newdoc = mx.createDocument()
    mx.readFromXmlString(newdoc, result)    
    if newdoc.getNodeDefs():
        print('Failed')
        sys.exit(-1)

    # Test again without the predicate.
    # Nodedefs should exist
    writeOptions.elementPredicate = None
    result = mx.writeToXmlString(doc, writeOptions)
    newdoc = mx.createDocument()
    mx.readFromXmlString(newdoc, result)    
    if not newdoc.getNodeDefs():
        print('Failed')
        sys.exit(-1)

if __name__ == '__main__':
    main()
