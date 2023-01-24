#!/usr/bin/env python
'''
Create a document per node definition such that an instance of that 
definition is routed to a "material" root. Makes use of "convert"
nodes for various output types. A material root is created for each
output on a nodedef, if multiple exist.
'''

import sys, os, argparse
import MaterialX as mx


# Given a node, add downstream material attachments
def addMaterialGraphs(node, doc, outdoc, nodedef):

    # Use more cumbersome method of grabbing from nodedef if not using
    # explicit outputs on node
    outputs = []
    outputs = nodedef.getActiveOutputs()
    isMultiOutput = len(outputs) > 1

    addedMaterial = False
    for output in outputs:
        outputName = output.getName()
        outputType = output.getType()

        shaderNodeName = outdoc.createValidChildName('shader_' + node.getName() + '_' + outputName)                
        materialNodeName = outdoc.createValidChildName('material_' + node.getName() + '_' + outputName)                

        # EDF and BSDF can feed into surface shader then a material. 
        # These do not have nodedefs as they current fail in code generation.
        if outputType in { 'EDF', 'BSDF' }:
            shaderNode = outdoc.addNode("surface", shaderNodeName, "surfaceshader")
            newInput = shaderNode.addInput(outputType.lower(), outputType)
            newInput.setNodeName(node.getName())
            if isMultiOutput:
                newInput.setAttribute('output', outputName)
            materialNode = outdoc.addMaterialNode(materialNodeName, shaderNode)
            if materialNode:
                addedMaterial = True

        # Shader can feed directly into a material
        elif outputType in { 'surfaceshader', 'volumeshader', 'displacementshader' }: 
            materialNode = outdoc.addMaterialNode(materialNodeName, node)
            if materialNode:
                addedMaterial = True

        # Other numeric and boolean can feed into utility "convert" nodes 
        elif outputType in { 'float', 'vector2', 'vector3', 'vector4', 'integer', 'boolean', 'color3', 'color4' }:

            convertDefinition = 'ND_convert_' + outputType + '_shader'
            convertNode = doc.getNodeDef(convertDefinition)
            if not convertNode:
                print("> Failed to find conversion definition: %s" % convertDefinition)
            else:
                shaderNode = outdoc.addNodeInstance(convertNode, shaderNodeName)
                shaderNode.removeAttribute('nodedef')
                newInput = shaderNode.addInput('in', outputType)
                newInput.setNodeName(node.getName())                        
                if isMultiOutput:
                    newInput.setAttribute('output', outputName)    
                materialNode = outdoc.addMaterialNode(materialNodeName, shaderNode)
                if materialNode:
                    addedMaterial = True

    return addedMaterial

# Create a node instance given a node definition with appropriate inputs and outputs
def createNodeInstance(nodedef, nodeName, outdoc, setEmptyValues):
    node = outdoc.addNodeInstance(nodedef, nodeName)
    node.removeAttribute('nodedef')
    version = nodedef.getVersionString()
    if len(version) > 0:
        node.setVersionString(version)

    nodeType = nodedef.getNodeString()
    isGeomProp = nodeType == 'geompropvalue' 
    isUsdPrimvarReader = nodeType == 'UsdPrimvarReader'

    for input in nodedef.getActiveInputs():
        inputName = input.getName()
        inputType = input.getType()
        valueElem = node.getInput(inputName)
        if (not valueElem):
            newElem = node.addInput(inputName, inputType)
            newElem.copyContentFrom(input)
            if not newElem.getValue():

                # Set input values here as default definition does not define these
                # values. This avoids error in node and code generation validation.
                # Note: For geometry routing, this is using stream / primvar names
                # generated for render tests internally (addAdditionalTestStreams())
                if setEmptyValues:
                    if isGeomProp and inputName == 'geomprop':
                        newElem.setValue('geompropvalue_' + nodedef.getType())
                    elif isUsdPrimvarReader and inputName == 'varname':
                        newElem.setValue('geompropvalue_' + nodedef.getType())
                    elif inputType == 'BSDF':
                        # Create an arbitrary input node 
                        bsdfNodeName = outdoc.createValidChildName('oren_nayar_diffuse_bsdf')
                        bsdfNode = outdoc.addNode('oren_nayar_diffuse_bsdf', bsdfNodeName, inputType)
                        newElem.setNodeName(bsdfNode.getName())
                    elif inputType == "EDF":
                        # Create an input uniform node
                        edfNodeName = outdoc.createValidChildName('uniform_edf')
                        edfNode = outdoc.addNode('uniform_edf', edfNodeName, inputType)
                        newElem.setNodeName(edfNode.getName())
                    elif inputType == 'surfaceshader':
                        ssNodeName = outdoc.createValidChildName('standard_surface')
                        ssNode = outdoc.addNode('standard_surface', ssNodeName, inputType)
                        newElem.setNodeName(ssNode.getName())
                    # There are no "existing" core definitions for either of these so nothing
                    # added here for now.    
                    #elif inputType == 'displacementshader':
                    #elif inputType == 'volumeshader':

            for attr in [ 'doc', 'uimin', 'uimax', 'uifolder', 'uisoftmin', 'uisoftmax', 'uiadvanced' ]:
                newElem.removeAttribute(attr)
    
    return node

