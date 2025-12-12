from __future__ import print_function
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import logging
import os
import argparse

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('USDPlugin')

have_usd = False
try:
    from pxr import Usd, Sdf, UsdShade, UsdGeom, Gf, UsdLux, UsdUtils
    logger.info("USD modules loaded successfully")
    have_usd = True
except ImportError:
    raise ImportError("USD modules not found. Please ensure it is installed.")
    
class USDSaver(mx_render.DocumentSaverPlugin):
    _plugin_name = "USDSaver"
    _ui_name = "Export to USD..."
    GEOMETRY_OPTION = "Write Geometry"
    FLATTEN_USD = "Flatten USD"
    PRINT_OUTPUT_USD = "PrintUSD output"

    def __init__(self):
        super().__init__()

        # Setup options
        self._options = {}
        self._options[self.GEOMETRY_OPTION] = mx.Value.createValueFromStrings('true', 'boolean')
        self._options[self.FLATTEN_USD] = mx.Value.createValueFromStrings('true', 'boolean')
        self._options[self.PRINT_OUTPUT_USD] = mx.Value.createValueFromStrings('true', 'boolean')
        #for key, value in self._options.items():
        #    logger.info(f"Option {key} = {value} (type: {type(value)})")
        #    logger.info(f"Has asA_bool: {hasattr(value, 'asA_bool')}")
        #logger.info(f"GEOMETRY_OPTION: {self._options[self.GEOMETRY_OPTION]}, type: {type(self._options[self.GEOMETRY_OPTION])}")
        #logger.info(f"Has asA_bool: {hasattr(self._options[self.GEOMETRY_OPTION], 'asA_bool')}")

    def name(self):
       return self._plugin_name
    
    def uiName(self):
       return self._ui_name
    
    def supportedExtensions(self):
        return [".mtlx"]

    def getOptions(self, options):
        # Fill the provided dict with current options
        for key, value in self._options.items():
            #logger.info(f"Return option: {key} = {value.getValueString()}")
            options[key] = value    

    def setOption(self, key, value):
        # value is expected to be an mx.Value
        if key in self._options and isinstance(value, mx.Value):
            self._options[key] = value
            #logger.info(f"Set option: {key} = {self._options[key].getValueString()}")

    def set_required_validation_attributes(self, stage):
        '''
        @brief This function sets the required validation attributes for the stage.
        For now this function sets the upAxis and metersPerUnit. to Y and 1.0 respectively.
        @param stage: The stage to set the required validation attributes.
        '''
        # Set the upAxis and metersPerUnit for validation
        UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
        UsdGeom.SetStageMetersPerUnit(stage, 1.0)

    def create_material_reference(self, materialx_file, usda_file):
        """
        Create a USD stage that references a MaterialX file and optionally adds geometry.
        Flatten the reference if specified, and optionally print the USD output to console.
        """
        # Check if MaterialX file exists
        if not os.path.exists(materialx_file):
            logger.error(f"Error: The MaterialX file '{materialx_file}' does not exist.")
            return
        
        # Get options
        #for o in self._options:
        #    logger.info(f"Option {o} = {self._options[o].getData()}")
        geometry = self._options[self.GEOMETRY_OPTION].getData()
        flatten = self._options[self.FLATTEN_USD].getData()
        print_usd = self._options[self.PRINT_OUTPUT_USD].getData()

        # Create a new USD stage (scene)
        stage = Usd.Stage.CreateInMemory()
        SCENE_ROOT = '/World/Scene'
        scene_prim = stage.DefinePrim(Sdf.Path(SCENE_ROOT), 'Xform')
        # - Specify a default prim for validation
        stage.SetDefaultPrim(stage.GetPrimAtPath(SCENE_ROOT))

        # Define a material in the USD stage (root location)
        material_path = '/World/MaterialX'
        if not flatten:
            material_path += '/Materials'
        material_prim = stage.DefinePrim(Sdf.Path(material_path))
        
        # Reference the MaterialX file as the source for this material
        materialx_reference = Sdf.Reference(materialx_file, "/MaterialX")
        material_prim.GetPrim().GetReferences().AddReference(materialx_reference)

        # Add metadata to pass usdchecker for version 25.08

        stage.documentation = f"Stage referencing: {materialx_file}"
        if flatten:        
            logger.info('flatten stage')
            flattened_layer = stage.Flatten()
            flattened_layer.documentation = f"Flattened stage referencing: {materialx_file}"
            stage = Usd.Stage.Open(flattened_layer)

        # Set up a scene with a default sphere
        SPHERE_PATH = '/World/Scene/Sphere'
        if geometry:    
            logger.info('Add placeholder sphere to stage')
            sphere = UsdGeom.Sphere.Define(stage, SPHERE_PATH)
            material_binding = UsdShade.MaterialBindingAPI.Apply(sphere.GetPrim())

            # Iterate and find the first prim of type "Material" under the root
            material = None
            for child_prim in stage.Traverse():
                if child_prim.GetTypeName() == "Material":
                    material = UsdShade.Material(child_prim)
                    break
            if material:
                if usda_file:
                    logger.info(f'# Bind material {material.GetPath()} to {sphere.GetPath()}')
                # Bind in main stage
                material_binding.Bind(material)
                # Bind in temp stage
                #if temp_stage:
                #    scene_prim = temp_stage.DefinePrim(Sdf.Path(SCENE_ROOT), 'Xform')
                #    sphere = UsdGeom.Sphere.Define(temp_stage, SPHERE_PATH)
                #    material_binding = UsdShade.MaterialBindingAPI.Apply(sphere.GetPrim())
                #    material_binding.Bind(material)

        self.set_required_validation_attributes(stage)

        usd_string = stage.GetRootLayer().ExportToString()

        # Save the stage as a USDA file
        if usda_file:
            logger.info(f'Save USD to: {usda_file}')
            # Save string to file
            with open(usda_file, 'w') as f:
                f.write(usd_string)

        if print_usd:
            logger.info(usd_string)

        # Start a subprocess to run usdview on the usda file
        self.run_usdview = False
        if usda_file and self.run_usdview:
            try:
                import subprocess
                # Get absolute path of usda_file
                usda_file = os.path.abspath(usda_file)
                # Convert \ to //
                usda_file = usda_file.replace("\\", "/")
                logger.info(f'Running usdview on {usda_file}')
                # Export all current environment variables to subprocess
                env = os.environ.copy()
                subprocess.run(['usdview', usda_file], check=True, env=env)
            except Exception as e:
                logger.error(f"Failed to run usdview: {e}")

    def run(self, doc, path):
        if have_usd:    
            usda_file = path.replace(".mtlx", ".usda")
            
            # Write to MTLX first
            if doc:
                logger.info('Write MaterialX to XML file...')
                mx.writeToXmlFile(doc, path)

            # Convert MaterialX to USD
            logger.info('Convert MTLX To USD...')
            try:
                self.create_material_reference(path, usda_file)
            except Exception as e:
                logger.error(f"Failed to convert MTLX to USD: {e}")
                return False
            return True 

        return False

# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="USD Plugin for MaterialX")
    # Add required args
    parser.add_argument("input_file", help="Path to the input MaterialX file")
    parser.add_argument("-f", "--flatten", action="store_true", help="Flatten the USD stage")
    parser.add_argument("-g", "--geometry", action="store_true", help="Write geometry to the USD stage")
    parser.add_argument("-p", "--print_usd", action="store_true", help="Print the USD output to console")
    args = parser.parse_args()

    input_file = args.input_file
    if not os.path.exists(input_file):
        logger.error(f"Input file does not exist: {input_file}")
        exit(1)
    
    usdSaver = USDSaver()
    flatten = False
    geometry = False
    print_usd = False
    if args.flatten:
        flatten = True
    if args.geometry:
        geometry = True
    if args.print_usd:
        print_usd = True
    usdSaver.setOption(usdSaver.GEOMETRY_OPTION, mx.Value.createValueFromStrings(str(geometry).lower(), 'boolean'))
    usdSaver.setOption(usdSaver.FLATTEN_USD, mx.Value.createValueFromStrings(str(flatten).lower(), 'boolean'))
    usdSaver.setOption(usdSaver.PRINT_OUTPUT_USD, mx.Value.createValueFromStrings(str(print_usd).lower(), 'boolean'))
    manager = mx_render.getPluginManager()
    manager.registerPlugin(usdSaver)
    usdSaver.run(None, input_file)

else:
    usdSaver = USDSaver()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(usdSaver)
    logger.info("Successfully registered USD module plugins.")
