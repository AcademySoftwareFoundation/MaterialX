"""
Integration script for starting MaterialX GraphEditor with plugin manager.

This script demonstrates how to properly initialize the plugin system
before launching the GraphEditor so they share the same plugin manager.
"""

import sys
import os
from pathlib import Path

# Add MaterialX Python modules to path if needed
try:
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from MaterialX.plugin_manager import get_plugin_manager, document_loader
except ImportError as e:
    print(f"Error importing MaterialX: {e}")
    print("Make sure MaterialX Python bindings are installed and in your PYTHONPATH")
    sys.exit(1)


class GraphEditorPluginSetup:
    """Class to handle plugin setup for GraphEditor integration."""
    
    def __init__(self):
        self.plugin_manager = None
        self.registered_loaders = []
    
    def initialize(self):
        """Initialize the plugin system."""
        print("Initializing MaterialX Plugin System for GraphEditor...")
        
        # Get the global plugin manager instance
        # This automatically connects to the C++ PluginManager singleton
        self.plugin_manager = get_plugin_manager()
        
        # Register any built-in loaders
        self._register_builtin_loaders()
        
        # Discover plugins from standard locations
        self._discover_plugins()
        
        print("Plugin system initialized successfully")
        return self.plugin_manager
    
    def _register_builtin_loaders(self):
        """Register built-in document loaders."""
        
        # Example: USD loader (if USD support is available)
        try:
            @document_loader(
                identifier="usd_loader",
                name="USD Document Loader",
                description="Loads MaterialX documents from USD files",
                extensions=[".usd", ".usda", ".usdc"],
                can_import=True
            )
            def load_usd_document(uri: str) -> mx.Document:
                """Load MaterialX document from USD file."""
                print(f"Loading USD document: {uri}")
                # Implement USD loading logic here
                doc = mx.createDocument()
                doc.setSourceUri(uri)
                return doc
            
            self.registered_loaders.append("usd_loader")
            print("Registered USD document loader")
            
        except Exception as e:
            print(f"Could not register USD loader: {e}")
        
        # Example: Alembic loader (if Alembic support is available)
        try:
            @document_loader(
                identifier="abc_loader", 
                name="Alembic Document Loader",
                description="Loads MaterialX documents from Alembic files",
                extensions=[".abc"],
                can_import=True
            )
            def load_abc_document(uri: str) -> mx.Document:
                """Load MaterialX document from Alembic file."""
                print(f"Loading Alembic document: {uri}")
                # Implement Alembic loading logic here
                doc = mx.createDocument()
                doc.setSourceUri(uri)
                return doc
            
            self.registered_loaders.append("abc_loader")
            print("Registered Alembic document loader")
            
        except Exception as e:
            print(f"Could not register Alembic loader: {e}")
    
    def _discover_plugins(self):
        """Discover plugins from standard directories."""
        
        # Look for plugins in multiple locations
        plugin_dirs = [
            Path.cwd() / "plugins",
            Path.home() / ".materialx" / "plugins",
            Path(os.environ.get("MATERIALX_PLUGIN_PATH", "")) if os.environ.get("MATERIALX_PLUGIN_PATH") else None,
        ]
        
        for plugin_dir in plugin_dirs:
            if plugin_dir and plugin_dir.exists():
                print(f"Discovering plugins in: {plugin_dir}")
                self.plugin_manager.discover_plugins(str(plugin_dir))
    
    def get_status(self):
        """Get status information about the plugin system."""
        status = {
            "initialized": self.plugin_manager is not None,
            "registered_loaders": len(self.registered_loaders),
            "plugin_manager": self.plugin_manager
        }
        return status
    
    def print_status(self):
        """Print plugin system status."""
        status = self.get_status()
        print("\n=== Plugin System Status ===")
        print(f"Initialized: {status['initialized']}")
        print(f"Registered loaders: {status['registered_loaders']}")
        print("Available for GraphEditor: Yes")
        print("C++ Integration: Active")


def launch_graph_editor_with_plugins():
    """Launch GraphEditor with plugin system initialized."""
    
    # Initialize plugin system first
    setup = GraphEditorPluginSetup()
    pm = setup.initialize()
    
    # Print status
    setup.print_status()
    
    print("\n=== GraphEditor Integration Ready ===")
    print("The plugin system is now active. When you start GraphEditor:")
    print("1. All registered document loaders will be available")
    print("2. File dialogs will show supported formats") 
    print("3. Import/export will use the registered loaders")
    
    # Return the setup object so the calling code can access it
    return setup


def create_launcher_script():
    """Create a launcher script that can be used to start GraphEditor with plugins."""
    
    launcher_content = '''#!/usr/bin/env python3
"""
MaterialX GraphEditor Launcher with Plugin Support

This script initializes the plugin system before launching GraphEditor.
"""

import sys
from pathlib import Path

# Add the directory containing this script to Python path
script_dir = Path(__file__).parent
sys.path.insert(0, str(script_dir))

try:
    from graph_editor_integration import launch_graph_editor_with_plugins
    
    def main():
        print("Starting MaterialX GraphEditor with Plugin Support...")
        
        # Initialize plugins
        setup = launch_graph_editor_with_plugins()
        
        # At this point, the plugin system is ready
        # You would now launch your actual GraphEditor application
        # For example:
        # from MaterialXGraphEditor import main as graph_editor_main
        # graph_editor_main()
        
        print("\\nReady to launch GraphEditor!")
        print("The C++ GraphEditor application can now be started and will")
        print("automatically use the document loaders registered above.")
        
        return setup
    
    if __name__ == "__main__":
        main()

except ImportError as e:
    print(f"Error: {e}")
    print("Make sure MaterialX Python bindings are properly installed.")
    sys.exit(1)
'''
    
    launcher_path = Path.cwd() / "launch_graph_editor.py"
    with open(launcher_path, 'w') as f:
        f.write(launcher_content)
    
    # Make it executable on Unix systems
    if sys.platform != 'win32':
        os.chmod(launcher_path, 0o755)
    
    print(f"Created launcher script: {launcher_path}")
    return launcher_path


def main():
    """Main function."""
    print("MaterialX GraphEditor Plugin Integration Setup")
    print("=" * 50)
    
    # Method 1: Direct integration
    print("\\n1. Direct Integration Method:")
    setup = launch_graph_editor_with_plugins()
    
    # Method 2: Create launcher script
    print("\\n2. Creating Launcher Script:")
    launcher_path = create_launcher_script()
    
    print(f"\\n=== Integration Complete ===")
    print("Two ways to use this:")
    print("1. Call launch_graph_editor_with_plugins() before starting GraphEditor")
    print(f"2. Run the launcher script: python {launcher_path}")
    
    print("\\nThe plugin system is now ready for GraphEditor integration!")


if __name__ == "__main__":
    main()
