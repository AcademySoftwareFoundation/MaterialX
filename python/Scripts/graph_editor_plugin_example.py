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
    if default_plugin_dir.exists():
        logger.info(f"Loading plugins from folder: {default_plugin_dir}")
        pm.load_plugins_from_dir(str(default_plugin_dir))
    else:
        logger.info(f"Default plugin directory {default_plugin_dir} does not exist - skipping")
    
    return pm


def setup_graph_editor_integration():
    """Set up GraphEditor to use the same plugin manager."""
    
    # The GraphEditor PluginIntegration class will automatically use
    # the same C++ PluginManager singleton that our Python plugin manager
    # is connected to through the callback integration system
    
    #logger.info("GraphEditor will automatically use the shared C++ PluginManager singleton")
    #logger.info("All document loaders registered through Python hooks will be available to GraphEditor")
    pass

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
    
    # Test document operations using the system
    #test_document_operations(pm)


def test_document_operations(pm):
    """Test document import/export through the plugin system."""
    logger.info("Testing document operations...")
    
    # Create a test document
    doc = mx.createDocument()
    doc.setVersionString("1.38")
    doc.setColorSpace("lin_rec709")
    
    # Add some content to make the test more meaningful
    nodegraph = doc.addNodeGraph("test_graph")
    constant_node = nodegraph.addNode("constant", "test_constant")
    constant_node.setType("color3") 
    constant_node.setInputValue("value", mx.Color3(0.8, 0.4, 0.2))
    
    logger.info(f"- Created test document with {len(doc.getNodeGraphs())} node graphs")
    
    # Test export via hooks (if JSON plugin is available)
    test_file = "test_graph_editor_export.json"
    
    try:
        logger.info(f"* Testing export to: {test_file}")
        success = pm.export_document_via_hooks(doc, test_file)
        
        if success:
            logger.info("- Export successful")
            
            # Test import
            logger.info(f"* Testing import from: {test_file}")
            imported_doc = pm.import_document(test_file)
            
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
        import traceback
        traceback.print_exc()


def check_plugin_registration(pm):
    """Check what plugins were successfully registered and their capabilities."""
    logger.info("="*40)
    logger.info("Plugin Registration Status")
    logger.info("="*40)
    
    plugin_info = pm.get_plugin_info()
    
    logger.info(f"Total registered plugins: {plugin_info['count']}")
    logger.info(f"Supported extensions: {plugin_info['extensions']}")
    json_supported = '.json' in plugin_info['extensions']
    # Check if JSON support is available
    logger.info(f"JSON support: {'Available' if json_supported else 'Not available'}")
    
    if plugin_info['plugins']:
        #logger.info("Registered plugins:")
        for plugin in plugin_info['plugins']:
            if '.json' in plugin['extensions']:
                logger.info(f"  - {plugin['name']} supports JSON")
                logger.info(f"    Extensions: {plugin['extensions']}")
                logger.info(f"    Import: {'Yes' if plugin['can_import'] else 'No'}")
                logger.info(f"    Export: {'Yes' if plugin['can_export'] else 'No'}")
    else:
        logger.info("No plugins registered")
        
    return json_supported


def main():
    """Main function to start the plugin system and prepare for GraphEditor."""
    
    logger.info("="*70)
    logger.info("MaterialX Plugin Manager + GraphEditor Integration")
    logger.info("="*70)
    
    # 1. Initialize the plugin system
    pm = initialize_plugin_system()
    
    # 2. Check what plugins were successfully registered
    json_supported = check_plugin_registration(pm)
    
    # 3. Set up GraphEditor integration
    setup_graph_editor_integration()
    
    # 4. Test the plugin system
    #test_plugin_system()
    test_document_operations(pm)

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
