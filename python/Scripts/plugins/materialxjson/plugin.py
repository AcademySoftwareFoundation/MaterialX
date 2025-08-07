"""
MaterialX JSON Plugin using materialxjson library

This plugin demonstrates how to create document loaders for custom formats
using the materialxjson library (https://github.com/kwokcb/materialxjson).
It provides JSON import/export capabilities for MaterialX documents.

This file serves as both the plugin discovery file and implementation.

The plugin must define the following functions/classes:
1. plugin_name() function - returns the plugin name
2. plugin_type() function - returns the plugin type (DOCUMENT_LOADER)
3. is_valid() function (optional) - returns True if plugin can be loaded
4. The actual plugin implementation class with hook implementations
"""

import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('MXjson')
logger.setLevel(logging.INFO)

# Import the plugin system components
try:
    import json as JSON
    import plugin_manager as pm
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from materialxjson import core as materialxjson_core
    MATERIALXJSON_AVAILABLE = True
    logger.info("materialxjson library is available")
except ImportError:
    MATERIALXJSON_AVAILABLE = False
    logger.info(f"Dependent libraries failed to load: {e}")

# Required plugin functions
#
def plugin_name():
    """Return the plugin name (required by plugin system)."""
    return "materialx_json_plugin"

def plugin_type():
    """Return the plugin type (required by plugin system)."""
    return pm.MaterialXPluginManager.PluginType.DOCUMENT_LOADER

def is_valid():
    """Check if plugin dependencies are available (optional)."""
    try:
        return True
    except ImportError as e:
        logger.info(f"JSON Plugin validation failed: {e}")
        return False

# Require class with hook implementations
class JSONDocumentPlugin:
    """
    Plugin that handles JSON import/export for MaterialX documents using materialxjson library.
    """
    
    def __init__(self):
        """Initialize the plugin with materialxjson handler."""
        if MATERIALXJSON_AVAILABLE:
            self._mtlxjson = materialxjson_core.MaterialXJson()
        else:
            self._mtlxjson = None    

    @pm.hookimpl
    def supportedExtensions(self):
        """Hook implementation for supported extensions."""
        return [".json"]
    
    @pm.hookimpl
    def getUIName(self):
        """Hook implementation for UI name."""
        return "MaterialX JSON Plugin (materialxjson)"
    
    @pm.hookimpl
    def canImport(self):
        """Hook implementation for import capability."""
        return MATERIALXJSON_AVAILABLE
    
    @pm.hookimpl
    def canExport(self):
        """Hook implementation for export capability."""
        return MATERIALXJSON_AVAILABLE
    
    @pm.hookimpl
    def importDocument(self, uri: str) -> mx.Document:
        """Hook implementation for document import."""
        return self.import_json_document(uri)
    
    @pm.hookimpl
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Hook implementation for document export."""
        return self.export_json_document(document, uri)
    
    #
    # Supporting methods for import/export using materialxjson
    #
    def import_json_document(self, uri: str) -> mx.Document:
        """
        Import a MaterialX document from JSON format using materialxjson library.
        @parms uri: Path to the JSON file to import
        @returns: MaterialX document on success, None on failure
        """
        logger.info(f"Importing document from {uri}")
        
        if not MATERIALXJSON_AVAILABLE or not self._mtlxjson:
            logger.info("Cannot import - materialxjson library not available")
            return None
        
        try:
            # Read JSON file using materialxjson Util
            json_object = materialxjson_core.Util.readJson(uri)
            if not json_object:
                logger.info(f"Failed to read JSON file {uri}")
                return None
            
            # Create a new MaterialX document
            doc = mx.createDocument()
            
            # Convert JSON to MaterialX document using materialxjson
            success = self._mtlxjson.documentFromJSON(json_object, doc)
            
            if success:
                #print(mx.prettyPrint(doc))
                doc.setSourceUri(uri)
                logger.info(f"Successfully imported document from {uri}")
                logger.info(f"Document contains {len(doc.getChildren())} top-level elements")
                return doc
            else:
                logger.info(f"Failed to convert JSON to MaterialX document: {uri}")
                return None
                
        except Exception as e:
            logger.info(f"Error importing {uri}: {e}")
            return None
    
    def export_json_document(self, document: mx.Document, uri: str) -> bool:
        """
        Export a MaterialX document to JSON format using materialxjson library.
        @param document: MaterialX document to export
        @param uri: Path to the JSON file to write
        @returns: True on success, False on failure
        """
        logger.info(f"Exporting document to {uri}")
        
        if not MATERIALXJSON_AVAILABLE or not self._mtlxjson:
            logger.info("Cannot export - materialxjson library not available")
            return False
        
        try:
            # Convert MaterialX document to JSON using materialxjson
            json_object = self._mtlxjson.documentToJSON(document)
            
            if json_object:
                # Write JSON to file using materialxjson Util
                #json_string = JSON.dumps(json_object, indent=2)
                #print(json_string)
                materialxjson_core.Util.writeJson(json_object, uri, indentation=2)                
                logger.info(f"Successfully exported document to {uri}")
                return True
            else:
                logger.info(f"Failed to convert MaterialX document to JSON")
                return False
                
        except Exception as e:
            logger.info(f"Error exporting to {uri}: {e}")
            return False

if __name__ != "__main__":
    pass
else:
    logger.info("Plugin is not intended to be run as a standalone script")
