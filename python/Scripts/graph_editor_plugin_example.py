"""
Example of how to start the plugin manager with GraphEditor integration.

This example shows how to:
1. Initialize the Python plugin manager with plugins
2. Discover and register plugins (including materialxjson plugin)
3. Set up the GraphEditor to use the same plugin manager
4. Coordinate between Python and C++ plugin systems
5. Demonstrate usage of the materialxjson plugin for JSON import/export
"""

from pathlib import Path
import sys
import os

import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('PMTest')
logger.setLevel(logging.INFO)


try:
    import MaterialX as mx
    from plugin_manager import get_plugin_manager
except ImportError as e:
    logger.info(f"Error importing MaterialX: {e}")
    logger.info("Make sure MaterialX Python bindings are installed and in your PYTHONPATH")
    sys.exit(1)

def initialize_plugin_system():
    """
    Initialize the plugin system and discover plugins.
    """

    logger.info("Initializing MaterialX Plugin System...")
    
    # Get the global plugin manager instance (this creates the manager)
    pm = get_plugin_manager()
    logger.info(f"Plugin manager created: {type(pm).__name__}")
    
    # Load plugins - the plugin manager handles all the discovery logic
    if pm:
        load_plugins(pm)
    
    # The plugin manager automatically coordinates with the C++ system
    # through its built-in callback integration
    logger.info(f"Plugin system initialization complete : {pm != None}")
    
    return pm


def load_plugins(pm):
    """Load plugins using the plugin manager's built-in discovery methods."""
    
    # 1. Load plugins from environment variable (MATERIALX_PLUGIN_PATHS)
    # The plugin manager automatically handles path parsing, multiple directories,
    # and proper error handling - no need to duplicate that logic here
    pm.load_plugins_from_environment_variable()
    
    # 2. Load plugins from default plugin directory (if it exists)
    default_plugin_dir = Path(__file__).parent / "plugins/materialxjson"
    dir_list = [default_plugin_dir, Path(__file__).parent / "plugins/ShadingLanguageX"]
    for dir_path in dir_list:
        if dir_path.exists():
            logger.info(f"Loading plugins from directory: {dir_path}")
            pm.load_plugins_from_dir(str(dir_path))
        else:
            logger.info(f"Plugin directory {dir_path} does not exist - skipping")
    
    return pm


def test_plugin_system():
    """Test the plugin system integration."""
    logger.info("\n" + "="*40)
    logger.info("Testing Plugin System")
    logger.info("="*50)
    
    pm = get_plugin_manager()
    
    # Test hook calls to see what plugins are available
    logger.info("Testing plugin hooks...")
    
    try:
        # Test supportedExtensions hook
        extensions_results = pm.hook.supportedExtensions()
        logger.info(f"Supported extensions from plugins: {extensions_results}")
        
        # Test getUIName hook
        names_results = pm.hook.getUIName()
        logger.info(f"Plugin UI names: {names_results}")
        
        # Test capability hooks
        import_results = pm.hook.canImport()
        export_results = pm.hook.canExport()
        logger.info(f"Import capabilities: {import_results}")
        logger.info(f"Export capabilities: {export_results}")
        
    except Exception as e:
        logger.info(f"Hook testing failed: {e}")
    
def create_test_document():
    doc = mx.createDocument()
    mx.readFromXmlFile(doc, "./brick.mtlx")
    return doc    

def test_slx_plugin(slx_plugin_instance):
    """Test document import/export through the specific SLX plugin instance."""
    logger.info("Testing SLX plugin operations...")
    
    # Create a test document
    doc = create_test_document()
    logger.info(f"- Loaded test doc")
    
    # Test export using the specific SLX plugin
    test_file = "test_graph_editor_export.mxsl"

    try:
        logger.info(f"* Testing export to: {test_file} using plugin instance")
        
        # Call the plugin's export method directly
        if hasattr(slx_plugin_instance, 'exportDocument'):
            success = slx_plugin_instance.exportDocument(doc, test_file)
        else:
            logger.info(f"Plugin doesn't have exportDocument method")
            return
        
        if success:
            logger.info("- Export successful")
            
            # Test import using the specific SLX plugin
            logger.info(f"* Testing import from: {test_file} using plugin instance")
            
            if hasattr(slx_plugin_instance, 'importDocument'):
                imported_doc = slx_plugin_instance.importDocument(test_file)
            else:
                logger.info(f"Plugin doesn't have importDocument method")
                return
            
            if imported_doc:
                logger.info("- Import successful")
                logger.info(f"  Imported document has {len(imported_doc.getNodeGraphs())} node graphs")
                
                # Clean up
                try:
                    #os.remove(test_file)
                    logger.info("- Cleaned up test file")
                except:
                    pass
            else:
                logger.info("- Import failed")
        else:
            logger.info("- Export failed")
            
    except Exception as e:
        logger.info(f"Document operation test failed: {e}")

