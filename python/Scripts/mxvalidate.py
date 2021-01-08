#!/usr/bin/env python
'''
Verify that a specified file is a valid MaterialX document.
'''

import sys
import os
import string
import MaterialX as mx


def usage():
    print("mxvalidate.py: verify that a specified file is a valid MaterialX document.")
    print("Usage:  mxvalidate.py [options] <file.mtlx>")
    print("    -h[elp]         Print usage information")
    print("    -v[erbose]      Print summary of elements found in the document")
    print(
        "    -vv[eryverbose] Also print material bindings, nodedef interfaces and nodegraph trees")
    print("    -i[ncludedefs]  Automatically XInclude stdlib_defs")
    print("    -r[esolve]      Resolve inheritance and string substitutions")


def main():
    verbose = 0
    incldefs = 0
    resolve = 0

    if len(sys.argv) < 2:
        usage()
        sys.exit(0)
    for arg in sys.argv[1:]:
        if arg in ['-v', '-verbose']:
            verbose = 1
        elif arg in ['-vv', '-veryverbose']:
            verbose = 2
        elif arg in ['-i', '-includedefs']:
            incldefs = 1
        elif arg in ['-r', '-resolve']:
            resolve = 1
        elif arg in ['-h', '-help', '--help']:
            usage()
            sys.exit(0)
        else:
            filename = arg

    doc = mx.createDocument()
    mxversion = mx.getVersionString()
    (mxmajorv, mxminorv, mxbuildv) = mx.getVersionIntegers()

    # The mx.readFromXmlFile() will fail with
    #   "MaterialX.ExceptionFileMissing" if the file is not found, or
    #   "MaterialX.ExceptionParseError" if the document is found but not readable.
    try:
        mx.readFromXmlFile(doc, filename)
    except mx.ExceptionFileMissing as err:
        # TODO: library doesn't correctly set missing filename if it's an XIncluded file (it's blank)
        print (err)
        return
    except mx.ExceptionParseError as err:
        print ("%s is not a valid XML file.\n%s" % (filename, err))
        return

    if incldefs:
        incldoc = mx.createDocument()
        try:
            mx.readFromXmlFile(incldoc, "stdlib_defs.mtlx")
            print ("Loaded stdlib_defs.mtlx")
            doc.importLibrary(incldoc)
        except mx.ExceptionFileMissing:
            try:
                mx.readFromXmlFile(incldoc, "stdlib/stdlib_defs.mtlx")
                print ("Loaded stdlib/stdlib_defs.mtlx")
                doc.importLibrary(incldoc)
            except mx.ExceptionFileMissing:
                mxspath = os.getenv('MATERIALX_SEARCH_PATH')
                print ('Unable to find stdlib_defs.mtlx in current MATERIALX_SEARCH_PATH "%s"' % mxspath)

    rc = doc.validate()
    if (len(rc) >= 1 and rc[0]):
        print ("%s is a valid MaterialX %s document." % (filename, mxversion))
    else:
        print ("%s is not a valid MaterialX %s document:" % (filename, mxversion))
        print (rc[1])

    if verbose:
        nodegraphs = doc.getNodeGraphs()
        materials = doc.getMaterials()
        looks = doc.getLooks()
        lookgroups = doc.getLookGroups()
        collections = doc.getCollections()
        nodedefs = doc.getNodeDefs()
        implementations = doc.getImplementations()
        geominfos = doc.getGeomInfos()
        # geompropdef was introduced in v1.36.3 codebase
        if mxminorv >= 37 or (mxminorv == 36 and mxbuildv >= 3):
            geompropdefs = doc.getGeomPropDefs()
        else:
            geompropdefs = []
        typedefs = doc.getTypeDefs()
        propsets = doc.getPropertySets()
        variantsets = doc.getVariantSets()
        backdrops = doc.getBackdrops()
        vv = (verbose > 1)

        print ("Document MaterialX Version: {}.{:02d}".format(*doc.getVersionIntegers()))
        clrmgmtsys = doc.getColorManagementSystem() or "undefined"
        clrmgmtconfig = doc.getColorManagementConfig() or "undefined"
        print ("Document CMS: %s  (config file: %s)" % (clrmgmtsys, clrmgmtconfig))

        print ("%4d Custom Type%s%s" % (len(typedefs), pl(typedefs), listContents(typedefs, resolve, vv)))
        print ("%4d Custom GeomProp%s%s" % (len(geompropdefs), pl(geompropdefs), listContents(geompropdefs, resolve, vv)))
        print ("%4d NodeDef%s%s" % (len(nodedefs), pl(nodedefs), listContents(nodedefs, resolve, vv)))
        print ("%4d Implementation%s%s" % (len(implementations), pl(implementations), listContents(implementations, resolve, vv)))
        print ("%4d Nodegraph%s%s" % (len(nodegraphs), pl(nodegraphs), listContents(nodegraphs, resolve, vv)))
        print ("%4d VariantSet%s%s" % (len(variantsets), pl(variantsets), listContents(variantsets, resolve, vv)))
        print ("%4d Material%s%s" % (len(materials), pl(materials), listContents(materials, resolve, vv)))
        print ("%4d Collection%s%s" % (len(collections), pl(collections), listContents(collections, resolve, vv)))
        print ("%4d GeomInfo%s%s" % (len(geominfos), pl(geominfos), listContents(geominfos, resolve, vv)))
        print ("%4d PropertySet%s%s" % (len(propsets), pl(propsets), listContents(propsets, resolve, vv)))
        print ("%4d Look%s%s" % (len(looks), pl(looks), listContents(looks, resolve, vv)))
        print ("%4d LookGroup%s%s" % (len(lookgroups), pl(lookgroups), listContents(lookgroups, resolve, vv)))
        print ("%4d Top-level backdrop%s%s" % (len(backdrops), pl(backdrops), listContents(backdrops, resolve, vv)))


