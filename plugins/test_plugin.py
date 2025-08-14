import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render

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
class PDFLoader(mx_render.DocumentLoaderPlugin):
    # Do NOT override __init__

    _plugin_name = "PDFLoader"

    def name(self):
        return self._plugin_name

    def run(self, path):
        doc = mx.createDocument()
        print(f"[Python] Loading document from path: {path}")
        return doc

# Function to register all plugin instances
def register_all_plugins():
    manager = mx_render.getPluginManager()
    for cls_name, cls in _registered_plugin_classes.items():
        instance = cls()  # Instantiate trampoline subclass
        manager.registerPlugin(instance)


# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":

    # Access plugin manager
    manager = mx_render.getPluginManager()

    try:
        pdfLoader = PDFLoader()
    except TypeError as e:
        raise RuntimeError(f"PDFLoader does not implement all required abstract methods: {e}")
    print(f"Loader name: {pdfLoader.name()}")
    manager.registerPlugin(pdfLoader)
    print(f"Registered plugins: {manager.getPluginList()}")

    # Get PDFLoader plugin by name and run it
    loader = manager.getLoader("PDFLoader")
    if loader:
        # Call .name() to ensure Python override is visible to C++
        doc = loader.run("test_document.pdf")
        print("[Python] Document loaded:")
        print(mx.prettyPrint(doc))

else:
    print("Loading PDF plugin module.")