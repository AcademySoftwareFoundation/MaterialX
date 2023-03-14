#!/usr/bin/env python
"""
Utility to generate json and hpp from MaterialX nodedef

Given a node def e.g. ND_standard_surface_surfaceshader will
generate a standard_surface.json and standard_surface.hpp
The hpp/json can be used for simple reflection instead
of parsing mtlx libraries
"""

import sys
import os
import argparse
import json
import hashlib
import MaterialX as mx

INPUTFILEHASH = 0
mx_stdTypes = {
    'color3': ['MaterialX::Color3', mx.Color3(1, 1, 1)],
    'color4': ['MaterialX::Color4', mx.Color4(1, 1, 1, 1)],
    'vector4': ['MaterialX::Vector4', mx.Vector4(1, 1, 1, 1)],
    'vector3': ['MaterialX::Vector3', mx.Vector3(1, 1, 1)],
    'vector2': ['MaterialX::Vector2', mx.Vector2(1, 1)],
    'matrix33': ['MaterialX::Matrix33', None],
    'matrix44': ['MaterialX::Matrix44', None],
    'integerarray': ['std::vector<int>',  None],
    'floatarray': ['std::vector<float>', None],
    'color3array': ['std::vector<MaterialX::Color3>', None],
    'color4array': ['std::vector<MaterialX::Color4>', None],
    'vector2array': ['std::vector<MaterialX::Vector2>', None],
    'vector3array': ['std::vector<MaterialX::Vector3>', None],
    'vector4array': ['std::vector<MaterialX::Vector4>', None],
    'stringarray': ['std::vector<std::string>', None],
    'boolean': ['bool', False],
    'integer': ['int', 0],
    'file': ['std::string', ""],
    'filename': ['std::string', ""],
    'string': ['std::string', ""],
    'float': ['float', 0],

    #TODO: create custom structs (fixme)
    'lightshader': ['lightshader', None],
    'volumeshader': ['volumeshader', None],
    'displacementshader': ['displacementshader', None],
    'surfaceshader': ['surfaceshader', None],
    'BSDF': ['BSDF', None],
    'EDF': ['EDF', None],
    'VDF': ['VDF', None],
}

def _getType(mxType):
    return mx_stdTypes[mxType][0]

def _getDefault(mxType):
    return mx_stdTypes[mxType][1]

# Compute gitHash
def _computeGitHash(mtlxfile):
    with open(mtlxfile, 'r') as afile:
        buf = afile.read().encode()
        hasher = hashlib.sha1()
        hasher.update(b"blob %u\0" % len(buf))
        hasher.update(buf)
        return hasher.hexdigest()

def main():
    parser = argparse.ArgumentParser(
        description="MaterialX nodedef to json/hpp converter.")
    parser.add_argument(dest="inputFilename",
                        help="Filename of the input document.")
    parser.add_argument("--node", dest="nodedef", type=str,
                        help="Node to export")
    parser.add_argument("--stdlib", dest="stdlib", action="store_true",
                        help="Import standard MaterialX libraries into the document.")
    opts = parser.parse_args()

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
        # Git hash for tracking source document
        global INPUTFILEHASH
        INPUTFILEHASH = _computeGitHash(opts.inputFilename)

    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    if opts.stdlib:
        stdlib = mx.createDocument()
        filePath = os.path.dirname(os.path.abspath(__file__))
        searchPath = mx.FileSearchPath(os.path.join(filePath, '..', '..'))
        searchPath.append(os.path.dirname(opts.inputFilename))
        libraryFolders = ["libraries"]
        mx.loadLibraries(libraryFolders, searchPath, stdlib)
        doc.importLibrary(stdlib)

    (valid, message) = doc.validate()
    if valid:
        print("%s is a valid MaterialX document in v%s" %
              (opts.inputFilename, mx.getVersionString()))
    else:
        print("%s is not a valid MaterialX document in v%s" %
              (opts.inputFilename, mx.getVersionString()))
        print(message)

    nodedefs = doc.getNodeDefs()
    nodedef = findNodeDef(nodedefs, opts.nodedef)

    print("Document Version: {}.{:02d}".format(*doc.getVersionIntegers()))
    if nodedef is None:
        print("Nodedef %s not found" % (opts.nodedef))
    else:
        try:
            exportNodeDef(nodedef)
            print("%d NodeDef%s found.\nNode '%s' exported to %s(.json/.hpp)"
                % (len(nodedefs), pl(nodedefs), opts.nodedef, nodedef.getNodeString()))
        except Exception as e:
            print(e)
            sys.exit(0)

def findNodeDef(elemlist, nodedefname):
    if len(elemlist) == 0:
        return None
    for elem in elemlist:
        if elem.isA(mx.NodeDef) and elem.getName() == nodedefname:
            return elem
    return None

def exportNodeDef(elem):
    if elem.isA(mx.NodeDef):
        jsonfilename = elem.getNodeString()+'.json'
        hppfilename = elem.getNodeString()+'.hpp'
        export_json(elem, jsonfilename)
        export_hpp(elem, hppfilename)

