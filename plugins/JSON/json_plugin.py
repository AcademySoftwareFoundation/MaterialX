import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import logging
import os

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('JSONPlugin')

have_jsoncore = False
try:
    import materialxjson.core as jsoncore
    logger.info("materialxjson.core module loaded successfully !!!!!")
    have_jsoncore = True
except ImportError:
    raise ImportError("materialxjson.core module not found. Please ensure it is installed.")

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
class JSONLoader(mx_render.DocumentLoaderPlugin):
    _plugin_name = "JSONLoader"
    _ui_name = "Load from JSON"

    def __init__(self):
        super().__init__()
        self._options = {}
        upValue = mx.Value.createValueFromStrings('true', 'boolean')
        #print('Option upgrade = ', upValue.getValueString())
        self._options['upgrade'] = upValue

    def name(self):
        return self._plugin_name

    def uiName(self):
        return self._ui_name

    def supportedExtensions(self):
        return [".json"]

    def getOptions(self, options):
        for key, value in self._options.items():
            options[key] = value
        #print(f"[Python] JSONLoader options: {self._options}")

    def setOption(self, key, value):
        #print(f"Set option: {key} = {value.getValueString()}")
        if key in self._options and isinstance(value, mx.Value):
            self._options[key] = value

    def run(self, path):
        doc = mx.createDocument()
        # Check if path exists first
        if not os.path.isabs(path):
            path = os.path.join(os.path.dirname(__file__), path)
        if not os.path.isfile(path):
            logger.error(f"File not found: {path}")
            return doc
        try:
            readOptions = jsoncore.JsonReadOptions()
            if 'upgrade' in self._options:
                readOptions.upgradeVersion = self._options['upgrade']
            doc = jsoncore.Util.jsonFileToXml(path)
            xmlString = mx.writeToXmlString(doc)
            logger.info(f"Loaded JSON document to XMl from path: {path}. Options: {readOptions}")
            print(xmlString[:400])  # Print first 800 characters for debugging
        except Exception as e:
            logger.error(f"Error loading document: {e}")
        return doc  
    
class JSONSaver(mx_render.DocumentSaverPlugin):
    _plugin_name = "JSONSaver"
    _ui_name = "Save to JSON..."

    def name(self):
       return self._plugin_name
    
    def uiName(self):
       return self._ui_name
    
    def supportedExtensions(self):
        return [".json"]

    def run(self, doc, path):
        if have_jsoncore:
            exporter = jsoncore.MaterialXJson()
            json_result = exporter.documentToJSON(doc)
            if json_result:
                text = jsoncore.Util.jsonToJSONString(json_result, 4)
                logger.info(text[:800])
                # Get location of this file
                if not os.path.isabs(path):
                    path = os.path.join(os.path.dirname(__file__), path)
                with open(path, 'w') as f:
                    logger.info(f"JSON Saving document to path: {path}")
                    f.write(text)
        return True

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

    # Access plugin manager
    manager = mx_render.getPluginManager()

    try:
        jsonLoader = JSONLoader()
    except TypeError as e:
        raise RuntimeError(f"JSONLoader does not implement all required abstract methods: {e}")
    logger.info(f"Loader name: {jsonLoader.name()}")
    manager.registerPlugin(jsonLoader)
    logger.info(f"Registered plugins: {manager.getPluginList()}")

    # Get JSONLoader plugin by name and run it
    loader = manager.getLoader("JSONLoader")
    if loader:
        logger.info(f"JSONLoader plugin found, running it...")
        # Call .name() to ensure Python override is visible to C++
        doc = loader.run("input.json")
        logger.info("[Python] Document loaded:")
        logger.info(mx.prettyPrint(doc))

else:
    try:
        jsonLoader = JSONLoader()
    except TypeError as e:
        raise RuntimeError(f"JSONLoader does not implement all required abstract methods: {e}")
    try:
        jsonSaver = JSONSaver()
    except TypeError as e:
       raise RuntimeError(f"JSONSaver does not implement all required abstract methods: {e}")

    manager = mx_render.getPluginManager()
    manager.registerPlugin(jsonLoader)
    #opts = mx.PluginOptionsMap()
    #jsonLoader.getOptions(opts)
    #print(f"options for loader: {opts}")
    manager.registerPlugin(jsonSaver)
    #opts = mx.PluginOptionsMap()
    #jsonSaver.getOptions(opts)
    #print(f"options for saver: {opts}")
    logger.info("Successfully registered JSON module plugins.")
