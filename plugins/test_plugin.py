import MaterialX.PyMaterialXRender as mx_render

class PDFLoader(mx_render.IPlugin):
    def __init__(self):
        super().__init__()
        self._id = "com.company.pdf_loader"
        self._type = "DocumentLoader"
    
    def getIdentifier(self):
        return self._id
        
    def getPluginType(self):
        return self._type

# Optional: Create instance for testing
if __name__ == "__main__":
    plugin = PDFLoader()
    print(f"Plugin ID: {plugin.getIdentifier()}")
    print(f"Plugin Type: {plugin.getPluginType()}")
    print("Plugin test successful!")
plugin = PDFLoader()
print(plugin.getIdentifier())  # "com.company.pdf_loader"
print(plugin.getPluginType())  # "DocumentLoader"

