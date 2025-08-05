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
    def register_import_plugins(self) -> List[mx_render.ImportPlugin]:
        """Register import plugins.
        
        Returns:
            List of ImportPlugin instances
        """

    @hookspec  
    def register_export_plugins(self) -> List[mx_render.ExportPlugin]:
        """Register export plugins.
        
        Returns:
            List of ExportPlugin instances
        """


class PythonImportPlugin(mx_render.ImportPlugin):
    """Python wrapper for import plugins."""
    
    def __init__(self, identifier: str, name: str, description: str, 
                 extensions: List[str], import_func, version: str = "1.0.0"):
        super().__init__(identifier, name, description, extensions, version)
        self._import_func = import_func
    
    def importDocument(self, filename: str, options: Dict[str, str] = None) -> Optional[mx.Document]:
        """Import a document from a file."""
        if options is None:
            options = {}
        return self._import_func(filename, options)


class PythonExportPlugin(mx_render.ExportPlugin):
    """Python wrapper for export plugins."""
    
    def __init__(self, identifier: str, name: str, description: str,
                 extensions: List[str], export_func, version: str = "1.0.0"):
        super().__init__(identifier, name, description, extensions, version)
        self._export_func = export_func
    
    def exportDocument(self, document: mx.Document, filename: str, 
                      options: Dict[str, str] = None) -> bool:
        """Export a document to a file."""
        if options is None:
            options = {}
        return self._export_func(document, filename, options)


class MaterialXPluginManager:
    """Python plugin manager that integrates with the C++ plugin manager."""
    
    def __init__(self):
        self._pm = pluggy.PluginManager("materialx")
        self._pm.add_hookspecs(MaterialXHookSpec)
        self._cpp_manager = mx_render.PluginManager.getInstance()
        self._registered_plugins = set()
        
        # Set up callback for plugin registration events
        self._cpp_manager.setPluginRegistrationCallback(self._on_plugin_registration)
    
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
        # Get import plugins
        import_plugins = self._pm.hook.register_import_plugins()
        for plugin_list in import_plugins:
            if plugin_list:
                for plugin in plugin_list:
                    if plugin.getIdentifier() not in self._registered_plugins:
                        self._cpp_manager.registerImportPlugin(plugin)
                        self._registered_plugins.add(plugin.getIdentifier())
        
        # Get export plugins
        export_plugins = self._pm.hook.register_export_plugins()
        for plugin_list in export_plugins:
            if plugin_list:
                for plugin in plugin_list:
                    if plugin.getIdentifier() not in self._registered_plugins:
                        self._cpp_manager.registerExportPlugin(plugin)
                        self._registered_plugins.add(plugin.getIdentifier())
    
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
    
    def get_plugin_manager(self) -> mx_render.PluginManager:
        """Get the underlying C++ plugin manager."""
        return self._cpp_manager
    
    def list_plugins(self) -> List[mx_render.PluginInfo]:
        """List all registered plugins."""
        return self._cpp_manager.getAllPluginInfo()
    
    def import_document(self, filename: str, plugin_id: str = "", 
                       options: Dict[str, str] = None) -> Optional[mx.Document]:
        """Import a document using the appropriate plugin."""
        if options is None:
            options = {}
        return self._cpp_manager.importDocument(filename, plugin_id, options)
    
    def export_document(self, document: mx.Document, filename: str, 
                       plugin_id: str = "", options: Dict[str, str] = None) -> bool:
        """Export a document using the appropriate plugin."""
        if options is None:
            options = {}
        return self._cpp_manager.exportDocument(document, filename, plugin_id, options)
    
    def get_supported_import_extensions(self) -> List[str]:
        """Get all supported import extensions."""
        return self._cpp_manager.getSupportedImportExtensions()
    
    def get_supported_export_extensions(self) -> List[str]:
        """Get all supported export extensions."""
        return self._cpp_manager.getSupportedExportExtensions()


# Global plugin manager instance
_plugin_manager = None


def get_plugin_manager() -> MaterialXPluginManager:
    """Get the global plugin manager instance."""
    global _plugin_manager
    if _plugin_manager is None:
        _plugin_manager = MaterialXPluginManager()
    return _plugin_manager


def create_import_plugin(identifier: str, name: str, description: str,
                        extensions: List[str], import_func, version: str = "1.0.0") -> PythonImportPlugin:
    """Create an import plugin.
    
    Args:
        identifier: Unique plugin identifier
        name: Human-readable plugin name
        description: Plugin description
        extensions: List of supported file extensions
        import_func: Function that takes (filename, options) and returns Document
        version: Plugin version
        
    Returns:
        PythonImportPlugin instance
    """
    return PythonImportPlugin(identifier, name, description, extensions, import_func, version)


def create_export_plugin(identifier: str, name: str, description: str,
                        extensions: List[str], export_func, version: str = "1.0.0") -> PythonExportPlugin:
    """Create an export plugin.
    
    Args:
        identifier: Unique plugin identifier
        name: Human-readable plugin name  
        description: Plugin description
        extensions: List of supported file extensions
        export_func: Function that takes (document, filename, options) and returns bool
        version: Plugin version
        
    Returns:
        PythonExportPlugin instance
    """
    return PythonExportPlugin(identifier, name, description, extensions, export_func, version)


# Convenience decorators for creating plugins
def import_plugin(identifier: str, name: str, description: str, 
                 extensions: List[str], version: str = "1.0.0"):
    """Decorator for creating import plugins."""
    def decorator(func):
        plugin = create_import_plugin(identifier, name, description, extensions, func, version)
        # Auto-register with global plugin manager
        pm = get_plugin_manager()
        pm._cpp_manager.registerImportPlugin(plugin)
        pm._registered_plugins.add(identifier)
        return func
    return decorator


def export_plugin(identifier: str, name: str, description: str,
                 extensions: List[str], version: str = "1.0.0"):
    """Decorator for creating export plugins."""
    def decorator(func):
        plugin = create_export_plugin(identifier, name, description, extensions, func, version)
        # Auto-register with global plugin manager
        pm = get_plugin_manager()
        pm._cpp_manager.registerExportPlugin(plugin)
        pm._registered_plugins.add(identifier)
        return func
    return decorator
