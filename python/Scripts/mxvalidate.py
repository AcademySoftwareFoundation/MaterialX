#!/usr/bin/env python
'''
Verify that the given file is a valid MaterialX document.
'''

import argparse
import sys

import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Verify that the given file is a valid MaterialX document.")
    parser.add_argument("--resolve", dest="resolve", action="store_true", help="Resolve inheritance and string substitutions.")
    parser.add_argument("--verbose", dest="verbose", action="store_true", help="Print summary of elements found in the document.")
    parser.add_argument("--stdlib", dest="stdlib", action="store_true", help="Import standard MaterialX libraries into the document.")
    parser.add_argument("--enhanced", dest="enhanced", action="store_true", help="Perform enhanced validation including surface shader checks, node connections, and material structure.")
    parser.add_argument(dest="inputFilename", help="Filename of the input document.")
    opts = parser.parse_args()

    # Load standard libraries if requested.
    stdlib = None
    if opts.stdlib:
        stdlib = mx.createDocument()
        try:
            mx.loadLibraries(mx.getDefaultDataLibraryFolders(), mx.getDefaultDataSearchPath(), stdlib)            
        except Exception as err:
            print(err)
            sys.exit(0)

    # Read and validate the source document.
    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
        if stdlib:
            doc.setDataLibrary(stdlib)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)
    
    # Basic validation
    valid, message = doc.validate()
    if (valid):
        print("%s is a valid MaterialX document in v%s" % (opts.inputFilename, mx.getVersionString()))
    else:
        print("%s is not a valid MaterialX document in v%s" % (opts.inputFilename, mx.getVersionString()))
        print(message)
        sys.exit(1)

    # Perform enhanced validation if requested
    if opts.enhanced:
        errors, warnings = perform_enhanced_validation(doc)
        
        # Print validation results
        if errors:
            print("\nEnhanced Validation Errors:")
            for error in errors:
                print("  ❌ %s" % error)
        
        if warnings:
            print("\nEnhanced Validation Warnings:")
            for warning in warnings:
                print("  ⚠️  %s" % warning)
        
        # Exit with error code if there are errors
        if errors:
            print("\nEnhanced validation FAILED")
            sys.exit(1)
        elif warnings:
            print("\nEnhanced validation PASSED with warnings")
        else:
            print("\nEnhanced validation PASSED")

    # Generate verbose output if requested.
    if opts.verbose:
        nodegraphs = doc.getNodeGraphs()
        materials = doc.getMaterialNodes()
        looks = doc.getLooks()
        lookgroups = doc.getLookGroups()
        collections = doc.getCollections()
        nodedefs = doc.getNodeDefs()
        implementations = doc.getImplementations()
        geominfos = doc.getGeomInfos()
        geompropdefs = doc.getGeomPropDefs()
        typedefs = doc.getTypeDefs()
        propsets = doc.getPropertySets()
        variantsets = doc.getVariantSets()
        backdrops = doc.getBackdrops()

        print("----------------------------------")
        print("Document Version: {}.{:02d}".format(*doc.getVersionIntegers()))
        print("%4d Custom Type%s%s" % (len(typedefs), pl(typedefs), listContents(typedefs, opts.resolve)))
        print("%4d Custom GeomProp%s%s" % (len(geompropdefs), pl(geompropdefs), listContents(geompropdefs, opts.resolve)))
        print("%4d NodeDef%s%s" % (len(nodedefs), pl(nodedefs), listContents(nodedefs, opts.resolve)))
        print("%4d Implementation%s%s" % (len(implementations), pl(implementations), listContents(implementations, opts.resolve)))
        print("%4d Nodegraph%s%s" % (len(nodegraphs), pl(nodegraphs), listContents(nodegraphs, opts.resolve)))
        print("%4d VariantSet%s%s" % (len(variantsets), pl(variantsets), listContents(variantsets, opts.resolve)))
        print("%4d Material%s%s" % (len(materials), pl(materials), listContents(materials, opts.resolve)))
        print("%4d Collection%s%s" % (len(collections), pl(collections), listContents(collections, opts.resolve)))
        print("%4d GeomInfo%s%s" % (len(geominfos), pl(geominfos), listContents(geominfos, opts.resolve)))
        print("%4d PropertySet%s%s" % (len(propsets), pl(propsets), listContents(propsets, opts.resolve)))
        print("%4d Look%s%s" % (len(looks), pl(looks), listContents(looks, opts.resolve)))
        print("%4d LookGroup%s%s" % (len(lookgroups), pl(lookgroups), listContents(lookgroups, opts.resolve)))
        print("%4d Top-level backdrop%s%s" % (len(backdrops), pl(backdrops), listContents(backdrops, opts.resolve)))
        print("----------------------------------")

