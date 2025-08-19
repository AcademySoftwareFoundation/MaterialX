from __future__ import print_function
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import logging
import os

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
        logger.info("Init parent of USDSaver")
        super().__init__()

        # Setup options
        self._options = {}
        self._options[self.GEOMETRY_OPTION] = mx.Value.createValueFromStrings('true', 'boolean')
        self._options[self.FLATTEN_USD] = mx.Value.createValueFromStrings('true', 'boolean')
        self._options[self.PRINT_OUTPUT_USD] = mx.Value.createValueFromStrings('true', 'boolean')
        for key, value in self._options.items():
            logger.info(f"Option {key} = {value} (type: {type(value)})")
            logger.info(f"Has asA_bool: {hasattr(value, 'asA_bool')}")
        logger.info(f"GEOMETRY_OPTION: {self._options[self.GEOMETRY_OPTION]}, type: {type(self._options[self.GEOMETRY_OPTION])}")
        logger.info(f"Has asA_bool: {hasattr(self._options[self.GEOMETRY_OPTION], 'asA_bool')}")

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
            logger.info(f"Set option: {key} = {self._options[key].getValueString()}")

    def create_material_reference(self, materialx_file, usda_file):
        logger.info('In create material reference...')

        # Check if MaterialX file exists
        if not os.path.exists(materialx_file):
            logger.error(f"Error: The MaterialX file '{materialx_file}' does not exist.")
            return
        
        # Get options
        for o in self._options:
            logger.info(f"Option {o} = {self._options[o].getData()}")
        geometry = self._options[self.GEOMETRY_OPTION].getData()
        flatten = self._options[self.FLATTEN_USD].getData()
        print_usd = self._options[self.PRINT_OUTPUT_USD].getData()
        #geometry = True
        #flatten = True
        #print_usd = True

        # Create a new USD stage (scene)
        logger.info('create stage')
        stage = Usd.Stage.CreateInMemory()
        
        # Define a material in the USD stage (root location)
        material_path = '/World/MaterialX'
        if not flatten:
            material_path += '/Materials'
        #material_prim = UsdShade.Material.Define(stage, Sdf.Path(material_path))
        material_prim = stage.DefinePrim(Sdf.Path(material_path))
        
        # Reference the MaterialX file as the source for this material
        # Create an SdfReference object to use in AddReference()
        materialx_reference = Sdf.Reference(materialx_file, "/MaterialX")
        material_prim.GetPrim().GetReferences().AddReference(materialx_reference)

        stage.documentation = f"Stage referencing: {materialx_file}"
        temp_stage = None
        if flatten or geometry:        
            logger.info('flatten stage')
            flattened_layer = stage.Flatten()
            flattened_layer.documentation = f"Flattened stage referencing: {materialx_file}"
            temp_stage = Usd.Stage.Open(flattened_layer)

        # Set up a scene with a default sphere
        scene_path = '/World/Scene'
        SPHERE_PATH = '/World/Scene/Sphere'
        if geometry:    
            logger.info('add dummy geometry to stage')
            scene_prim = stage.DefinePrim(Sdf.Path(scene_path), 'Xform')
            sphere = UsdGeom.Sphere.Define(stage, SPHERE_PATH)
            material_binding = UsdShade.MaterialBindingAPI.Apply(sphere.GetPrim())

            # Iterate and find the first prim of type "Material" under the root
            material = None
            for child_prim in temp_stage.Traverse():
                if child_prim.GetTypeName() == "Material":
                    material = UsdShade.Material(child_prim)
                    break
            if material:
                if usda_file:
                    logger.info(f'# Bind material {material.GetPath()} to {sphere.GetPath()}')
                # Bind in main stage
                material_binding.Bind(material)
                # Bind in temp stage
                if temp_stage:
                    scene_prim = temp_stage.DefinePrim(Sdf.Path(scene_path), 'Xform')
                    sphere = UsdGeom.Sphere.Define(temp_stage, SPHERE_PATH)
                    material_binding = UsdShade.MaterialBindingAPI.Apply(sphere.GetPrim())
                    material_binding.Bind(material)

        logger.info('get usd string')
        if flatten:
            usd_string = temp_stage.ExportToString()
        else:
            usd_string = stage.GetRootLayer().ExportToString()

        # Save the stage as a USDA file
        if usda_file:
            logger.info(f'Save USD to: {usda_file}')
            # Save string to file
            with open(usda_file, 'w') as f:
                f.write(usd_string)

        if print_usd:
            logger.info(usd_string)

    def run(self, doc, path):
        if have_usd:    
            usda_file = path.replace(".mtlx", ".usda")
            
            # Write to MTLX first
            logger.info('>>>>>>>>>>> Write MaterialX to XML file...')
            mx.writeToXmlFile(doc, path)

            # Convert MaterialX to USD
            logger.info('>>>>>>>>>>> Convert MTLX To USD...')
            self.create_material_reference(path, usda_file)
            return True

        return False

# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":

    usdSaver = USDSaver()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(usdSaver)
    doc = mx.createDocument()
    mx.readFromXmlFile(doc, "./plugins/carpaint.mtlx")
    usdSaver.run(doc, "./plugins/carpaint.mtlx")

else:
    usdSaver = USDSaver()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(usdSaver)
    logger.info("Successfully registered USD module plugins.")
