
import MaterialX as mx
import json

class MtlxTraversal:
    @staticmethod
    def printEdge(edge):
        "Sample utility to print out the basic information about an edge"

        upstreamElem = edge.getUpstreamElement()
        downstreamElem = edge.getDownstreamElement()
        connectingElem = edge.getConnectingElement()

        downstreamPath = ''; 
        if connectingElem:
            downstreamPath = connectingElem.getNamePath()
        else:
            downstreamPath  = downstreamElem.getNamePath()

        # Print out information about the edge with an "arrow" to show direction
        # of data flow               
        print('Edge: ' + upstreamElem.getNamePath() + ' --> ' + downstreamPath)

    @staticmethod
    def findEdge(edge, processedEdges):
        "Edge equality comparitor"
        for pe in processedEdges:
            # Note: the comparison (pe == edge) does not work 
            if (pe.getUpstreamElement() == edge.getUpstreamElement() and
                pe.getDownstreamElement() == edge.getDownstreamElement() and
                pe.getConnectingElement() == edge.getConnectingElement()):
                return True
        return False

    @staticmethod
    def updateGraphDictionaryPath(key, value, graphDictionary):
        """
        Add a parent / child to the GraphElement dictionary
        """
        if key in graphDictionary:
            graphDictionary[key].add(value)
        else:
            graphDictionary[key] = {value}

    @staticmethod
    def addStyle(key, value, styleDictionary):
        if key in styleDictionary:
            styleDictionary[key].add(value)
        else:
            styleDictionary[key] = {value}

    @staticmethod
    def updateGraphDictionaryItem(item, graphDictionary):
        """
        Add a Element to the GraphElement dictionary, where the keys are the GraphElement's path, and the value
        is a list of child Element paths
        """
        if not item:
            return

        parentElem = item.getParent()
        if not parentElem or not parentElem.isA(mx.GraphElement):
            return

        key = parentElem.getNamePath()
        value = item.getNamePath()
        MtlxTraversal.updateGraphDictionaryPath(key, value, graphDictionary)

    @staticmethod
    def updateGraphDictionary(edge, graphDictionary):
        """
        Add nodes from either end of the connection to a GraphElement dictionary
        """
        ends = [edge.getUpstreamElement(), edge.getDownstreamElement()]
        for end in ends:
            MtlxTraversal.updateGraphDictionaryItem(end, graphDictionary)

    @staticmethod
    def printGraphDictionary(graphDictionary):
        """
        Print out the sub-graph dictionary
        """
        for graphPath in graphDictionary:
            # Top level document has not path, so just output some identifier string
            if graphPath == '':
                print('Root Document:')
            else:
                print(graphPath + ':')
            for node in graphDictionary[graphPath]:
                print('- ', node)