# Given a node definition create a material node graph in a new document
def createMaterialFromNodedef(nodedef, doc, outdoc):

    nodeName = nodedef.getName()
    functionName = nodeName.removeprefix('ND_')
    functionName = outdoc.createValidChildName(functionName)

    node = createNodeInstance(nodedef, functionName, outdoc, True)
    if node:
        addedMaterial = addMaterialGraphs(node, doc, outdoc, nodedef)
        if not addedMaterial:
            node = None

    return node

# Print the document for node definitions in a file
def createMaterials(doc, opts):

    # thin_film_bsdf code generation produces undefined variable names for OSL and GLSL
    ignoreNodeList = [ "thin_film_bsdf", "surfacematerial", "volumematerial", "arrayappend", "dot_filename" ]
    ignoreTypeList = [ "lightshader" ]

    nodedefs = doc.getNodeDefs()
    nodedefCount = str(len(nodedefs))
    if nodedefCount == 0:
        print('No definitions to create materials for')

    count = 0
    ignoreList = []
    for nodedef in nodedefs:

        nodeinterface = nodedef.getImplementation(opts.target)
        if not nodeinterface: 
            continue

        sourceUri = nodedef.getSourceUri()

        skip = False
        for i in ignoreNodeList:
            if nodedef.getNodeString() == i:
                skip = True
                continue
        for i in ignoreTypeList:
            if nodedef.getType() == i:
                skip = True
                continue

        if skip:
            ignoreList.append( nodedef.getName() )
            continue

        outdoc = mx.createDocument()

        node = createMaterialFromNodedef(nodedef, doc, outdoc)

        if node:
            if opts.outputPath:
                filename = opts.outputPath + '/' + node.getName() + ".mtlx"
            else:
                filename = node.getName() + ".mtlx"
            print("Write defintion file: %s" % filename)
            mx.writeToXmlFile(outdoc, filename)

        count = count + 1

    print('Create materials for %d definitions' % count)
    print('Skipped nodedefs: ', ignoreList)


def main():
    parser = argparse.ArgumentParser(description="Create Materialx documents for instances of a nodedef. Each node instance's output is sampled by material node.")
    parser.add_argument(dest="libraryPath", help="Path for MaterialX libraries.")
    parser.add_argument('--outputPath', dest='outputPath', help='File path to output material files to.')
    parser.add_argument('--target', dest='target', default='genglsl', help='Shading language target. Default is genglsl')

    opts = parser.parse_args()

    # Read library
    rootPath = opts.libraryPath;
    stdlib = mx.createDocument()
    searchPath = rootPath
    mx.loadLibraries([ rootPath ], searchPath, stdlib)
    doc = mx.createDocument()
    doc.importLibrary(stdlib)

    # Create output directory
    if opts.outputPath:
        if not os.path.exists(opts.outputPath):
            os.makedirs(opts.outputPath)

    # Create material files
    createMaterials(doc, opts) 

if __name__ == '__main__':
    main()