def pl(elem):
    if len(elem) == 1:
        return ""
    else:
        return "s"


def listContents(elemlist, resolve, vv):
    if len(elemlist) == 0:
        return ''
    names = []
    for elem in elemlist:

        if elem.getCategory() == "nodedef":
            outtype = elem.getType()
            outs = ""
            if outtype == "multioutput":
                for ot in elem.getOutputs():
                    outs = outs + \
                        '\n\t    %s output "%s"' % (ot.getType(), ot.getName())
            names.append('%s %s "%s"%s' %
                         (outtype, elem.getNodeString(), elem.getName(), outs))
            if vv:
                names.append(listNodedefInterface(elem))

        elif elem.getCategory() == "implementation":
            impl = '%s for nodedef %s' % (
                elem.getName(), elem.getNodeDef().getName())
            targs = []
            if elem.hasTarget():
                targs.append("target %s" % elem.getTarget())
            if elem.hasLanguage():
                targs.append("language %s" % elem.getLanguage())
            if targs:
                impl = "%s (%s)" % (impl, string.join(targs, ", "))
            if elem.hasFunction():
                if elem.hasFile():
                    impl = "%s [%s:%s()]" % (
                        impl, elem.getFile(), elem.getFunction())
                else:
                    impl = "%s [function %s()]" % (impl, elem.getFunction())
            elif elem.hasFile():
                impl = "%s [%s]" % (impl, elem.getFile())
            names.append(impl)

        elif elem.getCategory() == "backdrop":
            names.append('%s: contains "%s"' %
                         (elem.getName(), elem.getContainsString()))

        elif elem.getCategory() == "nodegraph":
            nchildnodes = len(elem.getChildren()) - elem.getOutputCount()
            backdrops = elem.getBackdrops()
            nbackdrops = len(backdrops)
            outs = ""
            if nbackdrops > 0:
                for bd in backdrops:
                    outs = outs + '\n\t    backdrop "%s"' % (bd.getName())
                    if vv:
                        outs = outs + ' contains "%s"' % bd.getContainsString()
            if elem.getOutputCount() > 0:
                for ot in elem.getOutputs():
                    outs = outs + \
                        '\n\t    %s output "%s"' % (ot.getType(), ot.getName())
                    if vv:
                        outs = outs + traverseInputs(ot, "", 0)
            nd = elem.getNodeDef()
            if nd:
                names.append('%s (implementation for nodedef "%s"): %d nodes%s' % (
                    elem.getName(), nd.getName(), nchildnodes, outs))
            else:
                names.append("%s: %d nodes, %d backdrop%s%s" % (
                    elem.getName(), nchildnodes, nbackdrops, pl(backdrops), outs))

        elif elem.getCategory() == "material":
            shaders = []
            if resolve:
                srefs = elem.getActiveShaderRefs()
            else:
                srefs = elem.getShaderRefs()
            vvitems = []
            for sref in srefs:
                if sref.hasNodeDefString():
                    nodedefname = sref.getNodeDefString()
                    # Get nodedef with this name
                    nd = sref.getDocument().getNodeDef(nodedefname)
                    if nd:
                        nodetype = nd.getType()
                    else:
                        nodetype = "<unknown>"
                    shaders.append('%s %s(ND) "%s"' % (
                        nodetype, nd.getNodeString(), sref.getName()))
                    if vv:
                        vvitems.append((nodetype, nd.getNodeString(), sref))
                        shaders.append(listSrefBindings(
                            nodetype, nd.getNodeString(), sref))
                elif sref.hasNodeString():
                    node = sref.getNodeString()
                    # Get list of nodedefs for this (shader) node
                    nds = sref.getDocument().getMatchingNodeDefs(node)
                    if nds:
                        nodetype = nds[0].getType()
                    else:
                        nodetype = "<unknown>"
                    shaders.append('%s %s "%s"' %
                                   (nodetype, node, sref.getName()))
                    if vv:
                        vvitems.append((nodetype, node, sref))
                else:
                    shaders.append("unknown %s" % (sref.getName()))
            shnames = '(' + string.join(shaders, ", ") + ')'
            names.append("%s %s" % (elem.getName(), shnames))
            if vv:
                for i in vvitems:
                    names.append(listSrefBindings(i[0], i[1], i[2]))

        elif elem.getCategory() == "geominfo":
            # For 1.37, geomattr->geomprop
            (mxmajorv, mxminorv, mxbuildv) = mx.getVersionIntegers()
            if (mxminorv <= 36):
                props = elem.getGeomAttrs()
                if props:
                    propnames = " (Geomattrs: " + \
                        string.join(
                            map(lambda x: x.getName(), props), ", ") + ")"
                else:
                    propnames = ""
            else:
                props = elem.getGeomProps()
                if props:
                    # getConvertedValue() handles unit conversion if needed
                    propnames = " (Geomprops: " + string.join(map(lambda x: "%s=%s" %
                                                                  (x.getName(), getConvertedValue(x)), props), ", ") + ")"
                    #propnames = " (Geomprops: " + string.join(map(lambda x: "%s=%s"%(x.getName(),x.getResolvedValue().toValueString()), props), ", ") + ")"
                else:
                    propnames = ""
            tokens = elem.getTokens()
            if tokens:
                #tokennames = " (Tokens: " + string.join(map(lambda x: x.getName(), tokens), ", ") + ")"
                tokennames = " (Tokens: " + string.join(map(lambda x: "%s=%s" %
                                                            (x.getName(), x.getValueString()), tokens), ", ") + ")"
            else:
                tokennames = ""
            names.append("%s%s%s" % (elem.getName(), propnames, tokennames))

        elif elem.getCategory() == "variantset":
            vars = elem.getVariants()
            if vars:
                varnames = " (variants " + string.join(map(lambda x: '"' +
                                                           x.getName()+'"', vars), ", ") + ")"
            else:
                varnames = ""
            names.append("%s%s" % (elem.getName(), varnames))

        elif elem.getCategory() == "propertyset":
            props = elem.getProperties()
            if props:
                propnames = " (" + string.join(map(lambda x: "%s %s%s" %
                                                   (x.getType(), x.getName(), getTarget(x)), props), ", ") + ")"
            else:
                propnames = ""
            names.append("%s%s" % (elem.getName(), propnames))

        elif elem.getCategory() == "lookgroup":
            lks = elem.getLooks()
            if lks:
                names.append("%s (looks: %s)" % (elem.getName(), lks))
            else:
                names.append("%s (no looks)" % (elem.getName()))

        elif elem.getCategory() == "look":
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
    return ":\n\t" + string.join(names, "\n\t")