class MtlxGraphBuilder():
    '''
    Class to extract out the list of nodes and connections from a MaterialX document
    '''
    def __init__(self, doc):
        self.doc = doc
        self.graphDictionary = {}
        self.connections = []
        self.includeGraphs = ''

    def setIncludeGraphs(self, graphs):
        self.includeGraphs = graphs

    def getDictionary(self):
        return self.graphDictionary
    
    def getConnections(self):
        return self.connections

    def updateGraphDictionaryPath(self, key, item, nodetype, type, value, graphDictionary):
        """
        Add a parent / child to the GraphElement dictionary
        """
        if key in graphDictionary:
            #print('add:', key, value, nodetype)
            graphDictionary[key].append([item, nodetype, type, value])
        else:
            #print('add:', key, value, nodetype)
            graphDictionary[key] = [[item, nodetype, type, value]]


    def updateGraphDictionaryItem(self, item, graphDictionary):
        """
        Add a Element to the GraphElement dictionary, where the keys are the GraphElement's path, and the value
        is a list of child Element paths
        """
        if not item:
            return

        parentElem = item.getParent()
        if not parentElem or not parentElem.isA(mx.GraphElement):
            return

        key = parentElem.getNamePath()
        value = item.getNamePath()
        itemType = item.getType()
        itemCategory = item.getCategory()
        itemValue = ''
        if item.isA(mx.Node):
            inputs = item.getInputs()
            if len(inputs) == 1:
                itemValue = inputs[0].getValueString()
                #if itemValue:
                #    print('Scan node input:', inputs[0].getNamePath(), ' = ', itemValue)
        elif item.isA(mx.Input):
            itemValue = item.getValueString()
            #if itemValue:
            #    print('Scan input:', item.getNamePath(), ' = ', itemValue)
        #if itemCategory == 'constant':
        #    itemInput = item.getInput('value')
        #    if itemInput:
        #        itemValue =  itemInput.getValueString()
        self.updateGraphDictionaryPath(key, value, itemCategory, itemType, itemValue, graphDictionary)

        #if item.isA(mx.Input):
        #    valueString = item.getActiveValueString()
        #    print('Got value: ' + valueString + ' for item:' + item.getNamePath())
        #    if valueString:
        #        updateGraphDictionaryPath(key, mx.createValidName(valueString), 'value', graphDictionary)

    def printGraphDictionary(self, graphDictionary):
        """
        Print out the sub-graph dictionary
        """
        for graphPath in graphDictionary:
            if graphPath == '':
                print('Root Document:')
            else:
                print(graphPath + ':')

            filter = 'input'
            # Top level document has not path, so just output some identifier string
            for item in graphDictionary[graphPath]:
                if item[1] != filter:
                    continue
                print('- ', item)
            filter = 'output'
            # Top level document has not path, so just output some identifier string
            for item in graphDictionary[graphPath]:
                if item[1] != filter:
                    continue
                print('- ', item)
            filter = ['output', 'input']
            # Top level document has not path, so just output some identifier string
            for item in graphDictionary[graphPath]:
                if item[1] not in filter:
                    print('- ', item)

    def getParentGraph(self, elem):
        while (elem and not elem.isA(mx.GraphElement)):
            elem = elem.getParent()
        return elem

    def getDefaultOutput(self, node):

        if not node:
            return ''

        defaultOutput = None
        if node.isA(mx.Node):
            nodedef = node.getNodeDef()
            if nodedef:
                defaultOutput = nodedef.getActiveOutputs()[0]
            else:
                print('Cannot find nodedef for node:', node.getNamePath())
        elif node.isA(mx.NodeGraph):
            defaultOutput = node.getOutputs()[0]

        if defaultOutput:
            return defaultOutput.getName()
        return ''    

    def appendPath(self, p1, p2):
        if p2:
            return p1 + '/' + p2
        return p1

    def buildPortConnection(self, doc, portPath, connections, portIsNode):
        '''
        Build a list of connections for the given graphElement.

        Arguments:
        - doc: The document to search for the portPath
        - portPath: The path to the port to search for connections
        - connections: The list of connections to append to. Returned.
        - portIsNode: If True, the portPath is a node, otherwise it is a port 
        '''
        root = doc.getDocument()
        port = root.getDescendant(portPath)
        if not port:
            print('Element not found:', portPath)
            return
        
        if not (port.isA(mx.Input) or port.isA(mx.Output)):
            print('Element is not an input or output')
            return

        parent = port.getParent()
        parentPath = parent.getNamePath()
        parentGraph = self.getParentGraph(port)

        # Need to "jump out" of current graph if considering an input interfae
        # on a graph
        if port.isA(mx.Input) and parent.isA(mx.NodeGraph):
            parentGraph = parentGraph.getParent()

        if not parentGraph:
            print('Cannot find parent graph of port', port)
        parentGraphPath = parentGraph.getNamePath()

        outputName = port.getOutputString()

        destNode = portPath if portIsNode else parentPath
        destPort = '' if portIsNode else port.getName()

        nodename = port.getAttribute('nodename')
        if nodename:
            if len(parentGraphPath) == 0:
                result = [self.appendPath(nodename, ''), outputName, destNode, destPort, 'nodename']
            else:
                #if not doc.getDescendant(parentGraphPath + '/' + nodename):
                #    print('Cannot find nodename:', nodename, 'in graph:', parentGraphPath)
                result = [self.appendPath(parentGraphPath, nodename), outputName, destNode, destPort, 'nodename']
            #if portIsNode:
            #print('append nodename connection:', result)
            connections.append(result)
            return
        
        nodegraph = port.getNodeGraphString()
        if nodegraph:
            if not outputName:
                outputName = self.getDefaultOutput(parentGraph.getChild(nodegraph))
            if len(parentGraphPath) == 0:
                result = [self.appendPath(nodegraph, outputName), '', destNode, destPort, 'nodename']
            else:
                #if not doc.getDescendant(parentGraphPath + '/' + interfaceName):
                #    print('- Dyanmically add in nodegraph:', nodegraph, 'to  graph:', parentGraphPath)
                result = [self.appendPath(parentGraphPath, nodegraph), outputName, destNode, destPort, 'nodegraph']
            #if portIsNode:
            #print('append nodegraph connection:', result)
            connections.append(result)
            return            
        
        interfaceName = port.getInterfaceName()
        if interfaceName:
            if len(parentGraphPath) == 0:
                if not outputName:
                    outputName = self.getDefaultOutput(parentGraph.getChild(interfaceName))
                #print('- Dyanmically add in interfaceName:', interfaceName, 'to NO graph:', parentGraphPath)
                result = [self.appendPath(interfaceName, outputName), '', destNode, destPort, 'nodename']
            else:
                outputName = ''
                # This should be invalid but you can have an input name on a nodedef be the
                # same a node in the functional braph. Emit a warning and rename it.
                itemValue = ''
                if destNode == (parentGraphPath + '/' + interfaceName):
                    dictItem = self.graphDictionary.get(parentGraphPath)
                    if dictItem:
                        found = False
                        for item in dictItem:
                            if item[0] == parentGraphPath + '/' + interfaceName:
                                found = True
                                break
                        if found:
                            print('Warning: Rename duplicate interface:', parentGraphPath + '/' + interfaceName + ':in')
                            interfaceName = interfaceName + ':in'                

                found = False
                dictItem = self.graphDictionary.get(parentGraphPath)
                if dictItem:
                    for item in dictItem:
                        if item[0] == parentGraphPath + '/' + interfaceName:
                            found = True
                            break

                if not found:
                    # TODO: Grab the input value from the nodedef.
                    #print('- Dyanmically add in interfaceName:', interfaceName, 'to  graph:', parentGraphPath, '.Value: ', itemValue)
                    self.updateGraphDictionaryPath(parentGraphPath, parentGraphPath + '/' + interfaceName, 'input', port.getType(), itemValue, self.graphDictionary)
                result = [self.appendPath(parentGraphPath, interfaceName), outputName, destNode, destPort, 'interfacename']
            #if portIsNode:
            #print('append interface connection:', result)
            connections.append(result)
            return

        if outputName:
            if len(parentGraphPath) == 0:
                result = [self.appendPath(outputName, ''), '', parentPath, port.getName(), 'nodename']
            else:
                result = [self.appendPath(parentGraphPath, outputName), '', parentPath, port.getName(), 'output']
            #if portIsNode:
            #print('append connection:', result)
            connections.append(result)
            return

        #if port.isA(mx.Input):
        #    portValue = port.getValueString()
        #    if portValue:
        #        result = [portValue, '', destNode, destPort, 'value']
        #        connections.append(result)

    def buildConnections(self, doc, graphElement, connections):
        
        #print('get children for graph: "%s"' % graphElement.getNamePath())
        root = doc.getDocument()
        for elem in graphElement.getChildren():            
            if not elem.hasSourceUri():
                if elem.isA(mx.Input):
                    self.buildPortConnection(root, elem.getNamePath(), connections, True)
                elif elem.isA(mx.Output):
                    self.buildPortConnection(root, elem.getNamePath(), connections, True)
                elif elem.isA(mx.Node):
                    nodeInputs = elem.getInputs()
                    for nodeInput in nodeInputs:
                        self.buildPortConnection(root, nodeInput.getNamePath(), connections, False)
                elif elem.isA(mx.NodeGraph):
                    nodedef = elem.getNodeDef()
                    if nodedef:
                        connections.append([elem.getNamePath(), '', nodedef.getName(), '', 'nodedef'])
                    visited = set()
                    path = elem.getNamePath()
                    if path not in visited:
                        visited.add(path)
                        self.buildConnections(root, elem, connections)

    def buildGraphDictionary(self, doc):
        '''
        Build a dictionary of the graph elements in the document. The dictionary
        has the graph path as the key, and a list of child elements as the value.

        Arguments:
        - doc: The document to build the graph dictionary from

        Returnes:
        - The graph dictionary    
        '''
        graphDictionary = {}

        root = doc.getDocument()
        skipped = []

        for elem in doc.getChildren():
            if elem.hasSourceUri():
                skipped.append(elem.getNamePath())
            else:
                if elem.isA(mx.Input) or elem.isA(mx.Output) or elem.isA(mx.Node):
                    self.updateGraphDictionaryItem(elem, graphDictionary)
                elif (elem.isA(mx.NodeGraph)):
                    # Temporarily copy over inputs and from nodedef this is a
                    # functional graph
                    if elem.getAttribute('nodedef'):
                        nodeDef = elem.getAttribute('nodedef')
                        nodeDef = root.getDescendant(nodeDef)
                        if nodeDef:
                            nodeDefName = nodeDef.getName()
                            for nodeDefInput in nodeDef.getInputs():                        
                                newInput = elem.addInput(nodeDefInput.getName(), nodeDefInput.getType())
                                newInput.copyContentFrom(nodeDefInput)

                    for node in elem.getInputs():
                        self.updateGraphDictionaryItem(node, graphDictionary)
                    for node in elem.getOutputs():
                        self.updateGraphDictionaryItem(node, graphDictionary)
                    for node in elem.getNodes():
                        self.updateGraphDictionaryItem(node, graphDictionary)
                    for node in elem.getTokens():
                        self.updateGraphDictionaryItem(node, graphDictionary)
                elif elem.isA(mx.NodeDef):
                    self.updateGraphDictionaryItem(elem, graphDictionary)
                elif elem.isA(mx.Token):            
                    self.updateGraphDictionaryItem(elem, graphDictionary)
        
        return graphDictionary
    
    def execute(self):
        '''
        Build the graph dictionary and connections
        '''
        self.connections = []
        self.graphDictionary = {}

        graphElement = self.doc
        if self.includeGraphs:
            graph = self.includeGraphs
            graphElement = self.doc.getDescendant(graph)
            if graphElement:
                #suri = graphElement.getSourceUri()
                graphElement.setSourceUri('')
                print('Scan graph:', graphElement.getNamePath())
                #graphElement.setSourceUri(suri)
            else:
                print('Graph not found:', graph)

        self.graphDictionary = self.buildGraphDictionary(graphElement)
        self.buildConnections(self.doc, graphElement, self.connections)

    def exportToJSON(self, filename, inputFileName):
        data = {}
        data['doc'] = 'Graph connections for: ' + inputFileName
        data['copyright'] = 'Copyright 2025, NanMu Consulting. kwokcb@gmail.com'
        data['graph'] = self.graphDictionary
        data['connections'] = self.connections

        with open(filename, 'w') as outfile:
            # Write json with indentation
            json.dump(data, outfile, indent=2)
            outfile.write('\n')
            outfile.close()

    def importFromJSON(self, filename):
        with open(filename, 'r') as infile:
            data = json.load(infile)
            infile.close()
            self.graphDictionary = data['graph']
            self.connections = data['connections']

