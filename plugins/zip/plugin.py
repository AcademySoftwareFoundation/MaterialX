import logging
import os
import argparse
import pathlib
import tempfile

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

def resolve_all_image_paths(doc):
    """
    Resolve all image paths in the MaterialX document and return list of paths found
    """
    result = dict()
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
        #resolvedValue = mx.FilePath(resolvedValue).getBaseName()
        valueElem.setValueString(resolvedValue)

        result[valueElem.getNamePath()] = resolvedValue

    return result

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

    def run(self, doc, path):
        if have_zip:
            # Determine the .mtlx filename based on the .zip filename

            zip_basename = os.path.basename(path)
            zip_folder = os.path.splitext(zip_basename)[0]
            mtlx_name = os.path.join(zip_folder, zip_folder + ".mtlx")
            logger.info(f"zip base name: {zip_basename}, mtlx name: {mtlx_name}")

            # Write the document to a temporary file in the correct subdirectory
            with tempfile.TemporaryDirectory() as tmpdir:
                mtlx_dir = os.path.join(tmpdir, zip_folder)
                os.makedirs(mtlx_dir, exist_ok=True)
                mtlx_path = os.path.join(mtlx_dir, zip_folder + ".mtlx")

                # Determine the base directory for resolving relative texture paths
                # Use the directory of the source .mtlx file if available, else current working dir
                mtlx_source_dir = None
                if hasattr(doc, 'getSourceUri'):
                    source_uri = doc.getSourceUri()
                    if source_uri and os.path.isfile(source_uri):
                        mtlx_source_dir = os.path.dirname(os.path.abspath(source_uri))
                if not mtlx_source_dir:
                    mtlx_source_dir = os.getcwd()

                with zipfile.ZipFile(path, 'w') as z:

                    # Save all texture files under 'textures/'
                    texture_file_list = resolve_all_image_paths(doc)
                    for element_path, texture in texture_file_list.items():
                        # If texture path is not absolute, resolve it relative to the document's path
                        abs_texture = texture
                        if not os.path.isabs(texture):
                            logger.info(f"Texture path is relative: {texture}, resolving against {mtlx_source_dir}")
                            abs_texture = os.path.normpath(os.path.join(mtlx_source_dir, texture))
                        if os.path.isfile(abs_texture):
                            arcname = os.path.join("textures", os.path.basename(texture))
                            logger.info(f"Adding texture to ZIP: {abs_texture} as {arcname}")
                            z.write(abs_texture, arcname=arcname)

                            # Replace the references in the materialx
                            logger.info(f"Updating texture path on element {element_path} from {texture} to {arcname}")
                            doc.getDescendant(element_path).setValueString(arcname)
                        else:
                            logger.warning(f"Texture file not found: {abs_texture}")

                    mx.writeToXmlFile(doc, mtlx_path)
                    logger.info(f"Write MaterialX document to temp file: {mtlx_path}")
                    # Add the .mtlx file under the zip_folder path in the zip
                    z.write(mtlx_path, arcname=mtlx_name)
                    logger.info(f"Added MaterialX document to ZIP as: {mtlx_name}")

                    logger.info(f"MaterialX document and textures saved to ZIP: {path}")
        return True

# -------------------------------
# Existing test main logic preserved
# -------------------------------
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="MaterialX Zip Plugin Test")
    parser.add_argument("--zip-file", type=str, help="Path to the ZIP file to test")
    parser.add_argument("--mtlx-file", type=str, help="Path to the MaterialX file to test")
    args = parser.parse_args()

    zip_file = args.zip_file
    mtlx_file = args.mtlx_file

    # Access plugin manager
    manager = mx_render.getPluginManager()

    try:
        zipLoader = ZipLoader()
    except TypeError as e:
        raise RuntimeError(f"ZipLoader does not implement all required abstract methods: {e}")
    logger.info(f"Loader name: {zipLoader.name()}")
    manager.registerPlugin(zipLoader)
    logger.info(f"Registered plugins: {manager.getPluginList()}")

    test_loader = zip_file is not None and os.path.isfile(zip_file)
    if test_loader:
        # Get ZipLoader plugin by name and run it
        loader = manager.getLoader("ZipLoader")
        if loader:
            logger.info(f"ZipLoader plugin found, running it...")
            # Call .name() to ensure Python override is visible to C++
            doc = loader.run(zip_file)
            if doc:
                logger.info(f"Document loaded: {doc.getSourceUri()}")
                resolved_paths = resolve_all_image_paths(doc)
                for path, resolved_path in resolved_paths.items():
                    logger.info(f" - {path} resolved to {resolved_path}")
                #logger.info(mx.prettyPrint(doc)[:200])
            else:
                logger.error("Failed to load document.")

    test_saver = mtlx_file is not None and os.path.isfile(mtlx_file)
    if test_saver:
        saver = ZipSaver()
        doc = mx.createDocument()
        mx.readFromXmlFile(doc, mtlx_file)
        saver.run(doc, "new_zip.zip")

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
