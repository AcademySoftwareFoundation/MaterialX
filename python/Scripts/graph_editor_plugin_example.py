"""
Example of how to start the plugin manager with GraphEditor integration.

This example shows how to:
1. Initialize the Python plugin manager with hook-based plugins
2. Discover and register plugins (including materialxjson plugin)
3. Set up the GraphEditor to use the same plugin manager
4. Coordinate between Python and C++ plugin systems
5. Demonstrate usage of the materialxjson plugin for JSON import/export

Updated for the new hook-based plugin system with materialxjson integration.
The old loader-based system has been replaced with the modern hookspec/hookimpl approach.
"""

from pathlib import Path
import sys
import os
try:
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from plugin_manager import get_plugin_manager
except ImportError as e:
    print(f"Error importing MaterialX: {e}")
    print("Make sure MaterialX Python bindings are installed and in your PYTHONPATH")
    sys.exit(1)

def initialize_plugin_system():
    """Initialize the hook-based plugin system and discover plugins."""
    print("Initializing MaterialX Hook-based Plugin System...")
    
    # Get the global plugin manager instance (this creates the hook-based manager)
    pm = get_plugin_manager()
    print(f"Plugin manager created: {type(pm).__name__}")
    
    # Discover and load plugins from environment variable and standard locations
    discover_and_load_plugins(pm)
    
    # The plugin manager automatically coordinates with the C++ system
    # through its built-in callback integration
    print("Plugin system initialization complete")
    
    return pm


def discover_and_load_plugins(pm):
    """Discover and load plugins from various locations."""
    
    # 1. Load plugins from environment variable
    pm.load_plugins_from_environment_variable()
    
    # 2. Load plugins from standard plugin directories
    plugin_dirs = []
    
    # Check for environment variable with plugin paths
    envvar_plugin_paths = "MATERIALX_PLUGIN_PATHS"
    if envvar_plugin_paths in os.environ:
        paths = os.environ[envvar_plugin_paths]
        # Split on ; (Windows) or : (Unix)
        separator = ';' if os.name == 'nt' else ':'
        plugin_dirs.extend([Path(d.strip()) for d in paths.split(separator) if d.strip()])
        print(f"Using plugin directories from environment variable: {envvar_plugin_paths}")
    
    # Add default plugin directory
    default_plugin_dir = Path(__file__).parent / "plugins"
    plugin_dirs.append(default_plugin_dir)    # Discover plugins from each directory
    for plugin_dir in plugin_dirs:
        if plugin_dir.exists():
            print(f"Loading plugins from: {plugin_dir}")
            pm.load_plugins_from_dir(str(plugin_dir))
        else:
            print(f"Plugin directory {plugin_dir} does not exist - skipping")
    
    # Show discovered plugins
    print(f"Total registered plugins: {len(pm._registered_plugins)}")
    
    return pm


def setup_graph_editor_integration():
    """Set up GraphEditor to use the same plugin manager."""
    
    # The GraphEditor PluginIntegration class will automatically use
    # the same C++ PluginManager singleton that our Python plugin manager
    # is connected to through the callback integration system
    
    print("GraphEditor will automatically use the shared C++ PluginManager singleton")
    print("All document loaders registered through Python hooks will be available to GraphEditor")


def test_plugin_system():
    """Test the hook-based plugin system integration."""
    print("\n" + "="*50)
    print("Testing Hook-based Plugin System")
    print("="*50)
    
    pm = get_plugin_manager()
    
    # Test hook calls to see what plugins are available
    print("Testing plugin hooks...")
    
    try:
        # Test supportedExtensions hook
        extensions_results = pm.hook.supportedExtensions()
        print(f"Supported extensions from plugins: {extensions_results}")
        
        # Test getUIName hook
        names_results = pm.hook.getUIName()
        print(f"Plugin UI names: {names_results}")
        
        # Test capability hooks
        import_results = pm.hook.canImport()
        export_results = pm.hook.canExport()
        print(f"Import capabilities: {import_results}")
        print(f"Export capabilities: {export_results}")
        
    except Exception as e:
        print(f"Hook testing failed: {e}")
    
    # Test document operations using the hook-based system
    test_document_operations(pm)


