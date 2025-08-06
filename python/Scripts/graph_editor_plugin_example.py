"""
Example of how to start the plugin manager with GraphEditor integration.

This example shows how to:
1. Initialize the Python plugin manager
2. Register document loaders
3. Set up the GraphEditor to use the same plugin manager
4. Coordinate between Python and C++ plugin systems
"""

from pathlib import Path
import sys
import os
try:
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from plugin_manager import get_plugin_manager, document_loader, create_document_loader
except ImportError as e:
    print(f"Error importing MaterialX: {e}")
    print("Make sure MaterialX Python bindings are installed and in your PYTHONPATH")
    sys.exit(1)

def initialize_plugin_system():
    """Initialize the plugin system and register default loaders."""
    print("Initializing MaterialX Plugin System...")
    
    # Get the global plugin manager instance
    pm = get_plugin_manager()
    
    # Register the predefined loaders
    #print("Registering predefined document loaders...")
    for config in _loader_configs:
        try:
            if config.get("can_import", False):
                loader = create_document_loader(
                    identifier=config["identifier"],
                    name=config["name"],
                    description=config["description"],
                    extensions=config["extensions"],
                    import_func=config["func"]
                )
            elif config.get("can_export", False):
                loader = create_document_loader(
                    identifier=config["identifier"],
                    name=config["name"],
                    description=config["description"],
                    extensions=config["extensions"],
                    export_func=config["func"]
                )
            else:
                continue
                  # Register with the C++ system
            try:
                result = mx_render.registerDocumentLoader(loader)
                if result:
                    print(f"- Successfully registered: {config['name']}")
                else:
                    print(f"- Failed to register: {config['name']} (returned False)")
            except Exception as e:
                print(f"- Error registering {config['name']}: {e}")
                print("  This might happen if MaterialX libraries are not fully built")
                print("  The script will continue...")
                
        except Exception as e:
            print(f"✗ Error creating loader for {config['name']}: {e}")
    
    # The plugin manager automatically sets up the C++ callback integration
    # when it's created, so both Python and C++ systems will be coordinated
    
    return pm


def setup_graph_editor_integration():
    """Set up GraphEditor to use the same plugin manager."""
    
    # The GraphEditor PluginIntegration class will automatically use
    # the same C++ PluginManager singleton that our Python plugin manager
    # is connected to through mx_render.registerDocumentLoader() calls
    
    print("GraphEditor will automatically use the shared C++ PluginManager singleton")
    print("All document loaders registered through Python will be available to GraphEditor")


# Document loaders that will be registered when initialize_plugin_system() is called
_pending_loaders = []

def json_loader_impl(uri: str) -> mx.Document:
    """Example JSON document loader."""
    print(f"Loading JSON document from: {uri}")
    # This is just an example - implement your actual JSON loading logic
    doc = mx.createDocument()
    doc.setSourceUri(uri)
    return doc

def yaml_loader_impl(uri: str) -> mx.Document:
    """Example YAML document loader."""
    print(f"Loading YAML document from: {uri}")
    # This is just an example - implement your actual YAML loading logic
    doc = mx.createDocument()
    doc.setSourceUri(uri)
    return doc

def custom_exporter_impl(document: mx.Document, uri: str) -> bool:
    """Example custom document exporter."""
    print(f"Exporting document to custom format: {uri}")
    # This is just an example - implement your actual export logic
    return True

# Define loader configurations (but don't register them yet)
_loader_configs = [
    {
        "identifier": "json_loader",
        "name": "MY JSON Document Loader", 
        "description": "Loads MaterialX documents from JSON format",
        "extensions": [".json"],
        "can_import": True,
        "func": json_loader_impl
    },
    {
        "identifier": "yaml_loader",
        "name": "MY YAML Document Loader",
        "description": "Loads MaterialX documents from YAML format", 
        "extensions": [".yaml", ".yml"],
        "can_import": True,
        "func": yaml_loader_impl
    },
    {
        "identifier": "custom_exporter",
        "name": "Custom Format Exporter",
        "description": "Exports documents to custom format",
        "extensions": [".custom"],
        "can_export": True,
        "func": custom_exporter_impl
    }
]

_loader_configs = []


def register_custom_loaders():
    """Register additional custom document loaders programmatically."""
    
    # Example of creating a loader programmatically (without decorator)
    def load_xml_variant(uri: str) -> mx.Document:
        print(f"Loading XML variant from: {uri}")
        doc = mx.createDocument() 
        doc.setSourceUri(uri)
        return doc
    
    xml_loader = create_document_loader(
        identifier="xml_variant_loader",
        name="XML Variant Loader",
        description="Loads XML variant documents",
        extensions=[".xmlvar"],
        import_func=load_xml_variant
    )
    
    # Register with the C++ system
    mx_render.registerDocumentLoader(xml_loader)
    print("Registered XML variant loader")


def test_plugin_system():
    """Test the plugin system integration."""
    print("\nTesting plugin system...")
    
    pm = get_plugin_manager()
    
    # Test importing a document (this will use registered loaders)
    try:
        # This would normally load a real file - just for demonstration
        print("Testing document import through plugin manager...")
        # doc = pm.import_document("example.json")  # Uncomment to test with real file
        print("Import test completed")
    except Exception as e:
        print(f"Import test failed (expected if file doesn't exist): {e}")
    
    # Test exporting a document
    try:
        print("Testing document export through plugin manager...")
        doc = mx.createDocument()
        # result = pm.export_document(doc, "test.custom")  # Uncomment to test with real file
        print("Export test completed")
    except Exception as e:
        print(f"Export test failed: {e}")


def main():
    """Main function to start the plugin system and prepare for GraphEditor."""
    
    print("=== MaterialX Plugin Manager + GraphEditor Integration ===\n")
    
    # 1. Initialize the plugin system
    pm = initialize_plugin_system()
    
    # 2. Set up GraphEditor integration
    #setup_graph_editor_integration()
    
    # 3. Register custom document loaders
    #register_custom_loaders()
    
    # 4. Discover and load plugins from a directory (optional)
    plugin_dirs = []
    envvar_plugin_dir = "MATERIALX_PLUGIN_DIR"
    if envvar_plugin_dir in os.environ:
        plugin_dir = Path(os.environ[envvar_plugin_dir])
        # Split ; or : if multiple directories are specified
        if ";" in plugin_dir.as_posix() or ":" in plugin_dir.as_posix():
            plugin_dirs.extend([Path(d.strip()) for d in plugin_dir.as_posix().split(';') if d.strip()])
        else:
            plugin_dir = Path(plugin_dir)
            plugin_dirs.append(plugin_dir)
        print(f"Using plugin directories from environment variable: {envvar_plugin_dir}")
    
    plugin_dir = Path.cwd() / "plugins"
    plugin_dirs.append(plugin_dir)

    for plugin_dir in plugin_dirs:
        if plugin_dir.exists():
            pm.discover_plugins(str(plugin_dir))
        else:
            print(f"Plugin directory {plugin_dir} does not exist - skipping plugin discovery")
    
    # 5. Test the system
    #test_plugin_system()
    
    print("\n=== Plugin System Ready ===")
    
    return pm


if __name__ == "__main__":
    plugin_manager = main()
    
    # Keep the program running if needed
    print("\nPlugin manager is active.")
    exit(0)

    #try:
    #    import time
    #    while True:
    #        time.sleep(1)
    #except KeyboardInterrupt:
    #    print("\nShutting down plugin manager...")
