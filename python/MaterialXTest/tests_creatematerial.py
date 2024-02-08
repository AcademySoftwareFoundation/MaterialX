# -*- coding: utf-8 -*-
"""
Documentation: Unit tests for create material from texture directory in MaterialX Python.
"""
import os
import sys
import unittest

import MaterialX

sys.path.append('Scripts')
import creatematerial

MaterialxDir = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))

class TestCreateMaterial(unittest.TestCase):

    def test_getShaderModels(self):

        exampleModelName = "standard_surface"

        allShaderModels = creatematerial.getShaderModels()
        self.assertIsInstance(allShaderModels, dict)

        shaderModel = allShaderModels.get(exampleModelName)
        self.assertIsInstance(shaderModel, MaterialX.NodeDef)
        self.assertGreater(len(shaderModel.getInputs()), 0)

    def test_findBestMatch(self):

        exampleModelName = "standard_surface"
        allShaderModels = creatematerial.getShaderModels()
        shaderModel = allShaderModels.get(exampleModelName)

        texturename1 = "material_name_base_color"
        inputname1 = "base_color"

        texturename2 = "material_name_unknown.1001.png"
        inputname2 = "unknown"

        # Test case 1: Matching input
        result = creatematerial.findBestMatch(texturename1, shaderModel)
        self.assertEqual(result.getName(), inputname1)

        # Test case 2: No matching input
        result = creatematerial.findBestMatch(texturename2, shaderModel)
        self.assertIsNone(result)

    def test_udimFile(self):

        os.makedirs("__temp")
        filePattern = "__temp/filename_basecolor{}png".format(creatematerial.Constant.UdimToken)

        udimsFiles = []
        for i in range(1001, 1006):
            filepath = filePattern.replace(creatematerial.Constant.UdimToken, ".{}.".format(i))
            udimsFiles.append(filepath)
            with open(filepath, 'w') as f:
                pass

        with open('__temp/extrafile.png', 'w') as f:
            pass


        udimFile1 = creatematerial.UdimFile(udimsFiles[0])

        self.assertEqual(udimFile1.isUdim(), True)
        self.assertEqual(udimFile1.asPattern(), filePattern)
        self.assertEqual(sorted(udimFile1.getUdimNumbers()), ["1001", "1002", "1003", "1004", "1005"])
        self.assertEqual(udimFile1.getNameWithoutExtension(), "filename_basecolor")
        self.assertEqual(udimFile1.getNumberOfUdims(), len(udimsFiles))

        for _file in os.listdir('__temp'):
            os.remove(os.path.join('__temp', _file))
        os.removedirs('__temp')


    def test_listTextures(self):

        CWD = MaterialX.FilePath.getCurrentPath()
        creatematerial.logger.info("CWD: {}".format(CWD.asString()))
        creatematerial.logger.info("MaterialxDir: {}".format(MaterialxDir.asString()))
        creatematerial.logger.info("__file__: {}".format(os.path.abspath(__file__)))

        textureDir1 = MaterialX.FilePath(os.path.join(MaterialxDir, "resources/Materials/Examples/StandardSurface/chess_set"))
        result1 = creatematerial.listTextures(textureDir1)
        creatematerial.logger.info("Listing texture form folder: {}".format(textureDir1.asString()))
        self.assertIsInstance(result1, list)
        self.assertGreater(len(result1), 0)
        self.assertIsInstance(result1[0], creatematerial.UdimFile)
        self.assertFalse(result1[0].isUdim())

        textureDir2 = MaterialX.FilePath(os.path.join(MaterialxDir, "resources/Materials/Examples/StandardSurface"))
        result2 = creatematerial.listTextures(textureDir2)
        self.assertIsInstance(result2, list)
        self.assertEqual(len(result2), 0)

    def test_create_mtlx_doc(self):

        texturesRoot = os.path.join(MaterialxDir,"resources/Materials/Examples/StandardSurface/chess_set")

        materialName = 'queen_black.mtlx'
        mtlxFile = MaterialX.FilePath(materialName)

        textureFiles = ["queen_black_base_color.jpg", "queen_black_normal.jpg",
                        "queen_black_roughness.jpg", "queen_shared_metallic.jpg",
                        "queen_shared_scattering.jpg"]

        textureFiles = [
            creatematerial.UdimFile(os.path.abspath(os.path.join(texturesRoot, x)))
            for x in textureFiles
        ]

        # Standard Surface
        mtlxFile = creatematerial.createMtlxDoc(
            textureFiles,
            mtlxFile,
            'standard_surface',
            relativePaths=True,
            colorspace='ACEScg',
            useTileImage=True
        )
        self.assertTrue(os.path.exists(mtlxFile.asString()))

        doc = MaterialX.createDocument()
        MaterialX.readFromXmlFile(doc, mtlxFile.asString())

        self.assertTrue(doc.getNodes())
        surfaceShaderNode = doc.getNodes()[0]
        self.assertEqual(surfaceShaderNode.getCategory(), "standard_surface")
        self.assertTrue(surfaceShaderNode.getInputs())
        imageNode = surfaceShaderNode.getInputs()[0].getConnectedNode()
        self.assertEqual(imageNode.getCategory(), "tileimage")
        self.assertEqual(imageNode.getColorSpace(), "ACEScg")
        imagePath = imageNode.getInputs()[0].getAttribute('value')
        self.assertFalse(os.path.isabs(imagePath))
        os.remove(materialName)

        # USD Preview Surface
        mtlxFile = creatematerial.createMtlxDoc(
            textureFiles,
            mtlxFile,
            'UsdPreviewSurface',
            relativePaths=False,
            useTileImage=False
        )
        self.assertTrue(os.path.exists(mtlxFile.asString()))

        doc = MaterialX.createDocument()
        MaterialX.readFromXmlFile(doc, mtlxFile.asString())

        self.assertTrue(doc.getNodes())
        surfaceShaderNode = doc.getNodes()[0]
        self.assertEqual(surfaceShaderNode.getCategory(), "UsdPreviewSurface")
        self.assertTrue(surfaceShaderNode.getInputs())
        imageNode = surfaceShaderNode.getInputs()[0].getConnectedNode()
        self.assertEqual(imageNode.getCategory(), "image")
        self.assertEqual(imageNode.getColorSpace(), "srgb_texture")
        imagePath = imageNode.getInputs()[0].getAttribute('value')
        self.assertTrue(os.path.isabs(imagePath))
        os.remove(materialName)



if __name__ == "__main__":
    unittest.main()
