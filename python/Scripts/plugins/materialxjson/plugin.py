"""
MaterialX JSON Plugin using materialxjson library

This plugin demonstrates how to create document loaders for custom formats
using the official materialxjson library (https://github.com/kwokcb/materialxjson).
It provides JSON import/export capabilities for MaterialX documents.

This file serves as both the plugin discovery file and implementation.
The plugin system will look for:
1. plugin_name() function - returns the plugin name
2. is_valid() function (optional) - returns True if plugin can be loaded
3. The actual plugin implementation (included in this file)
"""

import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
from typing import Optional

import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('MXjson')
logger.setLevel(logging.INFO)

# Import the plugin system components
try:
    import plugin_manager as pm
    #from plugin_manager import get_plugin_manager, hookimpl, create_document_loader, register_plugin_with_manager
except ImportError:
    # If running from a different location, try relative import
    import sys
    import os
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    import plugin_manager as pm
    #from plugin_manager import get_plugin_manager, hookimpl, create_document_loader, register_plugin_with_manager

# Import the materialxjson library
try:
    from materialxjson import core as materialxjson_core
    MATERIALXJSON_AVAILABLE = True
    logger.info("materialxjson library is available")
except ImportError as e:
    MATERIALXJSON_AVAILABLE = False
    logger.info(f"materialxjson library not available: {e}")
    logger.info("Install with: pip install materialxjson")


def plugin_name():
    """Return the plugin name (required by plugin system)."""
    return "materialx_json_plugin"


def is_valid():
    """Check if plugin dependencies are available (optional)."""
    try:
        import MaterialX as mx
        import MaterialX.PyMaterialXRender as mx_render
        from materialxjson import core as materialxjson_core
        return True
    except ImportError as e:
        logger.info(f"JSON Plugin validation failed: {e}")
        return False