def perform_enhanced_validation(doc):
    """
    Perform enhanced validation checks beyond basic MaterialX validation.
    
    Args:
        doc: MaterialX document to validate
        
    Returns:
        tuple: (errors, warnings) lists
    """
    errors = []
    warnings = []
    
    # Validate surface shader setup (when surface shaders exist)
    validate_surface_shader_setup(doc, errors, warnings)
    
    # Validate node connections
    validate_node_connections(doc, errors, warnings)
    
    # Validate material structure (when materials exist)
    validate_material_structure(doc, errors, warnings)
    
    return errors, warnings

def validate_surface_shader_setup(doc, errors, warnings):
    """Validate surface shader setup and connections when surface shaders exist."""
    # Check for surface shader elements (any direct element can be a surface shader)
    surface_shaders = []
    
    # Check for surface shader elements (direct elements, not nodes)
    for elem in doc.getChildren():
        # Any direct element can be a surface shader - we don't restrict to specific types
        surface_shaders.append(elem.getName())
    
    # Check for incorrect node-based surface shader usage
    # Only flag known shader types that should be direct elements, not nodes
    known_direct_shaders = ["standard_surface", "gltf_pbr", "disney_principled", "open_pbr_surface"]
    for node in doc.getNodes():
        if node.getType() in known_direct_shaders:
            errors.append(f"Invalid {node.getType()} node '{node.getName()}' found - {node.getType()} should be a direct element, not a node")
    
    # Only validate material connections if surface shaders exist
    if surface_shaders:
        # Check material connections to surface shaders
        materials = doc.getMaterialNodes()
        for material in materials:
            material_has_shader = False
            for input_elem in material.getInputs():
                if input_elem.getType() == "surfaceshader" and input_elem.getNodeName():
                    material_has_shader = True
                    # Verify the referenced element exists
                    shader_elem = doc.getChild(input_elem.getNodeName())
                    if shader_elem:
                        # Any direct element can be a surface shader - we don't restrict types
                        break
                    else:
                        errors.append(f"Material '{material.getName()}' references non-existent surface shader '{input_elem.getNodeName()}'")
                    break
            
            if not material_has_shader:
                warnings.append(f"Material '{material.getName()}' has no 'surfaceshader' input connection")

def validate_node_connections(doc, errors, warnings):
    """Validate node connections."""
    nodes = doc.getNodes()
    disconnected_nodes = []
    orphaned_inputs = []
    nonexistent_references = []
    
    for node in nodes:
        node_has_connections = False
        node_has_values = False
        
        for input_elem in node.getInputs():
            if input_elem.getNodeName():
                node_has_connections = True
                # Check if referenced node exists
                referenced_node = doc.getNode(input_elem.getNodeName())
                if not referenced_node:
                    nonexistent_references.append(f"Node '{node.getName()}' references non-existent node '{input_elem.getNodeName()}'")
            elif input_elem.getValueString():
                node_has_values = True
            elif input_elem.getNodeGraphString():
                # Input connects to a nodegraph output - this is valid
                node_has_connections = True
            elif not input_elem.getConnectedOutput():
                # Input has no value and no connection
                orphaned_inputs.append(f"Node '{node.getName()}' input '{input_elem.getName()}' has no value or connection")
        
        # Check if node has any values or connections
        # Skip standard_surface nodes (they should be direct elements, not nodes)
        if node.getType() == "standard_surface":
            continue
        
        # Enhanced validation for specific node types
        node_type = node.getType()
        node_name = node.getName()
        
        # Get the actual node type from the element name
        actual_node_type = None
        try:
            # Try to get the node type from the element name
            if hasattr(node, 'getNodeString'):
                actual_node_type = node.getNodeString()
            else:
                # Fallback: check if the node name suggests the type
                if 'constant' in node_name.lower():
                    actual_node_type = 'constant'
                elif 'image' in node_name.lower():
                    actual_node_type = 'image'
                # Also check the element name directly
                elif hasattr(node, 'getCategory'):
                    actual_node_type = node.getCategory()
        except:
            pass
        
        # Check constant nodes - they must have values or connections
        if actual_node_type == 'constant' or node_type in ['constant', 'ramplr', 'ramp4']:
            has_value_or_connection = False
            for input_elem in node.getInputs():
                if input_elem.getValueString() or input_elem.getNodeName():
                    has_value_or_connection = True
                    break
            
            if not has_value_or_connection:
                disconnected_nodes.append(f"Constant node '{node.getName()}' has no values or connections (check 'value' input)")
        
        # Check image nodes - they must have file input or connections
        elif actual_node_type == 'image' or node_type == 'image':
            has_file_or_connection = False
            file_input = node.getInput('file')
            if file_input and file_input.getValueString():
                has_file_or_connection = True
            
            # Check if any input has a connection
            for input_elem in node.getInputs():
                if input_elem.getNodeName():
                    has_file_or_connection = True
                    break
            
            if not has_file_or_connection:
                disconnected_nodes.append(f"Image node '{node.getName()}' has no file or connections (check 'file' input)")
        
        # Flag nodes that have inputs but no values or connections
        elif len(node.getInputs()) > 0 and not node_has_connections and not node_has_values:
            input_names = [inp.getName() for inp in node.getInputs()]
            disconnected_nodes.append(f"Node '{node.getName()}' has inputs {input_names} but no values or connections")
        
        # Flag nodes that should have inputs but don't have any at all
        elif len(node.getInputs()) == 0 and (actual_node_type in ['constant', 'image'] or node_type in ['constant', 'image']):
            disconnected_nodes.append(f"Node '{node.getName()}' has no inputs at all (should have at least one input)")
    
    # Add detailed error messages for each issue
    for node_name in disconnected_nodes:
        errors.append(node_name)
    
    for orphaned_input in orphaned_inputs:
        errors.append(orphaned_input)
    
    for ref in nonexistent_references:
        errors.append(f"Reference error: {ref}")

