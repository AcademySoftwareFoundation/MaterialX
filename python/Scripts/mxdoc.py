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
        elif opts.documentType == 'md':
            print('### Node Group: ' + ng)
        else:
            print('/// @defgroup ' + ng + " Group: " + ng)
            print('///@{')

        groupString = ""
        if opts.documentType == "html":
            for n in nodegroupdict[ng]:
                groupString += '<a href="#%s">%s</a> ' % (n, n)
            print('<ul>')
            print('<li>' + groupString)
            print('</ul>')
        elif opts.documentType == 'md':
            for n in nodegroupdict[ng]:
                groupString += '[' + n + '](#' + n + ') '
            print('* ' + groupString)
        else:
            for n in nodegroupdict[ng]:
                print('/// @brief class ' + n + ' in ' + ng)
            print('///@}')

    if opts.documentType == "html":
        print('<hr>')
    elif opts.documentType == 'md':
        print('---------')
    else:
        print(' ')

# Print the document for node definitions in a file
def printNodeDefs(doc, opts):

    currentNodeString = ""

    # Crete grapher. Note that we don't need additional subgraphs groupings as the 
    # implemention is a subgraph.
    graphio = mx.MermaidGraphIo.create()
    graphOptions = mx.GraphIoGenOptions()
    graphOptions.setWriteSubgraphs(False)
    graphOptions.setOrientation(mx.GraphOrientation.LEFT_RIGHT)
    graphio.setGenOptions(graphOptions)

    for nd in doc.getNodeDefs():

        # HTML output
        if opts.documentType == 'html':
            nodeString = nd.getNodeString()
            if currentNodeString != nodeString:
                print('<h3><a id="%s">' % nodeString)
                print('Node: %s' % nodeString)
                print('</a></h3>')
                currentNodeString = nodeString  
            print('<details><summary>%s</summary>' % nd.getName())
            print('<p>')
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
            if opts.nodegraph:
                mdoutput = ''
                ng = nd.getImplementation()
                if ng and ng.isA(mx.NodeGraph):
                    outputList = ng.getOutputs()
                    mdoutput = graphio.write(ng, outputList)
                    print('<li> <em>Nodegraph</em>: %s' % ng.getName())
                    if mdoutput:
                        print('<pre><code class="language-mermaid"><div class="mermaid">')
                        print(mdoutput)
                        print('\n')
                        print('</div></code></pre>\n')   
                    else:
                        print('None')
                else:
                    print('<li> <em>Implementation</em>: Non-graph')

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
            print('</p></details>')

        # Markdown output
        elif opts.documentType == 'md':
            nodeString = nd.getNodeString()
            if currentNodeString != nodeString:
                print('### Node: *%s*' % nodeString)
                currentNodeString = nodeString
            print('<details><summary>%s</summary>' % nd.getName())
            print('<p>')
            print(' ')
            print('* *Nodedef*: %s' % nd.getName())
            print('* *Type*: %s' % nd.getType())
            if len(nd.getNodeGroup()) > 0:
                print('* *Node Group*: %s' % nd.getNodeGroup())
            if len(nd.getVersionString()) > 0:
                print('* *Version*: %s. Is default: %s' % (nd.getVersionString(), nd.getDefaultVersion()))
            if len(nd.getInheritString()) > 0:
                print('- *Inherits From*: %s' % nd.getInheritString())
            docstring = nd.getAttribute('doc')
            if not docstring:
                docstring = "UNDOCUMENTED"
            print('* *Doc*: %s' % docstring)
            if opts.nodegraph:
                mdoutput = ''
                ng = nd.getImplementation()
                if ng and ng.isA(mx.NodeGraph):
                    outputList = ng.getOutputs()
                    mdoutput = graphio.write(ng, outputList)
                    print('* *Nodegraph*: %s' % ng.getName())
                    if mdoutput:
                        print('\n')
                        print('```mermaid')
                        print(mdoutput)
                        print('```')
                    else:
                        print('None')
                else:
                    print('* *Implementation*: Non-graph')
            
            print(' \n')
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

            print('</p></details>')
            print(' ')

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


def printHeader(opts):
    if opts.documentType == "html":
        print('<html>')
        # Add in mermaid support
        if opts.nodegraph:
            print('<script src="https://unpkg.com/mermaid/dist/mermaid.min.js"></script>')
            print('<script>')
            print('mermaid.initialize({ startOnLoad: true, theme: document.body.classList.contains("vscode-dark") || document.body.classList.contains("vscode-high-contrast") ? "dark" : "default" });')
            print('</script>')        
        print('<head><style>')
        print('table, th, td {')
        print('   border-bottom: 1px solid; border-collapse: collapse; padding: 10px;')
        print('}')
        print('</style></head>')
        print('<body>')

def printFooter(opts):
    if opts.documentType == "html":
        print('</body>')
        print('</html>')

def main():
    parser = argparse.ArgumentParser(description="Print documentation for each nodedef in the given document.")
    parser.add_argument(dest="inputFilename", help="Path of the input MaterialX document or folder.")
    parser.add_argument('--docType', dest='documentType', default='md', help='Document type. Default is "md" (Markdown). Specify "html" for HTML output')
    parser.add_argument('--showInherited', default=False, action='store_true', help='Show inherited inputs. Default is False')
    parser.add_argument('--nodegraph', default=False, action='store_true', help='Show nodegraph implementation if any. Default is False')
    parser.add_argument('--printIndex', default=False, action='store_true', help="Print nodedef index. Default is False")

    opts = parser.parse_args()

    printHeader(opts)

    rootPath = opts.inputFilename;
    doc = mx.createDocument()
    readDocuments(rootPath, doc)    

    nodedict = getNodeDictionary(doc)
    printNodeDictionary(nodedict, opts)
            
    printNodeDefs(doc, opts) 

    printFooter(opts) 

if __name__ == '__main__':
    main()
