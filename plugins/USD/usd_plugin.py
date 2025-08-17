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

    def __init__(self):
        super().__init__()
        # Store options as mx.ValuePtr
        self._options = {
            "geometry": mx.Value.createValue(True, mx.Value.BOOLEAN),
            "flatten": mx.Value.createValue(True, mx.Value.BOOLEAN),
        }

    def name(self):
       return self._plugin_name
    
    def uiName(self):
       return self._ui_name
    
    def supportedExtensions(self):
        return [".mtlx"]

    def getOptions(self, options):
        # Fill the provided dict with current options
        for key, value in self._options.items():
            options[key] = value

    def setOption(self, key, value):
        # value is expected to be an mx.Value
        if key in self._options and isinstance(value, mx.Value):
            self._options[key] = value

    def create_material_reference(self, materialx_file, usda_file, geometry, flatten=False):
        # Check if MaterialX file exists
        if not os.path.exists(materialx_file):
            logger.info(f"Error: The MaterialX file '{materialx_file}' does not exist.")
            return
        
        # Create a new USD stage (scene)
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
            flattened_layer = stage.Flatten()
            flattened_layer.documentation = f"Flattened stage referencing: {materialx_file}"
            temp_stage = Usd.Stage.Open(flattened_layer)

        # Set up a scene with a default sphere
        scene_path = '/World/Scene'
        SPHERE_PATH = '/World/Scene/Sphere'
        if geometry:    
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
                    print(f'# Bind material {material.GetPath()} to {sphere.GetPath()}')
                # Bind in main stage
                material_binding.Bind(material)
                # Bind in temp stage
                if temp_stage:
                    scene_prim = temp_stage.DefinePrim(Sdf.Path(scene_path), 'Xform')
                    sphere = UsdGeom.Sphere.Define(temp_stage, SPHERE_PATH)
                    material_binding = UsdShade.MaterialBindingAPI.Apply(sphere.GetPrim())
                    material_binding.Bind(material)

        if flatten:
            usd_string = temp_stage.ExportToString()
        else:
            usd_string = stage.GetRootLayer().ExportToString()

        # Save the stage as a USDA file
        if usda_file:
            # Save string to file
            with open(usda_file, 'w') as f:
                f.write(usd_string)
        else:
            print(usd_string)
        print(f"Saved USDA contents:\n {usd_string}")

    def run(self, doc, path):
        if have_usd:
            usda_file = path.replace(".mtlx", ".usda")
            # Use options stored as mx.Value
            geometry = self._options["geometry"].asA_bool()
            flatten = self._options["flatten"].asA_bool()
            mx.writeToXmlFile(doc, path)
            self.create_material_reference(path, usda_file, geometry, flatten)
        return True

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