def validate_material_structure(doc, errors, warnings):
    """Validate proper material structure when materials exist."""
    materials = doc.getMaterialNodes()
    
    # Only validate if materials exist
    if not materials:
        return
    
    for material in materials:
        # Check for surfaceshader input
        surfaceshader_input = None
        for input_elem in material.getInputs():
            if input_elem.getType() == "surfaceshader":
                surfaceshader_input = input_elem
                break
        
        if surfaceshader_input:
            # If surfaceshader input exists, it should be properly connected
            if not surfaceshader_input.getNodeName() and not surfaceshader_input.getNodeGraphString():
                errors.append(f"Material '{material.getName()}' input 'surfaceshader' not connected to any node or nodegraph")
            else:
                # Check if the referenced node actually exists
                referenced_node_name = surfaceshader_input.getNodeName()
                if referenced_node_name:
                    referenced_node = doc.getChild(referenced_node_name)
                    if not referenced_node:
                        errors.append(f"Material '{material.getName()}' references non-existent surface shader '{referenced_node_name}'")
                    # Don't restrict shader types - any direct element can be a surface shader
                # Nodegraph connections are also valid
        
        # Check for proper material type
        if material.getType() != "material":
            warnings.append(f"Material '{material.getName()}' has incorrect type '{material.getType()}' (should be 'material')")

