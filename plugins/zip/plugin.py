import logging
import os

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('ZipPlugin')

have_zip = False
try:
    import zipfile
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    logger.info("MaterialX and zip modules loaded successfully")
    have_zip = True
except ImportError:
    raise ImportError("Please ensure MaterialX and zipfile modules are installed.")

# Zip plugin
class ZipLoader(mx_render.DocumentLoaderPlugin):
    _plugin_name = "ZipLoader"
    _ui_name = "Load from Zip..."

    def __init__(self):
        super().__init__()
        self._options = {}

    def name(self):
        return self._plugin_name

    def uiName(self):
        return self._ui_name

    def supportedExtensions(self):
        return [".zip"]

    def getOptions(self, options):
        for key, value in self._options.items():
            options[key] = value

    def setOption(self, key, value):
        if key in self._options and isinstance(value, mx.Value):
            self._options[key] = value

    def run(self, path):
        # Check if path exists first
        if not os.path.isabs(path):
            path = os.path.join(os.path.dirname(__file__), path)
        if not os.path.isfile(path):
            logger.error(f"File not found: {path}")
            return None
        try:
            # Create subfolder to path w/o .zip
            unzip_folder = os.path.splitext(path)[0]
            os.makedirs(unzip_folder, exist_ok=True)
            logger.info(f"Extracting ZIP: {path} to {unzip_folder}")

            # Get list of files in zip
            fileList = []
            with zipfile.ZipFile(path, 'r') as z:
                fileList = z.namelist()
                # If there are multiple .mtlx files, extract all of them
                # but only return the first document.
                # TODO: Alternative is to merge the documents together but
                # this may not make sense.
                first_mtlx_file = None
                for file_name in fileList:
                    if file_name.endswith(".mtlx"):
                        if not first_mtlx_file:
                            first_mtlx_file = file_name

                if not first_mtlx_file:
                    logger.error(f"No MaterialX files found in ZIP: {path}")
                    return None
                else:
                    logger.info(f"Extracting all files: {fileList} to {unzip_folder}")
                    z.extractall(unzip_folder)
                    doc = mx.createDocument()
                    extracted_mtlx_path = os.path.join(unzip_folder, first_mtlx_file)
                    logger.info(f"Loading MaterialX document from first .mtlx file: {extracted_mtlx_path}")
                    mx.readFromXmlFile(doc, extracted_mtlx_path)
                    return doc

        except Exception as e:
            logger.error(f"Error extracting zip: {e}")
        return None  
    
class ZipSaver(mx_render.DocumentSaverPlugin):
    _plugin_name = "ZipSaver"
    _ui_name = "Save to Zip..."

    def name(self):
       return self._plugin_name
    
    def uiName(self):
       return self._ui_name
    
    def supportedExtensions(self):
        return [".zip"]

    def resolve_all_image_paths(self, doc):
        """
        Resolve all image paths in the MaterialX document and return list of paths found
        """
        result = []
        for elem in doc.traverseTree():
            valueElem = None
            if elem.isA(mx.ValueElement):
                valueElem = elem
            if not valueElem or valueElem.getType() != mx.FILENAME_TYPE_STRING:
                continue

            unresolvedValue = mx.FilePath(valueElem.getValueString())
            if unresolvedValue.isEmpty():
                continue

            elementResolver = valueElem.createStringResolver()
            if unresolvedValue.isAbsolute():
                elementResolver.setFilePrefix('')
            resolvedValue = valueElem.getResolvedValueString(elementResolver)
            resolvedValue = mx.FilePath(resolvedValue).getBaseName()
            valueElem.setValueString(resolvedValue)

            result.append(resolvedValue)
        return result

    def run(self, doc, path):
        if have_zip:
            with zipfile.ZipFile(path, 'w') as z:
                # Save MaterialX document to a temporary XML file
                zip_folder = os.path.dirname(path)
                document_path = os.path.join(zip_folder, "document.mtlx")
                mx.writeToXmlFile(doc, document_path)
                logger.info(f"Adding MaterialX document to ZIP: {temp_xml}")
                z.write(document_path, arcname="document.mtlx")

                # Save all texture files
                texture_file_list = self.resolve_all_image_paths(doc)
                for texture in texture_file_list:
                    logger.info(f"Adding texture to ZIP: {texture}")
                    z.write(texture, arcname=os.path.basename(texture))
                logger.info(f"MaterialX document saved to ZIP: {path}")
        return True

# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":

    # Access plugin manager
    manager = mx_render.getPluginManager()

    try:
        zipLoader = ZipLoader()
    except TypeError as e:
        raise RuntimeError(f"ZipLoader does not implement all required abstract methods: {e}")
    logger.info(f"Loader name: {zipLoader.name()}")
    manager.registerPlugin(zipLoader)
    logger.info(f"Registered plugins: {manager.getPluginList()}")

    # Get ZipLoader plugin by name and run it
    loader = manager.getLoader("ZipLoader")
    if loader:
        logger.info(f"ZipLoader plugin found, running it...")
        # Call .name() to ensure Python override is visible to C++
        doc = loader.run("brown_planks_03_1k_materialx.zip")
        if doc:
            logger.info("[Python] Document loaded:")
            logger.info(mx.prettyPrint(doc)[:200])
        else:
            logger.error("[Python] Failed to load document.")

else:
    try:
        zipLoader = ZipLoader()
    except TypeError as e:
        raise RuntimeError(f"ZipLoader  does not implement all required abstract methods: {e}")
    try:
        zipSaver = ZipSaver()
    except TypeError as e:
       raise RuntimeError(f"ZipSaver does not implement all required abstract methods: {e}")

    manager = mx_render.getPluginManager()
    manager.registerPlugin(zipLoader)
    manager.registerPlugin(zipSaver)
    logger.info("Successfully registered ZIP module plugins.")
