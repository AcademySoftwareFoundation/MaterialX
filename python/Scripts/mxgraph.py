#!/usr/bin/env python
'''
Utility to generate diagrams for node graphs in a MaterialX document. By default
all renderable elements are scanned within the document to produce a single output file.
The current output options are 'Mermard' and GraphViz 'dot' format. The default format is Mermaid
which is appropriate for Markdown consumption.
'''

import sys, os, argparse, shutil, io
import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
import MaterialX.PyMaterialXGenGlsl as mx_gen_glsl
import MaterialX.PyMaterialXGenOsl as mx_gen_osl
import MaterialX.PyMaterialXGenMdl as mx_gen_mdl

def getFiles(rootPath):
    "Find all MaterialX files"
    filelist = []
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

def readLibraries(rootPath, doc):
    "Setup and read in libraries"

    readDoc = True
    readOptions = mx.XmlReadOptions()

    if os.path.isdir(rootPath): 
        filelist = getFiles(rootPath)
        for inputFilename in filelist:
            try:  
                libDoc = mx.createDocument()              
                mx.readFromXmlFile(libDoc, inputFilename, mx.FileSearchPath(), readOptions)
                doc.importLibrary(libDoc)
            except mx.ExceptionFileMissing as err:
                print(err)
    else:
        try:
            libDoc = mx.createDocument()              
            mx.readFromXmlFile(libDoc, rootPath, mx.FileSearchPath(), readOptions)
            doc.importLibrary(libDoc)
        except mx.ExceptionFileMissing as err:
            print(err)
            readDoc = False

    return readDoc

def generateGraph(opts, root, outputList, writeHeader):
    "Generate the graph for a set of outputs within a <nodegraph>"

    graphOutput = ''

    # By default we don't want the per graph headers and will add them
    # in as a wrapper outside this function.
    graphOptions = mx.GraphIoGenOptions()
    graphOptions.setWriteGraphHeader(writeHeader)
    graphOptions.setWriteCategories(False)
    graphOptions.setWriteSubgraphs(True)
    graphOptions.setOrientation(mx.GraphOrientation.LEFT_RIGHT)

    if opts.graphFormat == 'dot':
        # Write dot
        graphio = mx.DotGraphIo.create()
    else:
        # Write mermaid
        graphio = mx.MermaidGraphIo.create()

    graphio.setGenOptions(graphOptions)
    graphOutput = graphio.write(root, outputList)

    return graphOutput

def main():
    parser = argparse.ArgumentParser(description="Print documentation for each nodedef in the given document.")
    parser.add_argument(dest="inputFilename", help="Path of the input MaterialX document or folder.")
    parser.add_argument("--libraryPath", default="libraries", help="Path for MaterialX libraries.")
    parser.add_argument('--graphFormat', dest='graphFormat', default='mermaid', help='Graph format. Default is "mermaid" (Mermaid). Specify "dot" for GraphViz dot output')
    parser.add_argument('--outputFile', default="graph_output.md", help="Name of output file. Default name is 'graph.md")

    opts = parser.parse_args()

    rootPath = opts.inputFilename;
    doc = mx.createDocument()
    readLibraries(opts.libraryPath, doc)

    mx.readFromXmlFile(doc, opts.inputFilename, mx.FileSearchPath())

    # Try to find renderable elements in the document
    nodes = mx_gen_shader.findRenderableElements(doc, False)
    if not nodes:
        nodes = doc.getMaterialNodes()
        if not nodes:
            nodes = doc.getNodesOfType(mx.SURFACE_SHADER_TYPE_STRING)


    # Set up header and foot for the graph
    header = "```mermaid\n" 
    footer = "```"
    if opts.graphFormat == 'dot':
        header = "digraph {\n"
        footer = "}"

    graphOutput = header

    # Create the graph for each element found.
    writeHeader = True
    for elem in nodes:
        graphNode = elem.getDocument()
        roots = []

        # Check outputs
        if elem.isA(mx.Output):

            roots.append(elem.getNamePath())
            parent = elem.getParent()
            if parent.isA(mx.NodeGraph):
                graphNode = parent
            else:
                graphNode = elem.getDocument()

        # Check nodes
        elif elem.isA(mx.InterfaceElement):
            for out in elem.getActiveOutputs():
                roots.append(out.getNamePath())
            if not roots:
                roots.append(elem.getNamePath())
        
        # Generate graph for the set of outputs found
        if graphNode:
            for root in roots:
                print('Generate graph for root: ', root)
            graphOutput += generateGraph(opts, graphNode, roots, writeHeader)
            # Only write header once
            writeHeader = False
        

    graphOutput += footer
    
    # Write output to disk
    filename = opts.outputFile
    f = open(filename, 'w')
    f.write(graphOutput)
    f.close()
    print('Wrote file: ' + filename) 

if __name__ == '__main__':
    main()
