import logging
import os
import argparse
#import pathlib

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('ZipPlugin')

have_zip = False
try:
    import zipfile
    import tempfile
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    logger.info("MaterialX and zip modules loaded successfully")
    have_zip = True
except ImportError:
    raise ImportError("Please ensure MaterialX and zipfile modules are installed.")

have_requests = False
try:
    import requests
    import io
    have_requests = True
    logger.info("Requests module loaded successfully")
except ImportError:
    logger.warning("Requests module not found")

class URILoader:
    def __init__(self):
        self.zip_type = (
            "application/zip",
            "application/x-zip-compressed",
            "multipart/x-zip",
            "application/octet-stream"
        )
        self.mtlx_type = (
            "application/xml"
        )
        self.content_type = None
        self.contents = None
        self.uri = None

    def content_is_zip(self):
        return self.content_type in self.zip_type

    def content_is_mtlx(self):
        return self.content_type in self.mtlx_type
    
    def fetch_from_http(self, uri):
        """
        Fetch a contents from a HTTP URI 
        """
        if not have_requests:
            logger.warning("Requests module not available")
            return False

        if not uri.startswith("http://") and not uri.startswith("https://"):
            logger.warning(f"Invalid URI scheme: {uri}")
            return False


        try:
            response = requests.get(uri, stream=True)
            response.raise_for_status()  

            # Check if have a zip file or xml file based on content type
            content_type = response.headers.get("Content-Type")
          
            if content_type in self.zip_type:
                logger.info(f"Fetched ZIP file from {uri}")
                self.content_type = 'application/zip'
                self.content =  io.BytesIO(response.content)
                self.uri = uri
            elif content_type in self.mtlx_type:
                logger.info(f"Fetched XML file from {uri}")
                self.content_type = 'application/xml'
                self.content =  io.BytesIO(response.content)
                self.uri = uri
            else:
                logger.warning(f"Unsupported content type: {response.headers.get('Content-Type')}")                
                return False

        except Exception as e:
            logger.error(f"Failed to fetch {uri}: {e}")
            return False

        return True

    def get_content(self):
        return self.content.getbuffer()
    
    def get_base_name(self):
        return os.path.basename(self.uri)

    def write_contents_to_file(self, path):
        if not self.content:
            logger.warning("No content to write")
            return False

        type_string = ""
        if self.content_type in self.zip_type:
            type_string = "ZIP"
        elif self.content_type in self.mtlx_type:
            type_string = "MTLX"

        logger.info(f"Fetched {type_string} file from HTTP URI: {self.uri}")
        # Save zip to zip file name specified in url
        file_name = self.get_base_name()
        logger.info(f"Save {type_string} file to: {file_name}")
        with open(file_name, "wb") as f:
            f.write(self.get_content())

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
        if have_zip:
            # Check if the path is an http uri:
            if path.startswith("http") or path.startswith("https"):
                logger.info(f"Detected HTTP URI")
                uri_loader = URILoader()
                sucesss = uri_loader.fetch_from_http(path)
                if sucesss and uri_loader.content_is_zip():
                    path = os.path.basename(path)
                    logger.info(f"Save ZIP file: {path}")
                    uri_loader.write_contents_to_file(path)
                    # Unpack if we downloaded a zip file

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
        if not path:
            return None

        # Determine the .mtlx filename based on the .zip filename
        zip_basename = os.path.basename(path)
        zip_folder = os.path.splitext(zip_basename)[0]
        mtlx_name = zip_folder + ".mtlx"  # .mtlx at root of zip
        logger.info(f"zip base name: {zip_basename}, mtlx name: {mtlx_name}")

        # Write the document to a temporary file in the temp directory (not in a subfolder)
        with tempfile.TemporaryDirectory() as tmpdir:
            mtlx_path = os.path.join(tmpdir, mtlx_name)

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
                # Add the .mtlx file at the root of the zip
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
    parser.add_argument("--uri", type=str, help="URI to the ZIP file")
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

    # Test if this is a http uri
    uri = args.uri
    #if not uri:
    #    uri = 'https://kwokcb.github.io/materialxMaterials/src/materialxMaterials/data/ambientCgMaterials/WoodFloor038_1K-PNG.zip'
    if uri is not None and (uri.startswith("http") or uri.startswith("https")):
        logger.info(f"Detected HTTP URI for ZIP file: {uri}")
        uri_loader = URILoader()
        sucesss = uri_loader.fetch_from_http(uri)
        if sucesss:
            filename = os.path.basename(uri)
            logger.info(f"Save ZIP file: {filename}")
            uri_loader.write_contents_to_file(filename)
            # Unpack if we downloaded a zip file
            if uri_loader.content_is_zip():
                zip_file = filename

    test_loader = zip_file is not None # and os.path.isfile(zip_file)
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