def export_json(elem, filename):
    nodefInterface = {}
    nodefInterface["Nodedef"] = elem.getName()
    nodefInterface["SHA1"] = INPUTFILEHASH
    nodefInterface["MaterialX"] = mx.getVersionString()
    nodefInterface["name"] = elem.getNodeString()
    asJsonArray(nodefInterface, elem)
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(nodefInterface, f, indent=4)

def asJsonArray(nodefInterface, nodedef):
    inputs = []
    outputs = []
    for inp in nodedef.getActiveInputs():
        inputs.append((_getType(inp.getType()),
                           inp.getName(),
                           str(inp.getValue())))
    nodefInterface["inputs"] = inputs
    for output in nodedef.getActiveOutputs():
        outputs.append((_getType(output.getType()),
                           output.getName(),
                           str(output.getValue())))
    nodefInterface["outputs"] = outputs

def export_hpp(elem, filename):
    #      write to file
    preamble = "/*\nGenerated using MaterialX nodedef \
                  \n{nodename}\nSHA1:{filehash}\nVersion:{version}\n*/\n"\
                  .format(nodename=elem, filehash=INPUTFILEHASH, version=mx.getVersionString())
    variable_defs = ""
    for inp in elem.getActiveInputs():
        #create decl
        decl = getVarDeclaration(inp)
        #emit variable decl
        if decl is None:
            variable_def = '    {typename} {name};\n' \
                .format(typename=_getType(inp.getType()),
                        name=inp.getName())
        else:
            variable_def = '    {typename} {name} = {declaration};\n' \
                .format(typename=_getType(inp.getType()),
                        name=inp.getName(),
                        declaration=decl)
        variable_defs += variable_def
    for output in elem.getActiveOutputs():
        #create decl
        decl = getVarDeclaration(output)
        #emit output
        if decl is None:
            variable_def = '    {typename}* {name};\n' \
                .format(typename=_getType(output.getType()),
                        name=output.getName())
        else:
            variable_def = '    {typename} {name} = {declaration};\n' \
                .format(typename=_getType(output.getType()),
                        name=output.getName(),
                        declaration=decl)
        variable_defs += variable_def
    nodename_definition = '    std::string _nodename_ = "{nodename}";\n'.format(
        nodename=elem.getNodeString())
    # create struct definition
    struct_definition = """struct {structname} {{\n{variabledefs}{nodeiddef}}};""" \
        .format(structname=elem.getName(),
                variabledefs=variable_defs,
                nodeiddef=nodename_definition)

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(preamble)
        f.write(struct_definition)
        f.close()


def getVarDeclaration(inputVar):

    inputValue = inputVar.getValue()
    typeName = _getType(inputVar.getType())
    if isinstance(inputValue, (mx.Color3, mx.Vector3)):
        val = '{typename}({v0}f, {v1}f, {v2}f)'.format(typename=typeName,
                                                       v0=round(
                                                           inputValue[0], 5),
                                                       v1=round(
                                                           inputValue[1], 5),
                                                       v2=round(inputValue[2], 5))
        return val
    if isinstance(inputValue, (mx.Color4, mx.Vector4)):
        val = '{typename}({v0}f, {v1}f, {v2}f, {v3}f)'.format(typename=typeName,
                                                              v0=round(
                                                                  inputValue[0], 5),
                                                              v1=round(
                                                                  inputValue[1], 5),
                                                              v2=round(
                                                                  inputValue[2], 5),
                                                              v3=round(inputValue[3], 5))
        return val
    if isinstance(inputValue, float):
        val = '{0}f'.format(round(inputValue, 5))
        return val
    if isinstance(inputValue, bool):
        val = '{0}'.format('true' if inputValue is True else 'false')
        return val
    if isinstance(inputValue, int):
        val = '{0}'.format(inputValue)
        return val

    # use input type if value is not defined and set default
    defaultValue = _getDefault(inputVar.getType())
    if inputValue is None:
        if inputVar.getType() in ['vector2']:
            val = '{typename}({v0}f, {v1}f)'.format(typename=typeName,
                                                    v0=defaultValue[0],
                                                    v1=defaultValue[1])
            return val
        if inputVar.getType() in ['vector3', 'color3']:
            val = '{typename}({v0}f, {v1}f, {v2}f)'.format(typename=typeName,
                                                           v0=defaultValue[0],
                                                           v1=defaultValue[1],
                                                           v2=defaultValue[2])
            return val
        if inputVar.getType() in ['vector4', 'color4']:
            val = '{typename}({v0}f, {v1}f, {v2}f, {v3}f)'.format(typename=typeName,
                                                                  v0=defaultValue[0],
                                                                  v1=defaultValue[1],
                                                                  v2=defaultValue[2],
                                                                  v3=defaultValue[3])
            return val
        else:
            print("unhandled: " + typeName)
            return None


def pl(elem):
    if len(elem) == 1:
        return ""
    else:
        return "s"


if __name__ == '__main__':
    main()
