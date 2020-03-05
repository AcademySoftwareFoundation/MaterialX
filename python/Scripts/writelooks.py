#!/usr/bin/env python
'''
Generate the "Looks.mtlx" example file programmatically.
'''

import sys, string
import MaterialX as mx


def main():

    doc = mx.createDocument()

    # Add document CMS/colorspace
    doc.setColorManagementSystem("ocio")
    doc.setColorSpace("lin_rec709")

    # Add "SimpleSrf.mtlx" as a library, which will create an xi:include in the output file.
    xi_doc = mx.createDocument()
    inclfile = "SimpleSrf.mtlx"
    try:
	mx.readFromXmlFile(xi_doc, inclfile)
	doc.importLibrary(xi_doc)
    except mx.ExceptionFileMissing:
	print 'Unable to find "%s" in current MATERIALX_SEARCH_PATH' % inclfile

    #
    # Materials
    #
    mplastic1 = doc.addMaterial("Mplastic1")
    sref = mplastic1.addShaderRef("sr_mp1", "simple_srf")
    bin1 = sref.addBindInput("diffColor", "color3")
    bin1.setValue(mx.Color3(0.134, 0.130, 0.125))
    bin2 = sref.addBindInput("specColor", "color3")
    bin2.setValue(mx.Color3(0.114, 0.114, 0.114))
    bin3 = sref.addBindInput("specRoughness", "float")
    bin3.setValue(0.38)

    mplastic2 = doc.addMaterial("Mplastic2")
    mplastic2.setInheritsFrom(mplastic1)
    sref = mplastic2.addShaderRef("sr_mp2", "simple_srf")
    bin1 = sref.addBindInput("diffColor", "color3")
    bin1.setValue(mx.Color3(0.17, 0.26, 0.23))
    bin2 = sref.addBindInput("specRoughness", "float")
    bin2.setValue(0.24)

    mmetal1 = doc.addMaterial("Mmetal1")
    sref = mmetal1.addShaderRef("sr_mm1", "simple_srf")
    bin1 = sref.addBindInput("diffColor", "color3")
    bin1.setValue(mx.Color3(0.001, 0.001, 0.001))
    bin2 = sref.addBindInput("specColor", "color3")
    bin2.setValue(mx.Color3(0.671, 0.676, 0.667))
    bin3 = sref.addBindInput("specRoughness", "float")
    bin3.setValue(0.005)

    mmetal2 = doc.addMaterial("Mmetal2")
    sref = mmetal2.addShaderRef("sr_mm2", "simple_srf")
    bin1 = sref.addBindInput("diffColor", "color3")
    bin1.setValue(mx.Color3(0.049, 0.043, 0.033))
    bin2 = sref.addBindInput("specColor", "color3")
    bin2.setValue(mx.Color3(0.115, 0.091, 0.064))
    bin3 = sref.addBindInput("specRoughness", "float")
    bin3.setValue(0.35)

    #
    # VariantSet
    #
    vset = doc.addVariantSet("varset")
    vardry = vset.addVariant("dry")
    vardry.setParameterValue("wetgain", 0.0)
    varwet = vset.addVariant("wet")
    varwet.setParameterValue("wetgain", 1.0)
    vardamp = vset.addVariant("damp")
    vardamp.setParameterValue("wetgain", 0.2)

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
    nd_disklgt.setParameterValue("emissionmap", "", "filename")
    nd_disklgt.setParameterValue("gain", 2000.0)
    mheadlight = doc.addMaterial("mheadlight")
    sref = mheadlight.addShaderRef("lgtsr1", "disk_lgt")
    bpar = sref.addBindParam("gain", "float")
    bpar.setValue(500.0)

    #
    # Propertyset
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
    # Lookgroups
    #
    lookgrp = doc.addLookGroup("testlooks")
    looks = "%s,%s" % (lookA.getName(), lookB.getName())
    lookgrp.setLooks(looks)

    # Validate the doc just to be sure
    rc = doc.validate()
    if (len(rc) >= 1 and rc[0]):
	print "Mtlx document is valid."
    else:
	print "Mtlx document is NOT valid: %s" % str(rc[1])

    outfile = "myLooks.mtlx"
    mx.writeToXmlFile(doc, outfile)
    print "Wrote materials and looks and such to %s" % outfile


if __name__ == '__main__':
    main()

