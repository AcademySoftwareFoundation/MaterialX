import maya.cmds as cmds
import maya.api.OpenMaya as OpenMaya
import maya.app.renderSetup.common.utils as commonUtils
from maya.app.renderSetup.model.selection import Selection
import maya.app.renderSetup.model.connectionOverride as connectionOverride
import maya.app.renderSetup.model.plug as plug

sel = Selection()
sel.add('|Layout')

assignments = {}
materials = {}
shaders = {}
opgraphs = {}
file = open('/home/dalgos/Desktop/TestingStuff/Layout.mat', 'w')

def exportOpgraph(node, type):
    if cmds.objectType(node.name()) != "file":
        raise Exception(cmds.objectType(node.name())+" != file")
    plg = plug.findPlug(node.name(), "computedFileTextureNamePattern")
    splits = plg.value.split('/')[-1].split('<')
    if len(splits) == 3: # 1-based
        name = splits[0].rsplit('_',1)[0]
        attrs = {}
        opgraphs[name] = attrs
        attrs['type'] = type
        return (name, 'out')
    raise

def exportShader(shader):
    if shader.name() in shaders:
        return
    type = cmds.objectType(shader.name())
    attrs = {}
    attrs['type'] = 'unknown'
    shaders[shader.name()] = attrs    
    if type == "displacementShader":
        attrs["type"] = "displacement"
        toTranslate = ('scale',) + filter(lambda n: plug.findPlug(shader.name(), n).plug.isDestination, ('displacement', 'vectorDisplacement'))
    elif type == "aiStandard":
        toTranslate = ('transmittance', 'opacity', 'specular_anisotropy', 'IOR', 'Kt', 'Kt_color', 'Kd_color', 'Kd', 'Ks_color', 'Ks', 'specular_roughness', 'emission', 'emission_color')
        attrs["type"] = "standard"
    for key in toTranslate:
        attrs[key] = {}
        plg = plug.findPlug(shader.name(), key)
        attrs[key]['type'] = 'float' if not isinstance(plg.value, list) else 'color3'
        if not plg.plug.isDestination:
            attrs[key]['value'] = str(plg.value).replace('[','').replace(']','')
        else:
            attrs[key]['opgraph'], attrs[key]['graphoutput'] = exportOpgraph(OpenMaya.MFnDependencyNode(plg.plug.source().node()), attrs[key]['type'])
    return shader.name()

def exportMaterial(engine):
    if engine.name() in materials:
        return engine.name()
    shaders = {}
    materials[engine.name()] = shaders
    # surface
    surface = OpenMaya.MFnDependencyNode(engine.findPlug("surfaceShader", False).source().node())
    shaders['surface'] = surface.name()
    exportShader(surface)
    # displacement
    dispPlug = engine.findPlug("displacementShader", False)
    if dispPlug.isDestination:
        displacement = OpenMaya.MFnDependencyNode(dispPlug.source().node())
        shaders['displacement'] = displacement.name()
        exportShader(displacement)
    return engine.name()

for shape in sel.shapes():
    engine = None
    for (_,dst) in connectionOverride.dagPathToSEConnections(shape):
        engine = dst.node()
        break
    if not isinstance(engine, OpenMaya.MObject):
        continue
    engine = OpenMaya.MFnDependencyNode(engine)
    if engine.name() not in assignments:
        assignments[engine.name()] = []
        exportMaterial(engine)
    assignments[engine.name()].append(shape.fullPathName())

shaderKeys = shaders.keys()
list.sort(shaderKeys)
#file.write(shaderKeys)

file.write('<?xml version="1.0" encoding="UTF-8"?>\n')
file.write('<materialx version="1.25">\n')

for opgraph, params in opgraphs.iteritems():
    file.write('  <opgraph name="%s">\n' % opgraph)
    file.write('    <image name="img" type="%s">\n' % params['type'])
    file.write('      <parameter name="file" type="filename" value="/home/beauchc/dev/usdToArnold/data/usd/sven_images/%s_u2_v&#60;udim&#62;.tx" />\n' % opgraph)
    file.write('    </image>\n')
    file.write('    <output name="out" type="%s">\n' % params['type'])
    file.write('      <parameter name="in" type="opgraphnode" value="img" />\n')
    file.write('    </output>\n')
    file.write('  </opgraph>\n')

for shader, attrs in shaders.iteritems():
    file.write('  <shader name="%s" shaderprogram="%s">\n' % (shader, attrs['type']))
    for attr, params in attrs.iteritems():
        if attr == "type":
            continue
        file.write('    <input name="%s" %s/>\n' % (attr, ' '.join((key+'="'+value+'"' for key,value in params.iteritems()))))
#            (attr, 'color3' if isinstance(value, list) else 'float', str(value).replace('[','').replace(']',''))
    file.write('  </shader>\n')

for material, sh in materials.iteritems():
    file.write('  <material name="%s">\n' % material)
    for type, shader in sh.iteritems():
        file.write('    <shaderref name="%s" shadertype="%s" />\n' % (shader, type))
    file.write('  </material>\n')

for shader, paths in assignments.iteritems():
   file.write('  <collection name="%s_col">\n' % shader)
   for i,path in enumerate(paths):
       p = '/'.join(path.split('|')[0:-1])
       file.write('    <collectionadd name="ca%d" geom="%s"/>\n' % (i, p))
   file.write('  </collection>\n')

file.write('  <look name="Ground_look">\n')
for shader in assignments.iterkeys():
    file.write('    <materialassign name="%s" collection="%s_col"/>\n' % (shader, shader))
file.write('  </look>\n')

file.write('</materialx>\n')

file.close()


