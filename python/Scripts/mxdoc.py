#!/usr/bin/env python
'''
Print markdown documentation for each nodedef in the given document or documents in a folder
'''

import sys, os, argparse
import MaterialX as mx

HEADERS = ('Name', 'Type', 'Default Value',
           'UI name', 'UI min', 'UI max', 'UI Soft Min', 'UI Soft Max', 'UI step', 'UI group', 'UI Advanced', 'Doc', 'Uniform')

ATTR_NAMES = ('uiname', 'uimin', 'uimax', 'uisoftmin', 'uisoftmax', 'uistep', 'uifolder', 'uiadvanced', 'doc', 'uniform' )

# Find all MaterialX files
def getFiles(rootPath):
    filelist = []
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

# Create a dictionary with node group as the primary key for a list of associated
# node types
def getNodeDictionary(doc):
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

    return nodegroupdict

# Print out dictionary in Markdown format
def printNodeDictionary(nodegroupdict, opts):
    for ng in nodegroupdict:
        if opts.documentType == "html":
            print('<h3>Node Group: ' + ng + '</h3>')
        else:
            print('### Node Group: ' + ng)
        groupString = ""
        if opts.documentType == "html":
            for n in nodegroupdict[ng]:
                groupString += '<a href="#%s">%s</a> ' % (n, n)
            print('<ul>')
            print('<li>' + groupString)
            print('</ul>')
        else:  
            for n in nodegroupdict[ng]:
                groupString += n + ' '
            print('* ' + groupString)
    if opts.documentType == "html":
        print('<hr>')
    else:
        print('---------')

# Print the document for node definitions in a file
def printNodeDefs(doc, opts):

    if opts.documentType == "html":
        print('<head><style>')
        print('table, th, td {')
        print('   border-bottom: 1px solid; border-collapse: collapse; padding: 10px;')
        print('}')
        print('</style></head>')

    currentNodeString = ""
    for nd in doc.getNodeDefs():
        # HTML output
        if opts.documentType == "html":
            nodeString = nd.getNodeString()
            if currentNodeString != nodeString:
                print('<h3><a id="%s">' % nodeString)
                print('Node: %s' % nodeString)
                print('</a></h3>')
                currentNodeString = nodeString               
            print('<ul>')
            print('<li> <em>NodeDef</em>: %s' % nd.getName())
            print('<li> <em>Type</em>: %s' % nd.getType())
            if len(nd.getNodeGroup()) > 0:
                print('<li> <em>Node Group</em>: %s' % nd.getNodeGroup())
            if len(nd.getVersionString()) > 0:
                print('<li> <em>Version</em>: %s. Is default: %s' % (nd.getVersionString(), nd.getDefaultVersion()))
            if len(nd.getInheritString()) > 0:
                print('<li> <em>Inherits From</em>: %s' % nd.getInheritString())
            print('<li> <em>Doc</em>: %s\n' % nd.getAttribute('doc'))
            print('</ul>')
            print('<table><tr>')
            for h in HEADERS:
                print('<th>' + h + '</th>')
            print('</tr>')
            inputList = nd.getActiveInputs() if opts.showInherited  else nd.getInputs()            
            tokenList = nd.getActiveTokens() if opts.showInherited  else nd.getTokens()
            outputList = nd.getActiveOutputs() if opts.showInherited  else nd.getOutputs()
            totalList = inputList + tokenList + outputList;
            for port in totalList:
                print('<tr>')
                infos = []
                if port in outputList:
                    infos.append('<em>'+ port.getName() + '</em>')
                elif port in tokenList:
                    infos.append(port.getName())
                else:
                    infos.append('<b>'+ port.getName() + '</b>')
                infos.append(port.getType())
                val = port.getValue()
                if val and port.getType() == "float":
                    val = round(val, 6)
                infos.append(str(val))
                for attrname in ATTR_NAMES:
                    infos.append(port.getAttribute(attrname))
                for info in infos:
                    print('<td>' + info + '</td>')
                print('</tr>')
            print('</table>')

        # Markdown output
        else:
            print('- *Nodedef*: %s' % nd.getName())
            print('- *Type*: %s' % nd.getType())
            if len(nd.getNodeGroup()) > 0:
                print('- *Node Group*: %s' % nd.getNodeGroup())
            if len(nd.getVersionString()) > 0:
                print('- *Version*: %s. Is default: %s' % (nd.getVersionString(), nd.getDefaultVersion()))
            if len(nd.getInheritString()) > 0:
                print('- *Inherits From*: %s' % nd.getInheritString())
            print('- *Doc*: %s\n' % nd.getAttribute('doc'))
            print('| ' + ' | '.join(HEADERS) + ' |')
            print('|' + ' ---- |' * len(HEADERS) + '')
            inputList = nd.getActiveInputs() if opts.showInherited  else nd.getInputs()
            tokenList = nd.getActiveTokens() if opts.showInherited  else nd.getTokens()
            outputList = nd.getActiveOutputs() if opts.showInherited  else nd.getOutputs()
            totalList = inputList + tokenList + outputList;
            for port in totalList:
                infos = []
                if port in outputList:
                    infos.append('*'+ port.getName() + '*')
                elif port in tokenList:
                    infos.append(port.getName())
                else:
                    infos.append('**'+ port.getName() + '**')
                infos.append(port.getType())
                val = port.getValue()
                if val and port.getType() == "float":
                    val = round(val, 6)
                infos.append(str(val))
                for attrname in ATTR_NAMES:
                    infos.append(port.getAttribute(attrname))
                print('| ' + " | ".join(infos) + ' |')

# Read in a single document or documents in a folder
# Return false if any document cannot be read
def readDocuments(rootPath, doc):

    readDoc = True

    if os.path.isdir(rootPath): 
        filelist = getFiles(rootPath)
        for inputFilename in filelist:
            try:
                mx.readFromXmlFile(doc, inputFilename)
            except mx.ExceptionFileMissing as err:
                print(err)
    else:
        try:
            mx.readFromXmlFile(doc, rootPath)
        except mx.ExceptionFileMissing as err:
            print(err)
            readDoc = False

    return readDoc


def main():
    parser = argparse.ArgumentParser(description="Print documentation for each nodedef in the given document.")
    parser.add_argument(dest="inputFilename", help="Path of the input MaterialX document or folder.")
    parser.add_argument('--docType', dest='documentType', default='md', help='Document type. Default is "md" (Markdown). Specify "html" for HTML output')
    parser.add_argument('--showInherited', default=False, action='store_true', help='Show inherited inputs. Default is False')
    parser.add_argument('--printIndex', default=False, action='store_true', help="Print nodedef index. Default is False")
    opts = parser.parse_args()

    rootPath = opts.inputFilename;
    doc = mx.createDocument()
    readDocuments(rootPath, doc)    

    #if opts.printIndex:
    if opts.printIndex:
        nodedict = getNodeDictionary(doc)
        printNodeDictionary(nodedict, opts)

    printNodeDefs(doc, opts)    

if __name__ == '__main__':
    main()
