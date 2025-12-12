'''
    Set of utilities to perform MaterialX graph editing

    - Find definition to create
    - Add node instance based on category/type, or based on definition name    
    - Add nodegraph instance
    - Add nodegraph output
    - Connect an output to another output. For connections to nodegraph interface outputs
    - Connect an output to an input. Supports all permutations of connecting a node / nodegraph to another node / nodegraph. 
    - Add interface input to a nodegraph
    - Connect an node input to a interface input on a nodegraph ("publish")
    - Remove connection between a node input to a interface input on a nodegraph ("unpublish")

    Methods grouped under a MtxlNodeGraph class

    Requires: MaterialX package
'''
import MaterialX as mx

class MtlxNodeGraph:
    '''
    MaterialX <nodegraph> utilities
    '''

    @staticmethod
    def getNodeDefinition(doc, category, desiredType):
        '''
        Find a node definition given a category and a type
        '''
        nodedefs = doc.getMatchingNodeDefs(category)
        foundNodeDef = None
        for nodedef in nodedefs:
            if nodedef.getType() == desiredType:
                foundNodeDef = nodedef
                break

        return foundNodeDef

    @staticmethod
    def addNode(parent, category, desiredType, name):
        '''
        Add a named node under a given parent given a category and a type
        '''
        newNode = None
        doc = parent.getDocument() 
        nodedef = MtlxNodeGraph.getNodeDefinition(doc, category, desiredType)
        if nodedef:
            childName = parent.createValidChildName(name)
            newNode = parent.addNodeInstance(nodedef, childName)

        return newNode
    
    @staticmethod
    def addNode(parent, definitionName, name):
        '''
        Utility to create a node under a given parent using a definition name and desired instance name
        '''
        newNode = None
        doc = parent.getDocument()
        nodedef = doc.getNodeDef(definitionName)
        if nodedef:
            childName = parent.createValidChildName(name)
            newNode = parent.addNodeInstance(nodedef, childName)
        return newNode 

    @staticmethod
    def addNodeGraph(parent, name):
        '''
        Add named nodegraph under parent
        '''
        childName = parent.createValidChildName(name)
        nodegraph = parent.addChildOfCategory('nodegraph', childName)
        return nodegraph

    @staticmethod
    def addNodeGraphOutput(parent, type, name='out'):
        '''
        Create an output with a unique name and proper type
        '''
        if not parent.isA(mx.NodeGraph):
            return None
        
        newOutput = None
        childName = parent.createValidChildName(name)
        newOutput = parent.addOutput(childName, type)
        return newOutput    

    @staticmethod
    def connectOutputToOutput(outputPort, upstream, upstreamOutputName):
        '''
        Utility to connect a downstream output to an upstream node / node output
        If the types differ then no connection is made
        '''
        upstreamType = upstream.getType()

        # Check for an explicit upstream output on the upstream node
        # or upstream node's definition
        if upstreamOutputName:
            upStreamPort = upstream.getActiveOutput(upstreamOutputName)
            if not upStreamPort:
                upstreamNodeDef = upstream.getNodeDef()
                if upstreamNodeDef:
                    upStreamPort = upstreamNodeDef.getActiveOutput(upstreamOutputName)
                else:
                    return False
            if upStreamPort:
                upstreamType = upStreamPort.getType()
            
        outputPortType  = outputPort.getType()    
        if upstreamType != outputPortType:
            return False
        
        upstreamName = upstream.getName()
        attributeName = 'nodename'
        if upstream.isA(mx.NodeGraph):
            attributeName = 'nodegraph'
        outputPort.setAttribute(attributeName, upstreamName)
        
        # If an explicit output is specified on the upstream node/graph then
        # set it.
        if upstreamOutputName and upstream.getType() == 'multioutput':
            outputPort.setOutputString(upstreamOutputName)    
        
        return True

    @staticmethod
    def connectNodeToNode(inputNode, inputName, outputNode, outputName):
        '''
        Connect an input on one node to an output on another node. Existence and type checking are performed.
        Returns input port with connection set if succesful. Otherwise None is returned.
        '''
        if not inputNode or not outputNode:
            return None

        # Add an input to the downstream node if it does not exist
        inputPort = inputNode.addInputFromNodeDef(inputName)

        # Check for the type.
        outputType = outputNode.getType()  
        
        # If there is more than one output then we need to find the output type 
        # from the output with the name we are interested in.
        outputPortFound = None
        outputPorts = outputNode.getOutputs()
        if outputPorts:
            # Look for an output with a given name, or the first if not found                    
            if not outputName:
                outputPortFound = outputPorts[0]
            else:
                outputPortFound = outputNode.getOutput(outputName)

        # If the output port is not found on the node instance then
        # look for it the corresponding definition
        if not outputPortFound:
            outputNodedef = outputNode.getNodeDef()
            if outputNodedef:
                outputPorts = outputNodedef.getOutputs()
                
                if outputPorts:
                    # Look for an output with a given name, or the first if not found                    
                    if not outputName:
                        outputPortFound = outputPorts[0]
                    else:
                        outputPortFound = outputNodedef.getOutput(outputName)

        if outputPortFound:
            outputType = outputPortFound.getType()
        elif len(outputName) > 0:
            print('No output port found matching: ', outputName)        

        if inputPort.getType() != outputType:
            print('Input type (%s) and output type (%s) do not match: ' % (inputPort.getType(), outputType))
            return None

        if inputPort:
            # Remove any value, and set a "connection" but setting the node name
            inputPort.removeAttribute('value')
            attributeName = 'nodename' if outputNode.isA(mx.Node) else 'nodegraph'
            inputPort.setAttribute(attributeName, outputNode.getName())
            if outputNode.getType() == 'multioutput' and outputName:
                inputPort.setOutputString(outputName)
        return inputPort
    
    @staticmethod
    def addInputInterface(name, typeString, parent):
        '''
        Add a type input interface. Will always create a new interface
        '''
        validName = parent.createValidChildName(name)
        typedefs = parent.getDocument().getTypeDefs()
        validType = False
        for t in typedefs:
            if typeString in t.getName():
                validType = True
                break

        if validType:
            parent.addInput(validName, typeString)
    
    @staticmethod
    def connectInterface(nodegraph, interfaceName, internalInput):
        '''
        Add an interface input to a nodegraph if it does not already exist. 
        Connect the interface to the internal input. Returns interface input
        '''
        if not nodegraph or not interfaceName or not internalInput:
            return None

        interfaceInput = nodegraph.getInput(interfaceName)

        # Create a new interface with the desired type
        if not interfaceInput:
            interfaceName = nodegraph.createValidChildName(interfaceName)    
            interfaceInput = nodegraph.addInput(interfaceName, internalInput.getType())

        # Copy attributes from internal input to interface. 
        # Remove undesired attributes  as this is not desired to be copied
        interfaceInput.copyContentFrom(internalInput)
        interfaceInput.removeAttribute('sourceUri')
        interfaceInput.removeAttribute('interfacename')

        # Long logic to get the value from the internal input if it exists.
        # If not get the default value
        internalInputType = internalInput.getType()
        if internalInput.getValue():
            internaInputValue = internalInput.getValue() 
            if internaInputValue:
                interfaceInput.setValue(internaInputValue, internalInputType)
            else:
                internalNode = internalInput.getParent() 
                internalNodeDef = internalNode.getNodeDef() if internalNode else None
                internalNodeDefInput = internalNodeDef.getInput(interfaceName) if internalNodeDef else None
                internaInputValue = internalNodeDefInput.getValue() if internalNodeDefInput else None
                if internaInputValue:
                    interfaceInput.setValue(internaInputValue, internalInputType)

        # Remove "value" from internal input as it's value is via a connection
        internalInput.removeAttribute('value')

        # "Connect" the internal node's input to the interface. Remove any
        # specified value
        internalInput.setInterfaceName(interfaceName)

        return interfaceInput

    @staticmethod
    def findInputsUsingInterface(nodegraph, interfaceName):

        connectedInputs = []    
        connectedOutputs = []
        interfaceInput = nodegraph.getInput(interfaceName)
        if not interfaceInput:
            return
        
        # Find all downstream connections for this interface
        
        for child in nodegraph.getChildren():
            if child == interfaceInput:
                continue

        # Remove connection on node inputs and copy interface value
        # to the input value so behaviour does not change
        if child.isA(mx.Node):
            for input in child.getInputs():
                childInterfaceName = input.getAttribute('interfacename')
                if childInterfaceName == interfaceName:
                    connectedInputs.append(input.getNamePath())

        # Remove connection on the output. Value are not copied over.
        elif child.isA(mx.Output):
            childInterfaceName = child.getAttribute('interfacename')
            if childInterfaceName == interfaceName:
                connectedOutputs.append(child.getNamePath())

        return connectedInputs, connectedOutputs

    @staticmethod
    def renameNode(node : mx.Node, newName : str, updateReferences : bool = True):
        '''
        Rename a node and update downstream references if desired
        '''

        if not node or not newName:
            return
        if not (node.isA(mx.Node) or node.isA(mx.NodeGraph)):
            return 
        if node.getName() == newName:
            return

        parent = node.getParent()
        if not parent:
            return

        newName = parent.createValidChildName(newName)

        if updateReferences:
            downStreamPorts = node.getDownstreamPorts()
            if downStreamPorts:
                for port in downStreamPorts:
                    if (port.getAttribute('nodename')):
                        port.setNodeName(newName)
                        node.setName(newName)
                    elif (port.getAttribute('nodegraph')):
                        port.setAttribute('nodegraph', newName)
                        node.setName(newName)
                    elif (port.getAttribute('interfacename')):
                        port.setAttribute('interfacename', newName)
                        node.setName(newName)
        else:
            node.setName(newName)

    @staticmethod
    def unconnectInterface(nodegraph, interfaceName, removeInterface=True):
        '''
        Remove an interface input from a nodegraph
        '''
        interfaceInput = nodegraph.getInput(interfaceName)
        if not interfaceInput:
            return
        
        # Find all downstream connections for this interface
        for child in nodegraph.getChildren():
            if child == interfaceInput:
                continue

            # Remove connection on node inputs and copy interface value
            # to the input value so behaviour does not change
            if child.isA(mx.Node):
                for input in child.getInputs():
                    childInterfaceName = input.getAttribute('interfacename')
                    if childInterfaceName == interfaceName:
                        input.setValueString(interfaceInput.getValueString())
                        input.removeAttribute('interfacename')

            # Remove connection on the output. Value are not copied over.
            elif child.isA(mx.Output):
                childInterfaceName = child.getAttribute('interfacename')
                if childInterfaceName == interfaceName:
                    input.removeAttribute('interfacename')

        if removeInterface:
            nodegraph.removeChild(interfaceName)