class MxMermaidGraphExporter:
    def __init__(self, graphDictionary, connections):
        self.graphDictionary = graphDictionary
        self.connections = connections
        self.mermaid = []
        self.orientation = 'LR'
        self.emitCategory = False
        self.emitType = False
        self.emitValue = True

        # Formatting options: colors and shape

        self.node_colors = dict()
        self.node_colors['input'] = ['#09D', '#FFF']
        self.node_colors['output'] = ['#0C0', '#FFF']
        self.node_colors['surfacematerial'] = ['#090', '#FFF']
        self.node_colors['nodedef'] = ['#00C', '#FFF']
        self.node_colors['token'] = ['#222', '#FFF']
        self.node_colors['constant'] = ['#500', '#FFF']
        self.node_colors['ifequal'] = ['#C72', '#FFF']
        self.node_colors['ifgreatereq'] = ['#C72', '#FFF']
        self.node_colors['switch'] = ['#C72', '#FFF']

        self.FONT_COLOR = '#FFF'
        self.RECT_START = '['
        self.RECT_END = ']'
        self.ROUNDED_RECT_START = '(['
        self.ROUNDED_RECT_END = '])'
        self.SQUARE_RECT_START = '[['
        self.SQUARE_RECT_END = ']]'
        self.DIAMOND_START = '{'
        self.DIAMOND_END = '}'        

        self.node_shapes = dict()
        self.node_shapes['input'] = [self.ROUNDED_RECT_START, self.ROUNDED_RECT_END]
        self.node_shapes['output'] = [self.ROUNDED_RECT_START, self.ROUNDED_RECT_END]
        self.node_shapes['surfacematerial'] = [self.ROUNDED_RECT_START, self.ROUNDED_RECT_END]
        self.node_shapes['nodedef'] = [self.SQUARE_RECT_START, self.SQUARE_RECT_END]
        self.node_shapes['token'] = [self.DIAMOND_START, self.DIAMOND_END]
        self.node_shapes['constant'] = [self.ROUNDED_RECT_START, self.ROUNDED_RECT_END]
        self.node_shapes['ifequal'] = [self.DIAMOND_START, self.DIAMOND_END]
        self.node_shapes['ifgreatereq'] = [self.DIAMOND_START, self.DIAMOND_END]
        self.node_shapes['switch'] = [self.DIAMOND_START, self.DIAMOND_END]

    def setOrientation(self, orientation):
        self.orientation = orientation

    def setEmitCategory(self, emitCategory):
        self.emitCategory = emitCategory

    def setEmitType(self, emitType):
        self.emitType = emitType

    def setEmitValue(self, emitValue):
        self.emitValue = emitValue

    def getNodeColors(self):
        return self.node_colors
    
    def setNodeColors(self, colors):
        self.node_colors = colors

    def getNodeShapes(self):
        return self.node_shapes
    
    def setNodeShapes(self, shapes):
        self.node_shapes = shapes

    def sanitizeString(self, path):
        #return path
        path = path.replace('/default', '/default1')
        path = path.replace('/', '_')
        path = path.replace(' ', '_')
        return path

    def edgeString(self, label):
        if len(label) > 0:
            return '--"%s"-->' % (label)
        else:
            return '-->'

    def execute(self):

        CATEGORY_INDEX = 1
        TYPE_INDEX = 2
        VALUE_INDEX = 3       

        mermaid = []
        mermaid.append('graph %s' % self.orientation)
        for graphPath in self.graphDictionary:
            isSubgraph = graphPath != ''
            if isSubgraph:
                mermaid.append('    subgraph %s' % graphPath)
            
            for item in self.graphDictionary[graphPath]:
                path = item[0]
                # Get "base name" of the path
                label = path.split('/')[-1]
                # Sanitize the path name
                path = self.sanitizeString(path)

                if self.emitCategory:
                    label = item[CATEGORY_INDEX]
                if self.emitType:
                    label += ":" + item[TYPE_INDEX]
                if self.emitValue and item[3]:
                    label += ":" + item[VALUE_INDEX]
                
                # Emit formatted nodes 
                if (item[CATEGORY_INDEX] in self.node_colors):
                    colors = self.node_colors[item[CATEGORY_INDEX]]
                    mermaid.append('    style %s  fill:%s, color:%s' % (path, colors[0], colors[1]))
                
                if (item[CATEGORY_INDEX] in self.node_shapes):
                    shape = self.node_shapes[item[CATEGORY_INDEX]]
                    mermaid.append('    %s%s%s%s' % (path, shape[0], label, shape[1]))
                else:
                    mermaid.append('    %s[%s]' % (path, label))

            if isSubgraph:
                mermaid.append('    end')
        self.mermaid = mermaid
        
        for connection in self.connections:
            source = ''

            # Sanitize path names
            connection[0] = self.sanitizeString(connection[0])
            connection[2] = self.sanitizeString(connection[2])

            # Set source node. If nodes is in a graph then we use <graph>/<node> as source
            source = connection[0]
            
            # Set destination node
            dest = connection[2]

            # Edge can be combo of source output port + destination input port
            if len(connection[1]) > 0:
                if len(connection[3]) > 0:
                    edge = connection[1] + '-->' + connection[3]
                else:
                    edge = connection[1]
            else:
                edge = connection[3]

            sourceNode = mx.createValidName(source)
            if connection[4] == 'value':
                connectString = '    %s["%s"] %s %s' % (sourceNode, source, self.edgeString(edge), dest)
            else:
                connectString = '    %s %s %s' % (sourceNode, self.edgeString(edge), dest)
            mermaid.append(connectString)

        return mermaid

    def write(self, filename):
        with open(filename, 'w') as f:
            for line in self.export():
                f.write('%s\n' % line)

    def getGraph(self, wrap=True):
        result = ''
        if wrap:
            result = '```mermaid\n' + '\n'.join(self.mermaid) + '\n```'
        else:
            result = '\n'.join(self.mermaid)
        # Sanitize
        result = result.replace('/default', '/default1')
        return result

    #def display(self):
    #    display_markdown(self.getGraph(), raw=True)

     # Export mermaid
    def export(self, filename):
        mermaidGraph = self.getGraph()
        with open(filename, 'w') as outFile:
            outFile.write(mermaidGraph)