def test_document_operations(pm):
    """Test document import/export through the hook-based plugin system."""
    print("\nTesting document operations...")
    
    # Create a test document
    doc = mx.createDocument()
    doc.setVersionString("1.38")
    doc.setColorSpace("lin_rec709")
    
    # Add some content to make the test more meaningful
    nodegraph = doc.addNodeGraph("test_graph")
    constant_node = nodegraph.addNode("constant", "test_constant")
    constant_node.setType("color3") 
    constant_node.setInputValue("value", mx.Color3(0.8, 0.4, 0.2))
    
    print(f"Created test document with {len(doc.getNodeGraphs())} node graphs")
    
    # Test export via hooks (if JSON plugin is available)
    test_file = "test_graph_editor_export.json"
    
    try:
        print(f"Testing export to: {test_file}")
        success = pm.export_document_via_hooks(doc, test_file)
        
        if success:
            print("✓ Export successful")
            
            # Test import
            print(f"Testing import from: {test_file}")
            imported_doc = pm.import_document_via_hooks(test_file)
            
            if imported_doc:
                print("✓ Import successful")
                print(f"  Imported document has {len(imported_doc.getNodeGraphs())} node graphs")
                
                # Clean up
                try:
                    os.remove(test_file)
                    print("✓ Cleaned up test file")
                except:
                    pass
            else:
                print("✗ Import failed")
        else:
            print("✗ Export failed")
            
    except Exception as e:
        print(f"Document operation test failed: {e}")
        import traceback
        traceback.print_exc()


def check_materialxjson_plugin():
    """Check if the materialxjson plugin is available and working."""
    print("\n" + "="*40)
    print("Checking MaterialXJSON Plugin")
    print("="*40)
    
    try:
        from materialxjson import core as materialxjson_core
        print("✓ materialxjson library is available")
        
        # Test basic functionality
        mtlxjson = materialxjson_core.MaterialXJson()
        doc = mx.createDocument()
        doc.setVersionString("1.38")
        
        json_obj = mtlxjson.documentToJSON(doc)
        if json_obj:
            print("✓ materialxjson conversion working")
            print(f"  JSON keys: {list(json_obj.keys())}")
        else:
            print("✗ materialxjson conversion failed")
            
    except ImportError as e:
        print(f"✗ materialxjson library not available: {e}")
        print("  Install with: pip install materialxjson")
        print("  The JSON plugin will not be functional without this library")
    except Exception as e:
        print(f"✗ Error testing materialxjson: {e}")


def main():
    """Main function to start the hook-based plugin system and prepare for GraphEditor."""
    
    print("="*70)
    print("MaterialX Hook-based Plugin Manager + GraphEditor Integration")
    print("="*70)
    
    # 1. Check if materialxjson plugin dependencies are available
    check_materialxjson_plugin()
    
    # 2. Initialize the hook-based plugin system
    pm = initialize_plugin_system()
    
    # 3. Set up GraphEditor integration
    setup_graph_editor_integration()
    
    # 4. Test the plugin system
    test_plugin_system()
    
    print("\n" + "="*50)
    print("Plugin System Ready for GraphEditor")
    print("="*50)
    print("The plugin manager is now active and ready for GraphEditor integration.")
    print("All discovered plugins are available for document import/export.")
    
    return pm


if __name__ == "__main__":
    plugin_manager = main()
    
    print("\nPlugin manager is active and ready.")
    print("You can now start GraphEditor or use the plugin system programmatically.")
    
    # Uncomment to keep the program running if needed for testing
    # try:
    #     import time
    #     print("Keeping plugin manager alive... (Ctrl+C to exit)")
    #     while True:
    #         time.sleep(1)
    # except KeyboardInterrupt:
    #     print("\nShutting down plugin manager...")