def listContents(elemlist, resolve):
    if len(elemlist) == 0:
        return ''
    names = []
    for elem in elemlist:

        if elem.isA(mx.NodeDef):
            outtype = elem.getType()
            outs = ""
            if outtype == "multioutput":
                for ot in elem.getOutputs():
                    outs = outs + \
                        '\n\t    %s output "%s"' % (ot.getType(), ot.getName())
            names.append('%s %s "%s"%s' %
                         (outtype, elem.getNodeString(), elem.getName(), outs))
            names.append(listNodedefInterface(elem))

        elif elem.isA(mx.Implementation):
            impl = elem.getName()
            targs = []
            if elem.hasTarget():
                targs.append("target %s" % elem.getTarget())
            if targs:
                impl = "%s (%s)" % (impl, ", ".join(targs))
            if elem.hasFunction():
                if elem.hasFile():
                    impl = "%s [%s:%s()]" % (
                        impl, elem.getFile(), elem.getFunction())
                else:
                    impl = "%s [function %s()]" % (impl, elem.getFunction())
            elif elem.hasFile():
                impl = "%s [%s]" % (impl, elem.getFile())
            names.append(impl)

        elif elem.isA(mx.Backdrop):
            names.append('%s: contains "%s"' %
                         (elem.getName(), elem.getContainsString()))

        elif elem.isA(mx.NodeGraph):
            nchildnodes = len(elem.getChildren()) - elem.getOutputCount()
            backdrops = elem.getBackdrops()
            nbackdrops = len(backdrops)
            outs = ""
            if nbackdrops > 0:
                for bd in backdrops:
                    outs = outs + '\n\t    backdrop "%s"' % (bd.getName())
                    outs = outs + ' contains "%s"' % bd.getContainsString()
            if elem.getOutputCount() > 0:
                for ot in elem.getOutputs():
                    outs = outs + '\n\t    %s output "%s"' % (ot.getType(), ot.getName())
                    outs = outs + traverseInputs(ot, "", 0)
            nd = elem.getNodeDef()
            if nd:
                names.append('%s (implementation for nodedef "%s"): %d nodes%s' % (
                    elem.getName(), nd.getName(), nchildnodes, outs))
            else:
                names.append("%s: %d nodes, %d backdrop%s%s" % (
                    elem.getName(), nchildnodes, nbackdrops, pl(backdrops), outs))

        elif elem.isA(mx.Node, mx.SURFACE_MATERIAL_NODE_STRING):
            shaders = mx.getShaderNodes(elem)
            names.append("%s: %d connected shader node%s" % (elem.getName(), len(shaders), pl(shaders)))
            for shader in shaders:
                names.append('Shader node "%s" (%s), with bindings:%s' % (shader.getName(), shader.getCategory(), listShaderBindings(shader)))

        elif elem.isA(mx.GeomInfo):
            props = elem.getGeomProps()
            if props:
                propnames = " (Geomprops: " + ", ".join(map(
                            lambda x: "%s=%s" % (x.getName(), getConvertedValue(x)), props)) + ")"
            else:
                propnames = ""

            tokens = elem.getTokens()
            if tokens:
                tokennames = " (Tokens: " + ", ".join(map(
                             lambda x: "%s=%s" % (x.getName(), x.getValueString()), tokens)) + ")"
            else:
                tokennames = ""
            names.append("%s%s%s" % (elem.getName(), propnames, tokennames))

        elif elem.isA(mx.VariantSet):
            vars = elem.getVariants()
            if vars:
                varnames = " (variants " + ", ".join(map(
                           lambda x: '"' + x.getName()+'"', vars)) + ")"
            else:
                varnames = ""
            names.append("%s%s" % (elem.getName(), varnames))

        elif elem.isA(mx.PropertySet):
            props = elem.getProperties()
            if props:
                propnames = " (" + ", ".join(map(
                           lambda x: "%s %s%s" % (x.getType(), x.getName(), getTarget(x)), props)) + ")"
            else:
                propnames = ""
            names.append("%s%s" % (elem.getName(), propnames))

        elif elem.isA(mx.LookGroup):
            lks = elem.getLooks()
            if lks:
                names.append("%s (looks: %s)" % (elem.getName(), lks))
            else:
                names.append("%s (no looks)" % (elem.getName()))

        elif elem.isA(mx.Look):
            mas = ""
            if resolve:
                mtlassns = elem.getActiveMaterialAssigns()
            else:
                mtlassns = elem.getMaterialAssigns()
            for mtlassn in mtlassns:
                mas = mas + "\n\t    MaterialAssign %s to%s" % (
                    mtlassn.getMaterial(), getGeoms(mtlassn, resolve))
            pas = ""
            if resolve:
                propassns = elem.getActivePropertyAssigns()
            else:
                propassns = elem.getPropertyAssigns()
            for propassn in propassns:
                propertyname = propassn.getAttribute("property")
                pas = pas + "\n\t    PropertyAssign %s %s to%s" % (
                    propassn.getType(), propertyname, getGeoms(propassn, resolve))

            psas = ""
            if resolve:
                propsetassns = elem.getActivePropertySetAssigns()
            else:
                propsetassns = elem.getPropertySetAssigns()
            for propsetassn in propsetassns:
                propertysetname = propsetassn.getAttribute("propertyset")
                psas = psas + "\n\t    PropertySetAssign %s to%s" % (
                    propertysetname, getGeoms(propsetassn, resolve))

            varas = ""
            if resolve:
                variantassns = elem.getActiveVariantAssigns()
            else:
                variantassns = elem.getVariantAssigns()
            for varassn in variantassns:
                varas = varas + "\n\t    VariantAssign %s from variantset %s" % (
                    varassn.getVariantString(), varassn.getVariantSetString())

            visas = ""
            if resolve:
                visassns = elem.getActiveVisibilities()
            else:
                visassns = elem.getVisibilities()
            for vis in visassns:
                visstr = 'on' if vis.getVisible() else 'off'
                visas = visas + "\n\t    Set %s visibility%s %s to%s" % (
                    vis.getVisibilityType(), getViewerGeoms(vis), visstr, getGeoms(vis, resolve))

            names.append("%s%s%s%s%s%s" %
                         (elem.getName(), mas, pas, psas, varas, visas))

        else:
            names.append(elem.getName())
    return ":\n\t" + "\n\t".join(names)

