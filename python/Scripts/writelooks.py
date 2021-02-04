#!/usr/bin/env python
'''
Generate the "Looks.mtlx" example file programmatically.
'''

import MaterialX as mx

def main():
    doc = mx.createDocument()

    # Add document CMS/colorspace
    doc.setColorManagementSystem("ocio")
    doc.setColorSpace("lin_rec709")

    # Prepend an XInclude reference for "SimpleSrf.mtlx".
    mx.prependXInclude(doc, "SimpleSrf.mtlx")

    #
    # Materials
    #
    shader = doc.addNode("simple_srf", "sr_mp1")
    shader.setInputValue("diffColor", mx.Color3(0.134, 0.130, 0.125))
    shader.setInputValue("specColor", mx.Color3(0.114))
    shader.setInputValue("specRoughness", 0.38)
    mplastic1 = doc.addMaterialNode("Mplastic1", shader)

    shader = doc.addNode("simple_srf", "sr_mp2")
    shader.setInputValue("diffColor", mx.Color3(0.17, 0.26, 0.23))
    shader.setInputValue("specRoughness", 0.24)
    mplastic2 = doc.addMaterialNode("Mplastic2", shader)

    shader = doc.addNode("simple_srf", "sr_mm1")
    shader.setInputValue("diffColor", mx.Color3(0.001))
    shader.setInputValue("specColor", mx.Color3(0.671, 0.676, 0.667))
    shader.setInputValue("specRoughness", 0.005)
    mmetal1 = doc.addMaterialNode("Mmetal1", shader)

    shader = doc.addNode("simple_srf", "sr_mm2")
    shader.setInputValue("diffColor", mx.Color3(0.049, 0.043, 0.033))
    shader.setInputValue("specColor", mx.Color3(0.115, 0.091, 0.064))
    shader.setInputValue("specRoughness", 0.35)
    mmetal2 = doc.addMaterialNode("Mmetal2", shader)

    #
    # VariantSet
    #
    vset = doc.addVariantSet("varset")
    vardry = vset.addVariant("dry")
    vardry.setInputValue("wetgain", 0.0)
    varwet = vset.addVariant("wet")
    varwet.setInputValue("wetgain", 1.0)
    vardamp = vset.addVariant("damp")
    vardamp.setInputValue("wetgain", 0.2)

    #
    # Collections
    #
    cplastic = doc.addCollection("c_plastic")
    cplastic.setIncludeGeom("/a/g1,/a/g2,/a/g5")
    cmetal = doc.addCollection("c_metal")
    cmetal.setIncludeGeom("/a/g3,/a/g4")
    clamphouse = doc.addCollection("c_lamphouse")
    clamphouse.setIncludeGeom("/a/lamp1/housing*Mesh")
    csetgeom = doc.addCollection("c_setgeom")
    csetgeom.setIncludeGeom("/b")

    #
    # Nodedef and lightshader material
    #
    nd_disklgt = doc.addNodeDef("ND_disk_lgt_light", "lightshader", "disk_lgt")
    nd_disklgt.setInputValue("emissionmap", "", "filename")
    nd_disklgt.setInputValue("gain", 2000.0)
    shader = doc.addNode("disk_lgt", "lgtsr1")
    shader.setInputValue("gain", 500.0)
    mheadlight = doc.addMaterialNode("mheadlight", shader)

    #
    # PropertySet
    #
    propset = doc.addPropertySet("standard")
    p = propset.addProperty("displacementbound_sphere")
    p.setTarget("rmanris")
    p.setValue(0.05)
    p = propset.addProperty("trace_maxdiffusedepth")
    p.setTarget("rmanris")
    p.setValue(3.0)

    #
    # Looks
    #
    lookA = doc.addLook("lookA")
    ma = lookA.addMaterialAssign("ma1", mplastic1.getName())
    ma.setCollection(cplastic)
    ma = lookA.addMaterialAssign("ma2", mmetal1.getName())
    ma.setCollection(cmetal)
    ma = lookA.addMaterialAssign("ma3", mheadlight.getName())
    ma.setGeom("/a/b/headlight")
    va = lookA.addVisibility("v1")
    va.setVisibilityType("shadow")
    va.setViewerGeom("/a/b/headlight")
    va.setGeom("/")
    va.setVisible(0)
    va = lookA.addVisibility("v2")
    va.setVisibilityType("shadow")
    va.setViewerGeom("/a/b/headlight")
    va.setCollection(clamphouse)
    psa = lookA.addPropertySetAssign("psa1")
    psa.setAttribute("propertyset", propset.getName())
    psa.setGeom("/")

    lookB = doc.addLook("lookB")
    ma = lookB.addMaterialAssign("ma4", mplastic2.getName())
    ma.setCollection(cplastic)
    ma = lookB.addMaterialAssign("ma5", mmetal2.getName())
    ma.setCollection(cmetal)
    pa = lookB.addPropertyAssign("pa1")
    pa.setAttribute("property", "matte")
    pa.setValue("true", "boolean")
    pa.setGeom("/g/holdout,/g/holdout2")
    psa = lookB.addPropertySetAssign("psa2")
    psa.setAttribute("propertyset", propset.getName())
    psa.setGeom("/")
    vara = lookB.addVariantAssign("va1")
    vara.setVariantSetString(vset.getName())
    vara.setVariantString(vardamp.getName())
    va = lookB.addVisibility("v3")
    va.setVisibilityType("camera")
    va.setCollection(csetgeom)
    va.setVisible(0)

    #
    # LookGroups
    #
    lookgrp = doc.addLookGroup("testlooks")
    looks = "%s,%s" % (lookA.getName(), lookB.getName())
    lookgrp.setLooks(looks)

    # Validate the doc just to be sure
    rc = doc.validate()
    if (len(rc) >= 1 and rc[0]):
        print("Mtlx document is valid.")
    else:
        print("Mtlx document is NOT valid: %s" % str(rc[1]))

    outfile = "myLooks.mtlx"
    mx.writeToXmlFile(doc, outfile)
    print("Wrote materials and looks and such to %s" % outfile)

if __name__ == '__main__':
    main()
