import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import logging
import os

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('slxPlugin')

have_slx = False
try:
    from pathlib import Path
    from mxslc.Decompiler.decompile import Decompiler
    from mxslc.compile_file import compile_file
    logger.info("SLX module loaded successfully")
    have_slx = True
except ImportError:
    raise ImportError("SLX module not found. Please ensure it is installed.")

# Get the pybind11 metaclass of Plugin
pybind_meta = type(mx_render.Plugin)

# Global registry for Python plugin classes
_registered_plugin_classes = {}

# Decorator to auto-register plugin classes
def auto_register_plugin(cls):
    #print("[Python] Registering plugin:", cls.__name__)
    _registered_plugin_classes[cls.__name__] = cls
    return cls

# Python plugin subclassing the trampoline
@auto_register_plugin
class SLXLoader(mx_render.DocumentLoaderPlugin):
    _plugin_name = "ShadingLanguageX"
    _ui_name = "Load from SLX..."

    def name(self):
        return self._plugin_name

    def uiName(self):
        return self._ui_name

    def supportedExtensions(self):
        return [".mxsl"]

    def run(self, path):
        doc = None
        mtlx_path = path.replace('.mxsl', '.mtlx')
        try:
            logger.info(f"Copiling from SLX to MaterialX: {path}...")
            compile_file(Path(path), mtlx_path)
            # Check if the MaterialX file was created
            if not os.path.exists(mtlx_path):
                logger.error("Failed to compile SLX file to MaterialX: " + path)
            else:
                doc = mx.createDocument()
                logger.info(f"Compiled SLX file to MaterialX: {mtlx_path}")
                mx.readFromXmlFile(doc, mtlx_path)                
        except Exception as e:
            logger.error(f"Failed to compile SLX file: {e}")                
        return doc

# Function to register all plugin instances
def register_all_plugins():
    manager = mx_render.getPluginManager()
    for cls_name, cls in _registered_plugin_classes.items():
        instance = cls()  # Instantiate trampoline subclass
        logger.info(f"Registering plugin instance: {instance.name()}")
        manager.registerPlugin(instance)

# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":
    loader = SLXLoader()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(loader)
    pass

else:
    logger.info("Successfully loaded slx plugin module.")
    loader = SLXLoader()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(loader)
