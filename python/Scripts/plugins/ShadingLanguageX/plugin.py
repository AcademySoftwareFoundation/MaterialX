"""
MaterialX ShadingLanguageX Plugin

This plugin provides import/export capabilities for MaterialX documents to/from
ShadingLanguageX (.mxsl) format using the mxslc compiler and ShadingLanguageX Python package.

This file serves as both the plugin discovery file and implementation.

The plugin must define the following functions/classes:
1. plugin_name() function - returns the plugin name
2. plugin_type() function - returns the plugin type (DOCUMENT_LOADER)
3. is_valid() function (optional) - returns True if plugin can be loaded
4. The actual plugin implementation class with hook implementations
"""

import logging
import os
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('MXsl')
logger.setLevel(logging.INFO)

# Import the plugin system components
try:
    import plugin_manager as pm
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    
    # Try to import ShadingLanguageX package components
    try:
        from mxslc.Decompiler.decompile import Decompiler, decompile_string
        DECOMPILER_AVAILABLE = True
        logger.info("mxslc Decompiler is available")
    except ImportError:
        DECOMPILER_AVAILABLE = False
        logger.info("mxslc Decompiler not available")
    
    try:
        from mxslc.compile_file import compile_file        
        COMPILER_AVAILABLE = True
        logger.info("mxslc Compiler is available")
    except ImportError:
        COMPILER_AVAILABLE = False
        logger.info("mxslc Compiler not available")
    
    MXSLC_AVAILABLE = DECOMPILER_AVAILABLE or COMPILER_AVAILABLE
    
except ImportError as e:
    MXSLC_AVAILABLE = False
    DECOMPILER_AVAILABLE = False
    COMPILER_AVAILABLE = False
    logger.info(f"Dependent libraries failed to load: {e}")

# Required plugin functions
#
def plugin_name():
    """Return the plugin name (required by plugin system)."""
    return "materialx_mxsl_plugin"

def plugin_type():
    """Return the plugin type (required by plugin system)."""
    return pm.MaterialXPluginManager.PluginType.DOCUMENT_LOADER

def is_valid():
    """Check if plugin dependencies are available (optional)."""
    try:
        return MXSLC_AVAILABLE
    except Exception as e:
        logger.info(f"MXSL Plugin validation failed: {e}")
        return False

# Required class with hook implementations
class MXSLDocumentPlugin:
    """
    Plugin that handles MXSL import/export for MaterialX documents using ShadingLanguageX.
    """    
    def __init__(self):
        """Initialize the plugin with ShadingLanguageX handler."""
        self._decompiler_available = DECOMPILER_AVAILABLE
        self._compiler_available = COMPILER_AVAILABLE

    @pm.hookimpl
    def supportedExtensions(self):
        """Hook implementation for supported extensions."""
        return [".mxsl"]
    
    @pm.hookimpl
    def getUIName(self):
        """Hook implementation for UI name."""
        return "MaterialX ShadingLanguageX Plugin (mxsl)"
    
    @pm.hookimpl
    def canImport(self):
        """Hook implementation for import capability."""
        return self._compiler_available
    
    @pm.hookimpl
    def canExport(self):
        """Hook implementation for export capability."""
        return self._decompiler_available
    
    @pm.hookimpl
    def importDocument(self, uri: str) -> mx.Document:
        """Hook implementation for document import."""
        return self.import_mxsl_document(uri)
    
    @pm.hookimpl
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Hook implementation for document export."""
        return self.export_mxsl_document(document, uri)
    
    #
    # Supporting methods for import/export using ShadingLanguageX
    def import_mxsl_document(self, uri: str) -> mx.Document:
        """
        Import a MaterialX document from MXSL format using ShadingLanguageX.
        @param uri: Path to the MXSL file to import
        @returns: MaterialX document on success, None on failure
        """
        logger.info(f"Importing MXSL document from {uri}")
        
        if not self._compiler_available:
            logger.info("Cannot import - mxslc Compiler not available")
            return None
        
        try:
            # Check if the MXSL file exists
            if not os.path.exists(uri):
                logger.info(f"MXSL file not found: {uri}")
                return None
            
            # Read the MXSL file
            #with open(uri, 'r', encoding='utf-8') as f:
            #    mxsl_content = f.read()
            
            # Use the compiler to convert MXSL to MaterialX XML string
            mtlx_path = os.path.splitext(uri)[0] + ".mtlx"
            compile_file(uri, mtlx_path)
            
            if os.path.exists(mtlx_path):            
                # Create MaterialX document from the XML string
                doc = mx.createDocument()
                mx.readFromXmlFile(doc, mtlx_path)
            
            logger.info(f"Successfully imported MXSL document from {uri}")
            logger.info(f"Document contains {len(doc.getChildren())} top-level elements")
            return doc
                
        except Exception as e:            
            logger.info(f"Error importing MXSL {uri}: {e}")
            return None
    
    def export_mxsl_document(self, document: mx.Document, uri: str) -> bool:
        """
        Export a MaterialX document to MXSL format using ShadingLanguageX.
        @param document: MaterialX document to export
        @param uri: Path to the MXSL file to write
        @returns: True on success, False on failure
        """
        logger.info(f"Exporting MaterialX document to MXSL: {uri}")
        
        if not self._decompiler_available:
            logger.info("Cannot export - mxslc Decompiler not available")
            return False
        
        try:
            # Convert MaterialX document to XML string
            mtlx_content = mx.writeToXmlString(document)
            
            # Use the decompiler to convert MaterialX XML to MXSL
            from mxslc.Decompiler.decompile import decompile_string, Decompiler
            
            # Try decompile_string first, fallback to Decompiler class
            try:
                mxsl_content = decompile_string(mtlx_content)
            except:
                decompiler = Decompiler(mtlx_content)
                mxsl_content = decompiler.decompile()
            
            if not mxsl_content:
                logger.info(f"Failed to decompile MaterialX to MXSL")
                return False
            
            # Write MXSL content to file
            with open(uri, 'w', encoding='utf-8') as f:
                f.write(mxsl_content)
            
            logger.info(f"Successfully exported MaterialX document to MXSL: {uri}")
            return True
                
        except Exception as e:
            logger.info(f"Error exporting to MXSL {uri}: {e}")
            return False

if __name__ != "__main__":
    pass
else:
    logger.info("Plugin is not intended to be run as a standalone script")
