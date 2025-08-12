import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render

class PDFLoader(mx_render.IDocumentPlugin):
    def __init__(self):
        super().__init__()
        
        # Initialize base class properties directly
        self._identifier = "com.company.pdf_loader"
        self._pluginType = "DocumentLoader"
        
    def load(self, path):
        doc = mx.createDocument()
        print(f"Loading PDF from: {path}")
        print(mx.prettyPrint(doc))
        return doc    
    
    def save(self, document, path):
        print(f"> Saving PDF to: {path}")

def verify_override(plugin):
    import inspect
    
    base_load = mx_render.IDocumentPlugin.load
    derived_load = plugin.load
    
    print(f"Is overridden: {derived_load.__qualname__ != base_load.__qualname__}")
    print(f"Python method: {inspect.getsource(derived_load)}")
    
    # Test direct call
    print("Direct call test:")
    doc = plugin.load("test.mtlx")
    print(f"Return type: {type(doc)}")


# Optional: Test creating instance 
if __name__ == "__main__":
    plugin = PDFLoader()
    print(f"Plugin ID: {plugin.getIdentifier()}")
    print(f"Plugin Type: {plugin.getPluginType()}")
    print("Plugin test successful!")

    print("Is load overridden?", 
      PDFLoader.load is not mx_render.IDocumentPlugin.load)  # Should be True
    print("Is save overridden?", 
      PDFLoader.save is not mx_render.IDocumentPlugin.save)  # Should be True

    mx_render.registerPlugin(plugin) 
    plugins = mx_render.getPlugins('DocumentLoader')
    print(f"Registered plugins: {plugins}")
    for p in plugins:
        print(f" - {p.getIdentifier()}")
        print(f" - {p.getPluginType()}")
        result = plugin.load("test_document.pdf")  # "Load PDF document from path: test_document.pdf"
        print("Loaded document:")
        print(mx.prettyPrint(result))
        plugin.save(result, "test_document.pdf")

        verify_override(plugin)