### Old graph builder.
class MtlxMermaid:

    @staticmethod
    def emitMermaidEdge_nointerfaces(indent, edge):
        """
        Sample utility to print out edge information in Mermaid format
        Returns a string of form: `(upstream node path) --[downstream node input name]--> (downstream node path)`
        which represents a connection from an upstream node to a downstream one via a given input port.
        """
        outVal = ''

        upstreamElem = edge.getUpstreamElement()
        downstreamElem = edge.getDownstreamElement()
        connectingElem = edge.getConnectingElement()

        downstreamPath = ''
        connectionString = ''
        if connectingElem:
            connectionString = ' --".' + connectingElem.getName() + '"--> '
        else:
            connectionString = ' --> '
        downstreamPath  = downstreamElem.getNamePath()

        upstreamPath = upstreamElem.getNamePath()

        # Sanitize names for Mermaid output
        upstreamPathM = mx.createValidName(upstreamPath)
        downstreamPathM = mx.createValidName(downstreamPath)

        # Print out information about the edge with an "arrow" to show direction
        # of data flow  
        outVal = indent + upstreamPathM + '([' + upstreamPath + '])' + connectionString + downstreamPathM + '([' + downstreamPath + '])'
        return outVal

    @staticmethod
    def emitMermaidSubgraphs(subgraphs):
        """
        Emit GraphElement dictionary in Mermaid format
        """
        subGraphOutput = []

        for subgraph in subgraphs:
            if subgraph == '':
                continue
                
            subgraphM = mx.createValidName(subgraph)  
            subGraphOutput.append('subgraph ' + subgraphM + ':')
            for node in subgraphs[subgraph]:
                subGraphOutput.append('   ' + mx.createValidName(node))
            subGraphOutput.append('end')

        return subGraphOutput

    @staticmethod
    def generateMermaidGraph_nointerfaces(roots, orientation):
        """
        Output a Mermaid graph diagram given a set of root nodes
        """ 
        subgraphs = {}
        processedEdges = set()

        # Find all edges, and build up the GraphElement dictionary
        for root in roots:
            for edge in root.traverseGraph():
                if not MtlxTraversal.findEdge(edge,processedEdges):
                    processedEdges.add(edge)
                    MtlxMermaid.updateGraphDictionary(edge, subgraphs)

        # Get string output for each edge in Mermaid format
        edgeOutput = set()
        for edge in processedEdges:
            outVal = MtlxMermaid.emitMermaidEdge_nointerfaces('    ', edge)
            if outVal not in edgeOutput:
                edgeOutput.add(outVal)

        # Print graph header, edges, and sub-graphs
        outputGraph = []
        outputGraph.append('  graph %s;' % orientation)
        for outVal in edgeOutput:
            outputGraph.append(outVal)
        for line in MtlxMermaid.emitMermaidSubgraphs(subgraphs):
            outputGraph.append(line)

        return outputGraph

    @staticmethod
    def emitInterfaceInputs(indent, edge, subgraphs, edgeOutput, styleOutput):
        '''Emit interface inputs:
        - All inputerface inputs are colored "blue"
        - The links from interface inputs are drawn with thicker lines.
        '''
        outVal = ''

        # Look for upstream interface inputs
        upstreamElem = edge.getUpstreamElement()
        for input in upstreamElem.getInputs():
            # getInterfaceInput() will find the interface input if it exists
            interfaceInput = input.getInterfaceInput()
            if interfaceInput:

                # Emit connection from interface input to node input
                interfaceName = interfaceInput.getName()
                interfaceNameM = mx.createValidName(interfaceInput.getNamePath())
                nodeName = mx.createValidName(upstreamElem.getNamePath())
                outVal = indent + interfaceNameM + '([' + interfaceName + ']) ==".' + input.getName() + '"==> ' + nodeName
                if outVal not in edgeOutput:
                    edgeOutput.add(outVal)
                    styleOutput.add(indent + 'style ' + interfaceNameM + ' fill:#0CF, color:#111')

                # Update subgraphs to include this input
                MtlxTraversal.updateGraphDictionaryItem(interfaceInput, subgraphs)

        return outVal

    @staticmethod
    def emitMermaidEdge(indent, edge, subgraphs, edgeOutput, styleOutput):
        "Sample utility to print out edge information in Mermaid format"
        "The interface getConnectedOuput() is used to determine what output the dowstream input is connected to"

        outVal = ''

        # Current set of conditionals.
        # Will work even if the standard library is not loaded.
        conditionals = ['ifequal', 'ifgreater', 'ifgreatereq', 'switch']

        upstreamElem = edge.getUpstreamElement()
        if upstreamElem.getType() == mx.MATERIAL_TYPE_STRING:
            print('Material upstream: ' + upstreamElem.getNamePath())
        downstreamElem = edge.getDownstreamElement()
        connectingElem = edge.getConnectingElement()

        downstreamPath  = downstreamElem.getNamePath()
        upstreamPath = upstreamElem.getNamePath()
        upstreamPathM = mx.createValidName(upstreamPath)

        # Add a connection from the upstream output to the downstream 
        upstreamOutput = None
        if connectingElem:
            outputString = connectingElem.getAttribute("output")
            if outputString:
                leftBrace = '(['
                rightBrace = '])'
                if upstreamElem.getCategory() in conditionals:
                    leftBrace = '{'
                    rightBrace = '}'
                    styleOutput.add(indent + 'style ' + mx.createValidName(upstreamPath) + ' fill:#F80, color:#111')
                    
                upstreamOutput = downstreamElem.getConnectedOutput(connectingElem.getName())
                if upstreamOutput:
                    upstreamOutputName = upstreamOutput.getName()
                    upstreamOutputNameM = mx.createValidName(upstreamOutput.getNamePath())
                    outConnectionString =  upstreamOutputNameM + leftBrace + upstreamOutputName + rightBrace

                    outVal = indent + upstreamPathM + leftBrace + upstreamPath + rightBrace + ' --> ' + outConnectionString
                    if outVal not in edgeOutput:
                        edgeOutput.add(outVal)
                        styleOutput.add(indent + 'style ' + upstreamOutputNameM + ' fill:#0C0, color:#111')

                    MtlxTraversal.updateGraphDictionaryItem(upstreamOutput, subgraphs)

                    # The upstream output is the upstream path instead of the node.
                    upstreamPath = upstreamOutput.getNamePath()
                    upstreamElem = upstreamOutput

                # <output> is not explicitly specified. This occurs for Node outputs
                else:
                    upstreamOutputName = outputString
                    graphElementPath = upstreamElem.getParent().getNamePath()
                    upstreamOutputPath = graphElementPath + '/' + outputString
                    upstreamOutputNameM = mx.createValidName(upstreamOutputPath)
                    outConnectionString =  upstreamOutputNameM + '([' + upstreamOutputName + '])'

                    outVal = indent + upstreamPathM + leftBrace + upstreamPath + rightBrace + ' --> ' + outConnectionString
                    if outVal not in edgeOutput:
                        edgeOutput.add(outVal)
                        styleOutput.add(indent + 'style ' + upstreamOutputNameM + ' fill:#0C0, color:#111')

                    MtlxTraversal.updateGraphDictionaryPath(graphElementPath, upstreamOutputPath, subgraphs)

                    # The upstream output is the upstream path instead of the node.
                    upstreamPath = upstreamOutputPath
                    upstreamElem = upstreamOutput

        inputConnectionString = ''
        if connectingElem:
            inputConnectionString = ' --".' + connectingElem.getName() + '"--> '
        else:
            inputConnectionString = ' --> '

        # Sanitize names for Mermaid output
        upstreamPathM = mx.createValidName(upstreamPath)
        downstreamPathM = mx.createValidName(downstreamPath)

        # Print out information about the edge with an "arrow" to show direction
        # of data flow  
        upConditional = False
        leftBrace = '(['
        rightBrace = '])'
        if upstreamElem and upstreamElem.getCategory() in conditionals:
            upConditional = True
            leftBrace = '{'
            rightBrace = '}'
        outVal = indent + upstreamPathM
        outVal = outVal + leftBrace + upstreamPath + rightBrace
        
        outVal = outVal + inputConnectionString + downstreamPathM 

        downConditional = False
        leftBrace = '(['
        rightBrace = '])'
        if downstreamElem and downstreamElem.getCategory() in conditionals:
            downConditional = True
            leftBrace = '{'
            rightBrace = '}'
        outVal = outVal + leftBrace + downstreamPath + rightBrace

        if outVal not in edgeOutput:
            edgeOutput.add(outVal)
            if downstreamElem.getType() == mx.MATERIAL_TYPE_STRING:
                styleOutput.add(indent + 'style ' + downstreamPathM + ' fill:#0C0, color:#111')
            else:
                if downConditional:
                    styleOutput.add(indent + 'style ' + downstreamPathM + ' fill:#F80, color:#111')
                if upConditional:
                    styleOutput.add(indent + 'style ' + upstreamPathM + ' fill:#F80, color:#111')

    @staticmethod
    def generateMermaidGraph(roots, orientation):
        """
        Output a Mermaid graph diagram given a set of root nodes
        """ 
        subgraphs = {}
        processedEdges = set()

        # Find all edges, and build up the GraphElement dictionary
        for root in roots:
            for edge in root.traverseGraph():
                if not MtlxTraversal.findEdge(edge,processedEdges):
                    processedEdges.add(edge)
                    MtlxTraversal.updateGraphDictionary(edge, subgraphs)

        # Get string output for each edge in Mermaid format
        edgeOutput = set()
        styleOutput = set()
        for edge in processedEdges:
            outVal = MtlxMermaid.emitMermaidEdge('    ', edge, subgraphs, edgeOutput, styleOutput)
            if outVal not in edgeOutput:
                edgeOutput.add(outVal)

        # Include interface input edges
        for edge in processedEdges:
            MtlxMermaid.emitInterfaceInputs('    ', edge, subgraphs, edgeOutput, styleOutput)            

        # Print graph header, edges, sub-graphs, and styling
        outputGraph = []
        outputGraph.append('  graph %s;' % orientation)
        for outVal in edgeOutput:
            outputGraph.append(outVal)

        outputGraph.append('%% Subgraphs')
        for line in MtlxMermaid.emitMermaidSubgraphs(subgraphs):
            outputGraph.append(line)

        outputGraph.append('%% Style')
        for line in styleOutput:
            outputGraph.append(line)

        return outputGraph