def test_json_plugin(json_plugin_instance):
    """Test document import/export through the specific JSON plugin instance."""
    logger.info("Testing JSON plugin operations...")
    
    # Create a test document
    doc = create_test_document()
    logger.info(f"- Loaded test doc")
    
    # Test export using the specific JSON plugin
    test_file = "test_graph_editor_export.json"
    
    try:
        logger.info(f"* Testing export to: {test_file} using plugin instance")
        
        # Call the plugin's export method directly
        if hasattr(json_plugin_instance, 'exportDocument'):
            success = json_plugin_instance.exportDocument(doc, test_file)
        else:
            logger.info(f"Plugin doesn't have exportDocument method")
            return
        
        if success:
            logger.info("- Export successful")
            
            # Test import using the specific JSON plugin
            logger.info(f"* Testing import from: {test_file} using plugin instance")
            
            if hasattr(json_plugin_instance, 'importDocument'):
                imported_doc = json_plugin_instance.importDocument(test_file)
            else:
                logger.info(f"Plugin doesn't have importDocument method")
                return
            
            if imported_doc:
                logger.info("- Import successful")
                logger.info(f"  Imported document has {len(imported_doc.getNodeGraphs())} node graphs")
                
                # Clean up
                try:
                    #os.remove(test_file)
                    logger.info("- Cleaned up test file")
                except:
                    pass
            else:
                logger.info("- Import failed")
        else:
            logger.info("- Export failed")
            
    except Exception as e:
        logger.info(f"Document operation test failed: {e}")

def check_plugin_registration(pm):
    """Check what plugins were successfully registered and their capabilities."""
    logger.info("="*40)
    logger.info("Plugin Registration Status")
    logger.info("="*40)
    
    plugin_info = pm.get_plugin_info()
    
    logger.info(f"Total registered plugins: {plugin_info['count']}")
    logger.info(f"Supported extensions: {plugin_info['extensions']}")
    json_supported = '.json' in plugin_info['extensions']
    mxsl_supported = '.mxsl' in plugin_info['extensions']
    # Check if JSON support is available
    logger.info(f"JSON support: {'Available' if json_supported else 'Not available'}")
    logger.info(f"SLX support: {'Available' if mxsl_supported else 'Not available'}")
    
    json_plugin_instance = None
    mxsl_plugin_instance = None
    
    # Find actual plugin instances from the registered plugins
    if hasattr(pm, '_name2plugin'):
        for plugin_name, plugin_instance in pm._name2plugin.items():
            # Check if this plugin supports JSON
            try:
                if hasattr(plugin_instance, 'supportedExtensions'):
                    extensions = plugin_instance.supportedExtensions()
                    if '.json' in extensions:
                        json_plugin_instance = plugin_instance
                        logger.info(f"Found JSON plugin instance: {plugin_name}")
                    elif '.mxsl' in extensions:
                        mxsl_plugin_instance = plugin_instance
                        logger.info(f"Found MXSL plugin instance: {plugin_name}")
            except Exception as e:
                logger.info(f"Error checking plugin {plugin_name}: {e}")
    
    if plugin_info['plugins']:
        for plugin in plugin_info['plugins']:
            logger.info(f"  - {plugin['name']}")
            logger.info(f"    Extensions: {plugin['extensions']}")
            logger.info(f"    Import: {'Yes' if plugin['can_import'] else 'No'}")
            logger.info(f"    Export: {'Yes' if plugin['can_export'] else 'No'}")
    else:
        logger.info("No plugins registered")
        
    if json_plugin_instance:
        test_json_plugin(json_plugin_instance)
    else:
        logger.info("No JSON plugin instance found for testing")
        
    if mxsl_plugin_instance:
        print("*"*80)
        test_slx_plugin(mxsl_plugin_instance)
    else:
        logger.info("No MXSL plugin instance found for testing")

    return json_supported


def main():
    """Main function to start the plugin system and prepare for GraphEditor."""
    
    logger.info("="*70)
    logger.info("MaterialX Plugin Manager Test")
    logger.info("="*70)
    
    pm = initialize_plugin_system()
    json_supported = check_plugin_registration(pm)

    return pm


if __name__ == "__main__":
    plugin_manager = main()
    
    logger.info("Plugin manager is active and ready.")
    
    # Uncomment to keep the program running if needed for testing
    # try:
    #     import time
    #     logger.info("Keeping plugin manager alive... (Ctrl+C to exit)")
    #     while True:
    #         time.sleep(1)
    # except KeyboardInterrupt:
    #     logger.info("Shutting down plugin manager...")
