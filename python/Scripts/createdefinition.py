#!/usr/bin/env python
'''
Create definitions based on the nodegraphs in the specified document. Definitions are 
saved to new documents with either the definition and functional graph in a single file or
in separated into a file for definitions and a file for functional graphs.

It is assumed that all nodegraphs have the same category, nodegroup, version and namespace
as only one set of these input options can be provided. It is also assumed that each nodegraph
has a different output signature. If the same signature if out then no definition will be created
for that nodegraph.

e.g. python createdefinition.py 
        resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx 
        --category mymarble 
        --version "2.0.9" 
        --nodegroup 'texture2d' 
        --separateDocuments 1 
        --comment "This is a new custom marble node."
'''

import sys, argparse, os
import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Create definition and functional graph from compound graphs.")
    parser.add_argument("--category", dest="category", default='', help="Category of the definition. If not specified the nodegraph name will be used")
    parser.add_argument("--nodegroup", dest="nodegroup", default='', help="Node group of the definition. Defaults to an empty string")
    parser.add_argument("--version", dest="version", default='', help="Version string of the definition. Defaults to empty string")
    parser.add_argument("--defaultversion", dest="defaultversion", type=mx.stringToBoolean, default=False, help="Flag to indicate this is the default version. Defaults to false.")
    parser.add_argument("--namespace", dest="namespace", default='', help="Namespace of the definition. Defaults to an empty string.")
    parser.add_argument("--comment", dest="comment", default='', help="XML comment to embed before the definition or nodegraph")
    parser.add_argument("--documentation", dest="documentation", default='', help="Definition documentation")
    parser.add_argument('--separateDocuments', dest='separateDocuments', type=mx.stringToBoolean, default=False, help="Writ the definition and functional graph to separate files.")
    parser.add_argument(dest="inputFilename", help="Filename of the input document.")
    parser.add_argument('--outputPath', dest='outputPath', default="./", help='File path to output documents to. If not specified the files are output to the current directory.')

    opts = parser.parse_args()

    version_major, version_minor, version_patch = mx.getVersionIntegers()
    use_1_38_8 = False
    print(version_major, version_minor, version_patch)
    if version_major >=1 and version_minor >= 38 and version_patch >= 8:
        use_1_38_8 = True

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(-1)

    valid, msg = doc.validate()
    if not valid:
        print("Validation warnings for input document:")
        print(msg)
        sys.exit(-1)

    nodeGraphs = doc.getNodeGraphs()
    parameter_signatures = set()
    for nodeGraph in nodeGraphs:
        # Skip nodegraphs which are already functional graphs
        if nodeGraph.getNodeDef():
            print('Skip functional nodegraph %s' % nodeGraph.getNamePath())
            continue
    
        version = mx.createValidName(opts.version) 
        category = mx.createValidName(opts.category)
        nodegroup = mx.createValidName(opts.nodegroup)
        namespace = mx.createValidName(opts.namespace)
        documentation  = opts.documentation
        defaultversion = opts.defaultversion
        
        # User nodegraph name if no category provided as a a category name
        # must be provided
        if not category:
            category = nodeGraph.getName() 

        # Build identifier for new nodedef and functional graph. Includes:
        #   - version
        #   - output types
        #   - namespace
        #
        identifier = category
        if version:
            identifier = identifier + '_' + version

        outputs = nodeGraph.getOutputs()
        parameter_signature = ''
        for output in outputs:
            outputType = output.getType()
            parameter_signature = parameter_signature + '_' + outputType
        if parameter_signature in parameter_signatures:
            print('Duplicate parameter signature found on on nodegraph %s. Skipping definition createion.' % nodeGraph.getNamePath())
            continue
        else:
            parameter_signatures.add(parameter_signature)
        identifier = identifier + parameter_signature

        # Prefix with "ND_" or "NG_" for definition and functional graph names
        nodedefName = 'ND_' + identifier
        nodegraphName = 'NG_' + identifier

        # Create the definition
        definition = None
        if use_1_38_8:
            definition = doc.addNodeDefFromGraph(nodeGraph, nodedefName,
                            category, opts.version, defaultversion, nodegroup, nodegraphName,
                            documentation, namespace)
        else:
            definition = doc.addNodeDefFromGraph(nodeGraph, nodedefName,
                            category, opts.version, defaultversion, nodegroup, nodegraphName)
        funcgraph = doc.getNodeGraph(nodegraphName)

        if not definition or not funcgraph:
            print('Failed to create definition for nodegraph %s' % nodeGraph.getNamePath())
            continue

        if not use_1_38_8:
            print('Patch 1.38.8 definition creation...')
            definition.setDocString(documentation)
            definition.setNamespace(namespace)

            funcgraph.setNamespace(namespace)
            for graphChild in funcgraph.getChildren():
                graphChild.removeAttribute('xpos')
                graphChild.removeAttribute('ypos')

            filterAttributes = { 'nodegraph', 'nodename', 'channels', 'interfacename', 'xpos', 'ypos' }

            # Transfer input interface from the graph to the nodedef
            for input in funcgraph.getInputs():
                nodeDefInput = definition.addInput(input.getName(), input.getType())
                if nodeDefInput:
                    nodeDefInput.copyContentFrom(input)
                    for filterAttribute in filterAttributes:
                        nodeDefInput.removeAttribute(filterAttribute);
                    nodeDefInput.setSourceUri('')
                    input.setInterfaceName(nodeDefInput.getName())

            # Remove interface from the nodegraph
            for input in funcgraph.getInputs():
                funcgraph.removeInput(input.getName())

            # Copy the output interface from the graph to the nodedef
            for output in nodeGraph.getOutputs():
                nodeDefOutput = definition.getOutput(output.getName())
                if nodeDefOutput:
                    definition.removeOutput(output.getName())
                definition.addOutput(output.getName(), output.getType())
                if nodeDefOutput:
                    nodeDefOutput.copyContentFrom(output)
                    for filterAttribute in filterAttributes:
                        nodeDefOutput.removeAttribute(filterAttribute)
                    nodeDefOutput.setSourceUri('')

        # Generate an XML comment string from the given comment.
        commentString = ''
        if opts.comment:
            commentString = '\n\tNode: <' + category + '> '
            commentString = commentString + '\n\t' + opts.comment + '\n  '

        # Separate out the new definition and functional graph and write
        # to either 1 or 2 files depending on if the "separateDocuments" flag
        # is set.
        #
        defDoc = mx.createDocument()
        if commentString:
            comment = defDoc.addChildOfCategory('comment')
            comment.setDocString(commentString);
        # Note: that addNodeDef() will automatically add an output if a type is specified
        # so leave this empty. The outputs will be copied when the content is copied over.
        newDef = defDoc.addNodeDef(definition.getName(), '', definition.getCategory())
        newDef.copyContentFrom(definition)        

        graphDoc = defDoc
        if opts.separateDocuments:
            graphDoc = mx.createDocument()
            if commentString:
                comment = graphDoc.addChildOfCategory('comment')
                comment.setDocString(commentString);
        newGraph = graphDoc.addNodeGraph(funcgraph.getName())
        newGraph.copyContentFrom(funcgraph);

        # Write to file
        outputFilePath = mx.FilePath(opts.outputPath)
        pathExists = os.path.exists(outputFilePath.asString())
        if not pathExists:
            print('Created folder: ', outputFilePath.asString())
            os.makedirs(outputFilePath.asString())

        fileLocation = identifier + '.mtlx'
        if opts.separateDocuments:
            fileLocation = 'ND_' + fileLocation 
        fileLocation = outputFilePath / mx.FilePath(fileLocation)
        print('Write definition to file:', fileLocation.asString())
        mx.writeToXmlFile(defDoc, fileLocation);

        if opts.separateDocuments:
            fileLocation = 'NG_' + identifier + '.mtlx'
            fileLocation = outputFilePath / mx.FilePath(fileLocation)
            print('Write functional graph to file:', fileLocation.asString())
            mx.writeToXmlFile(graphDoc, fileLocation);

if __name__ == '__main__':
    main()
