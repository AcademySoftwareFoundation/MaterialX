#!/usr/bin/env python
"""
Unit tests for importing MaterialX Python C extension modules.
"""

import sys
import unittest


# List of names of modules that are registered after importing `MaterialX`
AUTO_IMPORTED_MODULE_NAMES = [
    "MaterialX",
    "MaterialX.PyMaterialXCore",
    "MaterialX.PyMaterialXFormat",
]

# List of names of other modules that are available in `MaterialX`
OTHER_AVAILABLE_MODULE_NAMES = [
    "MaterialX.PyMaterialXGenShader",
    "MaterialX.PyMaterialXGenGlsl",
    "MaterialX.PyMaterialXGenMdl",
    "MaterialX.PyMaterialXGenMsl",
    "MaterialX.PyMaterialXGenOsl",
    "MaterialX.PyMaterialXRender",
    "MaterialX.PyMaterialXRenderGlsl",
    "MaterialX.PyMaterialXRenderMsl",
    "MaterialX.PyMaterialXRenderOsl",
]

# List of names of all modules that are available in `MaterialX`
ALL_MODULE_NAMES = AUTO_IMPORTED_MODULE_NAMES + OTHER_AVAILABLE_MODULE_NAMES


class TestMaterialXImports(unittest.TestCase):

    def setUp(self):
        """
        Before running each test, verify that no MaterialX Python modules are
        registered in `sys.modules`.
        """
        for module_name in ALL_MODULE_NAMES:
            self.assertNotIn(module_name, sys.modules.keys())

        # Initialize a list of names of other modules that are expected to be
        # imported by a particular test, apart from the ones that are imported
        # by the `MaterialX` Python package itself
        self.other_expected_modules = []

    def tearDown(self):
        """
        After running each test, remove all MaterialX Python modules from
        `sys.modules`, and verify that the MaterialX Python modules that were
        expected to have been imported were registered in `sys.modules`, but
        not the others.
        """
        # Create a copy of the names of registered modules
        sys_modules = list(sys.modules)

        # Remove all MaterialX Python modules from `sys.modules`
        for module_name in sys_modules:
            if "MaterialX" in module_name:
                del sys.modules[module_name]

        # Verify the modules that were expected in `sys.modules`
        for module_name in ALL_MODULE_NAMES:
            if module_name in (AUTO_IMPORTED_MODULE_NAMES
                               + self.other_expected_modules):
                self.assertIn(module_name, sys_modules)
            else:
                self.assertNotIn(module_name, sys_modules)

    def test_import_package(self):
        """
        Test the registered modules after importing the package as a whole.
        """
        import MaterialX

    def test_import_PyMaterialXCore(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXCore`.
        """
        import MaterialX.PyMaterialXCore

    def test_import_PyMaterialXFormat(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXFormat`.
        """
        import MaterialX.PyMaterialXFormat

    def test_import_PyMaterialXGenShader(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXGenShader`.
        """
        import MaterialX.PyMaterialXGenShader

        self.other_expected_modules = [
            "MaterialX.PyMaterialXGenShader",
        ]

    def test_import_PyMaterialXGenGlsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXGenGlsl`.
        """
        import MaterialX.PyMaterialXGenGlsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXGenShader",
            "MaterialX.PyMaterialXGenGlsl",
        ]

    def test_import_PyMaterialXGenMdl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXGenMdl`.
        """
        import MaterialX.PyMaterialXGenMdl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXGenShader",
            "MaterialX.PyMaterialXGenMdl",
        ]

    def test_import_PyMaterialXGenMsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXGenMsl`.
        """
        import MaterialX.PyMaterialXGenMsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXGenShader",
            "MaterialX.PyMaterialXGenMsl",
        ]

    def test_import_PyMaterialXGenOsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXGenOsl`.
        """
        import MaterialX.PyMaterialXGenOsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXGenShader",
            "MaterialX.PyMaterialXGenOsl",
        ]

    def test_import_PyMaterialXRender(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXRender`.
        """
        import MaterialX.PyMaterialXRender

        self.other_expected_modules = [
            "MaterialX.PyMaterialXRender",
        ]

    def test_import_PyMaterialXRenderGlsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXRenderGlsl`.
        """
        import MaterialX.PyMaterialXRenderGlsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXRender",
            "MaterialX.PyMaterialXRenderGlsl",
        ]

    def test_import_PyMaterialXRenderMsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXRenderMsl`.
        """
        import MaterialX.PyMaterialXRenderMsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXRender",
            "MaterialX.PyMaterialXRenderMsl",
        ]

    def test_import_PyMaterialXRenderOsl(self):
        """
        Test the registered modules after importing `MaterialX.PyMaterialXRenderOsl`.
        """
        import MaterialX.PyMaterialXRenderOsl

        self.other_expected_modules = [
            "MaterialX.PyMaterialXRender",
            "MaterialX.PyMaterialXRenderOsl",
        ]


if __name__ == '__main__':
    unittest.main()