class JSONDocumentPlugin:
    """
    Plugin that handles JSON import/export for MaterialX documents using materialxjson library.
    
    This demonstrates the plugin pattern using the pluggy framework with hookspec/hookimpl
    and the official materialxjson library for robust JSON conversion.
    """
    
    def __init__(self):
        """Initialize the plugin with materialxjson handler."""
        if MATERIALXJSON_AVAILABLE:
            self._mtlxjson = materialxjson_core.MaterialXJson()
        else:
            self._mtlxjson = None
    
    @pm.hookimpl
    def register_document_loaders(self):
        """Hook implementation to register our document loaders."""
        
        if not MATERIALXJSON_AVAILABLE:
            logger.info("Cannot register loader - materialxjson library not available")
            return []
        
        # Create JSON import / export loader using the new hook-based approach
        json_loader = pm.create_document_loader(
            identifier="materialxjson_loader",
            name="MaterialX JSON Document Loader", 
            description="Import / Export MaterialX documents from JSON format using materialxjson library",
            plugin_manager = pm.get_plugin_manager(),
            plugin_instance=self
        )
        
        logger.info("Registered MaterialX JSON document loader")
        return [json_loader]
    
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
    def importDocument(self, uri: str) -> Optional[mx.Document]:
        """Hook implementation for document import."""
        return self.import_json_document(uri)
    
    @pm.hookimpl
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Hook implementation for document export."""
        return self.export_json_document(document, uri)
    
    def import_json_document(self, uri: str) -> Optional[mx.Document]:
        """
        Import a MaterialX document from JSON format using materialxjson library.
        
        Args:
            uri: Path to the JSON file to import
            
        Returns:
            MaterialX Document object or None on failure
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
                doc.setSourceUri(uri)
                logger.info(f"Successfully imported document from {uri}")
                logger.info(f"Document contains {len(doc.getChildren())} top-level elements")
                return doc
            else:
                logger.info(f"Failed to convert JSON to MaterialX document: {uri}")
                return None
                
        except Exception as e:
            logger.info(f"Error importing {uri}: {e}")
            import traceback
            traceback.print_exc()
            return None
    
    def export_json_document(self, document: mx.Document, uri: str) -> bool:
        """
        Export a MaterialX document to JSON format using materialxjson library.
        
        Args:
            document: The MaterialX document to export
            uri: Path where to save the JSON file
            
        Returns:
            True on success, False on failure
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
                materialxjson_core.Util.writeJson(json_object, uri, indentation=2)
                
                logger.info(f"Successfully exported document to {uri}")
                return True
            else:
                logger.info(f"Failed to convert MaterialX document to JSON")
                return False
                
        except Exception as e:
            logger.info(f"Error exporting to {uri}: {e}")
            import traceback
            traceback.print_exc()
            return False


# Plugin registration function (called by plugin system)
def register_plugin():
    """
    Register this plugin with the MaterialX plugin system.
    
    This function is called automatically when the plugin is discovered.
    """
    logger.info("Registering plugin...")
    
    if not MATERIALXJSON_AVAILABLE:
        logger.info("Plugin registration skipped - materialxjson library not available")
        logger.info("Install with: pip install materialxjson")
        return
    
    # Create plugin instance
    plugin = JSONDocumentPlugin()
    
    # Register with plugin manager using the new interface
    pm.register_plugin_with_manager(plugin)
    
    logger.info("Registration complete")


def initialize_plugin():
    """Initialize the plugin."""
    try:
        register_plugin()
        if MATERIALXJSON_AVAILABLE:
            logger.info("JSON Plugin (materialxjson) initialized successfully")
        else:
            logger.info("JSON Plugin initialization skipped - missing materialxjson library")
    except Exception as e:
        logger.info(f"Error initializing JSON plugin: {e}")
        raise


def test_json_plugin():
    """Test the JSON plugin functionality using materialxjson library."""
    
    if not MATERIALXJSON_AVAILABLE:
        logger.info("Test skipped - materialxjson library not available")
        return
    
    logger.info("Testing MaterialX JSON Plugin with materialxjson library...")
    
    # Create a test document
    doc = mx.createDocument()
    doc.setVersionString("1.38")
    doc.setColorSpace("lin_rec709")
    
    # Add a simple node graph
    nodegraph = doc.addNodeGraph("test_graph")
    node = nodegraph.addNode("constant", "test_node")
    node.setType("color3")
    node.setInputValue("value", mx.Color3(1.0, 0.5, 0.0))
    
    # Add a material
    material = doc.addMaterial("test_material")
    shader_ref = material.addShaderRef("surface_shader", "test_graph")
    
    logger.info(f"Created test document with:")
    logger.info(f"  - {len(doc.getNodeGraphs())} node graphs")
    logger.info(f"  - {len(doc.getMaterials())} materials")
    logger.info(f"  - Version: {doc.getVersionString()}")
    logger.info(f"  - Color space: {doc.getColorSpace()}")
    
    # Test export
    plugin = JSONDocumentPlugin()
    test_file = "test_materialxjson_export.json"
    
    success = plugin.export_json_document(doc, test_file)
    logger.info(f"Export test: {'SUCCESS' if success else 'FAILED'}")
    
    if success:
        # Test import
        imported_doc = plugin.import_json_document(test_file)
        logger.info(f"Import test: {'SUCCESS' if imported_doc else 'FAILED'}")
        
        if imported_doc:
            logger.info(f"Imported document contains:")
            logger.info(f"  - {len(imported_doc.getNodeGraphs())} node graphs")  
            logger.info(f"  - {len(imported_doc.getMaterials())} materials")
            logger.info(f"  - Version: {imported_doc.getVersionString()}")
            logger.info(f"  - Color space: {imported_doc.getColorSpace()}")
        
        # Clean up test file
        try:
            import os
            os.remove(test_file)
            logger.info(f"Cleaned up test file: {test_file}")
        except:
            pass


# Auto-initialize when discovered by plugin system
if __name__ != "__main__":
    # When imported by plugin system, auto-initialize
    initialize_plugin()
else:
    # If run directly, run tests
    logger.info("Running MaterialX JSON Plugin tests...")
    test_json_plugin()