def listSrefBindings(nodetype, node, sref):
    s = '  Bindings for %s "%s":' % (nodetype, node)
    for inp in sref.getBindInputs():
        bname = inp.getName()
        btype = inp.getType()
        if inp.hasOutputString():
            outname = inp.getOutputString()
            if inp.hasNodeGraphString():
                ngname = inp.getNodeGraphString()
                s = s + \
                    '\n\t    %s input "%s" -> nodegraph "%s" output "%s"' % (
                        btype, bname, ngname, outname)
            else:
                s = s + \
                    '\n\t    %s input "%s" -> output "%s"' % (
                        btype, bname, outname)
        else:
            bval = getConvertedValue(inp)
            s = s + '\n\t    %s input "%s" = %s' % (btype, bname, bval)
    for parm in sref.getBindParams():
        bname = parm.getName()
        btype = parm.getType()
        bval = getConvertedValue(parm)
        s = s + '\n\t    %s parameter "%s" = %s' % (btype, bname, bval)
    for tok in sref.getBindTokens():
        bname = tok.getName()
        btype = tok.getType()
        bval = tok.getValueString()
        s = s + '\n\t    %s token "%s" = %s' % (btype, bname, bval)
    return s


def listNodedefInterface(nodedef):
    s = ''
    for inp in nodedef.getActiveInputs():
        iname = inp.getName()
        itype = inp.getType()
        if s:
            s = s + '\n\t'
        s = s + '    %s input "%s"' % (itype, iname)
    for parm in nodedef.getActiveParameters():
        pname = parm.getName()
        ptype = parm.getType()
        if s:
            s = s + '\n\t'
        s = s + '    %s parameter "%s"' % (ptype, pname)
    for tok in nodedef.getActiveTokens():
        tname = tok.getName()
        ttype = tok.getType()
        if s:
            s = s + '\n\t'
        s = s + '    %s token "%s"' % (ttype, tname)
    return s


def traverseInputs(node, port, depth):
    s = ''
    if node.getCategory() == "output":
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
        parms = node.getActiveParameters()
        for i in parms:
            if i.hasInterfaceName():
                intname = i.getInterfaceName()
                s = s + \
                    '%s[P]%s ^- %s interface "%s"' % (
                        spc(depth+1), i.getName(), i.getType(), intname)
            elif i.hasValueString():
                val = getConvertedValue(i)
                s = s + \
                    '%s[P]%s = %s value %s' % (
                        spc(depth+1), i.getName(), i.getType(), val)
            else:
                s = s + \
                    '%s[P]%s error: no valueString' % (
                        spc(depth+1), i.getName())
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
