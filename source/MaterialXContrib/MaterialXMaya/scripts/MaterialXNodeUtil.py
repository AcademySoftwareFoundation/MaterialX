import os
import maya.cmds as cmds
import maya.mel as mel

from distutils.spawn import find_executable

def editMaterialXDocument(documentPath, element, editor):
    editor_path = find_executable(editor)
    if editor_path == None:
        print ("Unable to find editor: " + editor)
    else:
        print "Launching editor..."
        cmd = "system(\"start " + editor + " \\\"" + documentPath + "\\\"\");"
        mel.eval(cmd)

def editMaterialXNode(nodeName):
    documentFilePath = cmds.getAttr(nodeName + "documentFilePath")
    elementPath = cmds.getAttr(nodeName + "elementPath")
    if cmds.optionVar(exists='MAYA_MATERIALX_EDITOR'):
        editor = cmds.optionVar(q='MAYA_MATERIALX_EDITOR')
        editMaterialXDocument(documentFilePath, elementPath, editor)
    else:
        print "Please set optionVar: 'MAYA_MATERIALX_EDITOR'"

def getMaterialXNodesForDocument(documentPath):
    nodes = cmds.ls(type="MaterialXNode")
    results = []
    for node in nodes:
        nodeDocumentPath = cmds.getAttr(node + ".documentFilePath")
        if os.path.realpath(nodeDocumentPath) == os.path.realpath(documentPath):
            results.append(node)
    return results

def reloadMaterialXNodesForDocument(documentPath):
    nodes = getMaterialXNodesForDocument(documentPath)
    for node in nodes:
        cmds.reloadMaterialXNode(node)
