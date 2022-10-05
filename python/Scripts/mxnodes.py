#!/usr/bin/env python
'''
Print out all node types grouped by node group for all nodedefs in library definition documents
under a given root folder.
'''

import sys, os, argparse
import MaterialX as mx

def parseArgs():
    parser = argparse.ArgumentParser(description="Print out all node types grouped by node group for all nodedefs in library definition documents under a given root folder.")
    parser.add_argument(dest="inputFolder", help="Root folder for MaterialX definition documents.")
    opts = parser.parse_args()
    return opts

# Find all MaterialX files
def getFiles(opts):
    rootdir = opts.inputFolder
    filelist = []
    for subdir, dirs, files in os.walk(rootdir):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

# Create a dictionary with node group as the primary key for a list of associated
# node types
def getNodeDictionary(inputFilenames):

    doc = mx.createDocument()
    for inputFilename in inputFilenames:
        try:
            mx.readFromXmlFile(doc, inputFilename)
        except mx.ExceptionFileMissing as err:
            print(err)
            sys.exit(0)

    nodegroups = { "" } 
    nodetypes = { "" }
    nodegroupdict = {}
    for nd in doc.getNodeDefs():
        nodestring = nd.getNodeString() 
        nodetypes.add( nodestring )
        nodegroup = nd.getNodeGroup()
        if not nodegroup:
            nodegroup = "no group"
        nodegroups.add( nodegroup )
        if not nodegroup in nodegroupdict.keys():
            nodegroupdict[nodegroup] = { nodestring }
        else:
            nodegroupdict[nodegroup].add(nodestring)

    return nodegroupdict;

# Print out dictionary in Markdown format
def printNodeDictionary(nodegroupdict):
    for ng in nodegroupdict:
        print('### Node Group: ' + ng)
        for n in nodegroupdict[ng]:
            print('* ' + n)

def main():
    opts = parseArgs()
    filelist = getFiles(opts)
    nodedict = getNodeDictionary(filelist)
    printNodeDictionary(nodedict)

if __name__ == '__main__':
    main()