def listShaderBindings(shader):
    s = ''
    for inp in shader.getInputs():
        bname = inp.getName()
        btype = inp.getType()
        if inp.hasOutputString():
            outname = inp.getOutputString()
            if inp.hasNodeGraphString():
                ngname = inp.getNodeGraphString()
                s = s + '\n\t    %s "%s" -> nodegraph "%s" output "%s"' % (btype, bname, ngname, outname)
            else:
                s = s + '\n\t    %s "%s" -> output "%s"' % (btype, bname, outname)
        else:
            bval = getConvertedValue(inp)
            s = s + '\n\t    %s "%s" = %s' % (btype, bname, bval)
    return s

def listNodedefInterface(nodedef):
    s = ''
    for inp in nodedef.getActiveInputs():
        iname = inp.getName()
        itype = inp.getType()
        if s:
            s = s + '\n\t'
        s = s + '    %s input "%s"' % (itype, iname)
    for tok in nodedef.getActiveTokens():
        tname = tok.getName()
        ttype = tok.getType()
        if s:
            s = s + '\n\t'
        s = s + '    %s token "%s"' % (ttype, tname)
    return s

def traverseInputs(node, port, depth):
    s = ''
    if node.isA(mx.Output):
        parent = node.getConnectedNode()
        s = s + traverseInputs(parent, "", depth+1)
    else:
        s = s + '%s%s -> %s %s "%s"' % (spc(depth), port,
                                        node.getType(), node.getCategory(), node.getName())
        ins = node.getActiveInputs()
        for i in ins:
            if i.hasInterfaceName():
                intname = i.getInterfaceName()
                s = s + \
                    '%s%s ^- %s interface "%s"' % (spc(depth+1),
                                                   i.getName(), i.getType(), intname)
            elif i.hasValueString():
                val = getConvertedValue(i)
                s = s + \
                    '%s%s = %s value %s' % (
                        spc(depth+1), i.getName(), i.getType(), val)
            else:
                parent = i.getConnectedNode()
                if parent:
                    s = s + traverseInputs(parent, i.getName(), depth+1)
        toks = node.getActiveTokens()
        for i in toks:
            if i.hasInterfaceName():
                intname = i.getInterfaceName()
                s = s + \
                    '%s[T]%s ^- %s interface "%s"' % (
                        spc(depth+1), i.getName(), i.getType(), intname)
            elif i.hasValueString():
                val = i.getValueString()
                s = s + \
                    '%s[T]%s = %s value "%s"' % (
                        spc(depth+1), i.getName(), i.getType(), val)
            else:
                s = s + \
                    '%s[T]%s error: no valueString' % (
                        spc(depth+1), i.getName())
    return s

def pl(elem):
    if len(elem) == 1:
        return ""
    else:
        return "s"

def spc(depth):
    return "\n\t    " + ": "*depth

# Return a value string for the element, converting units if appropriate
def getConvertedValue(elem):
    if elem.getType() in ["float", "vector2", "vector3", "vector4"]:
        if elem.hasUnit():
            u = elem.getUnit()
            print ("[Unit for %s is %s]" % (elem.getName(), u))
            if elem.hasUnitType():
                utype = elem.getUnitType()
                print ("[Unittype for %s is %s]" % (elem.getName(), utype))
            # NOTDONE...
    return elem.getValueString()

def getGeoms(elem, resolve):
    s = ""
    if elem.hasGeom():
        if resolve:
            s = s + ' geom "%s"' % elem.getActiveGeom()
        else:
            s = s + ' geom "%s"' % elem.getGeom()
    if elem.hasCollectionString():
        s = s + ' collection "%s"' % elem.getCollectionString()
    return s

def getViewerGeoms(elem):
    s = ""
    if elem.hasViewerGeom():
        s = s + ' viewergeom "%s"' % elem.getViewerGeom()
    if elem.hasViewerCollection():
        s = s + ' viewercollection "%s"' % elem.getViewerCollection()
    if s:
        s = " of" + s
    return s

def getTarget(elem):
    if elem.hasTarget():
        return ' [target "%s"]' % elem.getTarget()
    else:
        return ""

if __name__ == '__main__':
    main()
