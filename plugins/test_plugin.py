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
        return doc    
    
    def save(self, document, path):
        print(f"> Saving PDF to: {path}")

# Optional: Test creating instance 
if __name__ == "__main__":
    plugin = PDFLoader()
    print(f"Plugin ID: {plugin.getIdentifier()}")
    print(f"Plugin Type: {plugin.getPluginType()}")
    print("Plugin test successful!")

    mx_render.registerPlugin(plugin) 
    plugins = mx_render.getPlugins('DocumentLoader')
    print(f"Registered plugins: {plugins}")
    for p in plugins:
        print(f" - {p.getIdentifier()}")
        print(f" - {p.getPluginType()}")
        result = plugin.load("test_document.pdf")  # "Load PDF document from path: test_document.pdf"
        print(mx.prettyPrint(result))
        plugin.save(result, "test_document.pdf")
