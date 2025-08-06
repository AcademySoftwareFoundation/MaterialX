"""
Python plugin system for MaterialX using pluggy.

This module provides a Python-based plugin system that integrates with the
MaterialX C++ plugin manager. It uses pluggy for plugin discovery and
registration.
"""

import pluggy
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
from typing import Dict, List, Optional, Any
from pathlib import Path


# Plugin hook specifications
hookspec = pluggy.HookspecMarker("materialx")
hookimpl = pluggy.HookimplMarker("materialx")


class MaterialXHookSpec:
    """Hook specifications for MaterialX plugins."""

    @hookspec
    def register_document_loaders(self) -> List[mx_render.DocumentLoader]:
        """Register document loaders.
        
        Returns:
            List of DocumentLoader instances
        """


class PythonDocumentLoader(mx_render.DocumentLoader):
    """Python wrapper for document loaders."""
    
    def __init__(self, identifier: str, name: str, description: str, 
                 extensions: List[str], import_func=None, export_func=None, version: str = "1.0.0"):
        super().__init__(identifier, name, description)
        self._extensions = set(extensions)
        self._import_func = import_func
        self._export_func = export_func
        self._version = version
    
    def supportedExtensions(self):
        """Get supported extensions."""
        # Return a Python set of strings that pybind11 can convert to std::set<string>
        # Ensure all extensions are strings
        return {str(ext) for ext in self._extensions}
    
    def importDocument(self, uri: str) -> Optional[mx.Document]:
        """Import a document from a URI."""
        if self._import_func:
            return self._import_func(uri)
        return None
    
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Export a document to a URI."""
        if self._export_func:
            return self._export_func(document, uri)
        return False


class MaterialXPluginManager:
    """Python plugin manager that integrates with the C++ plugin manager."""
    
    def __init__(self):
        self._pm = pluggy.PluginManager("materialx")
        self._pm.add_hookspecs(MaterialXHookSpec)
        self._registered_plugins = set()
        
        # Set up callback for plugin registration events
        mx_render.setRegistrationCallback(self._on_plugin_registration)
    
    def _on_plugin_registration(self, plugin_id: str, registered: bool):
        """Callback for plugin registration events."""
        if registered:
            print(f"Plugin registered: {plugin_id}")
        else:
            print(f"Plugin unregistered: {plugin_id}")
    
    def register_plugin(self, plugin_module):
        """Register a plugin module."""
        self._pm.register(plugin_module)
        self._load_plugins_from_module(plugin_module)
    
    def _load_plugins_from_module(self, plugin_module):
        """Load plugins from a module."""
        # Get document loaders
        loaders = self._pm.hook.register_document_loaders()
        for loader_list in loaders:
            if loader_list:
                for loader in loader_list:
                    if loader.getIdentifier() not in self._registered_plugins:
                        mx_render.registerDocumentLoader(loader)
                        self._registered_plugins.add(loader.getIdentifier())
    
    def discover_plugins(self, plugin_dir: str = None):
        """Discover and load plugins from a directory."""
        if plugin_dir is None:
            plugin_dir = Path.cwd() / "plugins"
        else:
            plugin_dir = Path(plugin_dir)
        
        if not plugin_dir.exists():
            print(f"Plugin directory {plugin_dir} does not exist")
            return
        
        # Add plugin directory to path temporarily
        import sys
        sys.path.insert(0, str(plugin_dir))
        
        try:
            # Find all Python files in plugin directory
            for plugin_file in plugin_dir.glob("*.py"):
                if plugin_file.name.startswith("__"):
                    continue
                
                module_name = plugin_file.stem
                try:
                    module = __import__(module_name)
                    self.register_plugin(module)
                    print(f"Loaded plugin: {module_name}")
                except Exception as e:
                    print(f"Failed to load plugin {module_name}: {e}")
        finally:
            # Remove plugin directory from path
            sys.path.remove(str(plugin_dir))
    
    def import_document(self, uri: str) -> Optional[mx.Document]:
        """Import a document using the appropriate loader."""
        return mx_render.importDocument(uri)
    
    def export_document(self, document: mx.Document, uri: str) -> bool:
        """Export a document using the appropriate loader."""
        return mx_render.exportDocument(document, uri)


# Global plugin manager instance
_plugin_manager = None


def get_plugin_manager() -> MaterialXPluginManager:
    """Get the global plugin manager instance."""
    global _plugin_manager
    if _plugin_manager is None:
        _plugin_manager = MaterialXPluginManager()
    return _plugin_manager


def create_document_loader(identifier: str, name: str, description: str,
                          extensions: List[str], import_func=None, export_func=None, 
                          version: str = "1.0.0") -> PythonDocumentLoader:
    """Create a document loader.
    
    Args:
        identifier: Unique loader identifier
        name: Human-readable loader name
        description: Loader description
        extensions: List of supported file extensions
        import_func: Function that takes (uri) and returns Document (optional)
        export_func: Function that takes (document, uri) and returns bool (optional)
        version: Loader version
        
    Returns:
        PythonDocumentLoader instance
    """
    return PythonDocumentLoader(identifier, name, description, extensions, import_func, export_func, version)


# Convenience decorators for creating document loaders
def document_loader(identifier: str, name: str, description: str, 
                   extensions: List[str], version: str = "1.0.0", 
                   can_import: bool = True, can_export: bool = False):
    """Decorator for creating document loaders."""
    def decorator(func):
        # Determine if this is an import or export function based on signature
        import inspect
        sig = inspect.signature(func)
        params = list(sig.parameters.keys())
        
        import_func = None
        export_func = None
        
        if can_import and len(params) == 1:
            # Single parameter = import function (uri)
            import_func = func
        elif can_export and len(params) == 2:
            # Two parameters = export function (document, uri)
            export_func = func
        elif can_import:
            # Default to import if can_import is True
            import_func = func
            
        loader = create_document_loader(identifier, name, description, extensions, 
                                      import_func, export_func, version)
        # Auto-register with global plugin manager
        mx_render.registerDocumentLoader(loader)
        return func
    return decorator


def initialize_plugin_system(plugin_dirs: List[str] = None, auto_discover: bool = True) -> MaterialXPluginManager:
    """Initialize the MaterialX plugin system with common settings.
    
    This is a convenience function that:
    1. Gets the global plugin manager instance
    2. Optionally discovers plugins from specified directories
    3. Sets up integration with the C++ PluginManager
    
    Args:
        plugin_dirs: List of directories to search for plugins (optional)
        auto_discover: Whether to automatically discover plugins in standard locations
        
    Returns:
        The initialized MaterialXPluginManager instance
    """
    pm = get_plugin_manager()
    
    if auto_discover:
        # Standard plugin discovery locations
        standard_dirs = []
        
        # Current working directory plugins folder
        cwd_plugins = Path.cwd() / "plugins"
        if cwd_plugins.exists():
            standard_dirs.append(str(cwd_plugins))
        
        # Home directory plugins folder
        import os
        home_plugins = Path.home() / ".materialx" / "plugins"
        if home_plugins.exists():
            standard_dirs.append(str(home_plugins))
        
        # Environment variable plugin path
        env_plugin_path = os.environ.get("MATERIALX_PLUGIN_PATH")
        if env_plugin_path:
            for path in env_plugin_path.split(os.pathsep):
                plugin_path = Path(path)
                if plugin_path.exists():
                    standard_dirs.append(str(plugin_path))
        
        # Discover plugins in standard directories
        for plugin_dir in standard_dirs:
            pm.discover_plugins(plugin_dir)
    
    # Discover plugins in user-specified directories
    if plugin_dirs:
        for plugin_dir in plugin_dirs:
            pm.discover_plugins(plugin_dir)
    
    return pm


# Convenience function for GraphEditor integration
def setup_for_graph_editor(plugin_dirs: List[str] = None) -> MaterialXPluginManager:
    """Set up the plugin system specifically for GraphEditor integration.
    
    This function initializes the plugin system in a way that's optimized
    for use with MaterialX GraphEditor applications.
    
    Args:
        plugin_dirs: Additional plugin directories to search (optional)
        
    Returns:
        The configured MaterialXPluginManager instance
    """
    print("Setting up MaterialX plugin system for GraphEditor...")
    
    # Initialize with auto-discovery
    pm = initialize_plugin_system(plugin_dirs=plugin_dirs, auto_discover=True)
    
    print("Plugin system ready for GraphEditor integration")
    print("All registered document loaders will be available to GraphEditor")
    
    return pm
