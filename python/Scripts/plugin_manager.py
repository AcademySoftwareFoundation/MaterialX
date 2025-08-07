"""
Materialx Python Plugin Manager.
Requires that pluggy be insstalled as well as the MaterialX Core and Render bindings be installed.
"""

import sys
import os
import importlib.util
from pathlib import Path
from typing import TYPE_CHECKING, Any, List, Union
import logging

try:
    import pluggy
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from typing import Dict, List, Optional, Any
    import traceback
    import inspect
    print("Manager: MaterialX Python bindings and pluggy are available")
except ImportError as e:
    print(f"Error importing required modules: {e}")
    print("Make sure MaterialX Python bindings and pluggy are installed")
    raise

# The environment variable that contains the paths to the plugins
PLUGINS_ENV_VAR = "MATERIALX_PLUGIN_PATHS"

# The name of the plugin file
PLUGIN_FILE_NAME = "plugin"  # .py

# Necessary function name by each plugin to implement and return a plugin instance
PLUGIN_NAME_FUNCTION_NAME = "plugin_name"

# If a plugin decides it can be invalid (e.g. missing dependencies), it can implement this function
PLUGIN_VALID_FUNCTION_NAME = "is_valid"

logger = logging.getLogger(__name__)
hookspec = pluggy.HookspecMarker("materialx")
hookimpl = pluggy.HookimplMarker("materialx")

PathOrStr = Union[Path, str]

class MaterialXHookSpec:
    """Interfaces for MaterialX plugins."""

    @hookspec
    def register_document_loaders(self) -> List[mx_render.DocumentLoader]:
        """Register document loaders.
        
        Returns:
            List of DocumentLoader instances
        """

    @hookspec
    def importDocument(self, uri: str) -> Optional[mx.Document]:
        """Import a document from a URI.
        
        Args:
            uri: URI of the document to import
        
        Returns:
            mx.Document instance or None if import failed
        """

    @hookspec
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Export a document to a URI.
        
        Args:
            document: mx.Document instance to export
            uri: URI where the document should be exported
        
        Returns:
            True if export succeeded, False otherwise
        """

    @hookspec
    def supportedExtensions(self) -> List[str]:
        """Get the list of supported file extensions.
        
        Returns:
            List of strings representing supported file extensions
        """

    @hookspec
    def getUIName(self) -> str:
        """Get the human-readable name for the plugin.
        
        Returns:
            String representing the plugin name
        """

    @hookspec
    def canImport(self) -> bool:
        """Check if the plugin can import.
        
        Returns:
            True if the plugin supports importing, False otherwise
        """

    @hookspec
    def canExport(self) -> bool:
        """Check if the plugin can export.
        
        Returns:
            True if the plugin supports exporting, False otherwise
        """

class MaterialXPluginManager(pluggy.PluginManager):
    """Plugin manager that uses hookspec/hookimpl pattern for MaterialX plugins."""
    
    def __init__(self):
        super().__init__("materialx")
        self.add_hookspecs(MaterialXHookSpec)
        self._registered_plugins = set()
        self._registered_modules = set()  # Track registered modules
        
        # Set up callback for plugin registration events
        try:
            mx_render.setRegistrationCallback(self._on_plugin_registration)
            print("MaterialXPluginManager initialized with C++ callback")
        except Exception as e:
            print(f"Warning: Could not set registration callback: {e}")
    
    def _on_plugin_registration(self, plugin_id: str, registered: bool):
        """Callback for plugin registration events."""
        if registered:
            print(f"Manager: Plugin registered: {plugin_id}")
        else:
            print(f"Manager: Plugin unregistered: {plugin_id}")
    
    def register_plugin(self, plugin_module):
        """Register a plugin module and load its document loaders."""
        # Check if module is already registered
        module_id = getattr(plugin_module, '__name__', str(plugin_module))
        if module_id in self._registered_modules:
            print(f"Manager: Module {module_id} already registered, skipping")
            return
        
        self.register(plugin_module)
        self._create_document_loaders_from_hooks(plugin_module)
        self._registered_modules.add(module_id)
    
    def _create_document_loaders_from_hooks(self, plugin_module):
        """Create document loaders from plugin hooks."""
        try:
            # Use the hook system to get loaders from registered plugins
            loaders = self.hook.register_document_loaders()
            
            # Filter loaders from this specific module
            if loaders:
                for loader in loaders:
                    if loader and loader.getIdentifier() not in self._registered_plugins:
                        try:
                            result = mx_render.registerDocumentLoader(loader)
                            if result:
                                self._registered_plugins.add(loader.getIdentifier())
                                print(f"Manager: Registered document loader: {loader.getIdentifier()}")
                            else:
                                print(f"Manager: Warning: Failed to register loader {loader.getIdentifier()}")
                        except Exception as e:
                            print(f"Manager: Error registering loader {loader.getIdentifier()}: {e}")
        except Exception as e:
            print(f"Manager: Error creating loaders from hooks: {e}")
    
    def import_document_via_hooks(self, uri: str) -> Optional[mx.Document]:
        """Import a document using plugin hooks."""
        try:
            # Try each plugin's import hook
            results = self.hook.importDocument(uri=uri)
            for result in results:
                if result is not None:
                    return result
        except Exception as e:
            print(f"Error importing document via hooks {uri}: {e}")
        
        # Fallback to C++ loader registry
        try:
            return mx_render.importDocument(uri)
        except Exception as e:
            print(f"Error importing document {uri}: {e}")
            return None
    
    def export_document_via_hooks(self, document: mx.Document, uri: str) -> bool:
        """Export a document using plugin hooks."""
        try:
            # Try each plugin's export hook
            results = self.hook.exportDocument(document=document, uri=uri)
            for result in results:
                if result is True:
                    return True
        except Exception as e:
            print(f"Error exporting document via hooks to {uri}: {e}")
        
        # Fallback to C++ loader registry
        try:
            return mx_render.exportDocument(document, uri)
        except Exception as e:
            print(f"Error exporting document to {uri}: {e}")
            return False
    
    def load_plugins_from_dir(self, plugin_dir: PathOrStr):
        """Load plugins from a directory containing plugin.py files."""
        plugin_dir = Path(plugin_dir) if isinstance(plugin_dir, str) else plugin_dir
        
        # Check for the presence of plugin.py in the subfolder
        plugin_file = plugin_dir / "plugin.py"
        if not plugin_file.exists():
            logger.warning(f"Plugin file not found at {plugin_file}")
            return

        module_name = f"{str(plugin_dir)}.plugin"
        spec = importlib.util.spec_from_file_location(module_name, plugin_file)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        # Check for necessary presence of PLUGIN_NAME_FUNCTION_NAME function
        # and skip if it does not exist or does not return a string
        if hasattr(module, PLUGIN_NAME_FUNCTION_NAME):
            plugin_name_function = getattr(module, PLUGIN_NAME_FUNCTION_NAME)
            plugin_name: Any = plugin_name_function()
            if not isinstance(plugin_name, str):
                logger.warning(f"Plugin name {plugin_name} is not valid. Skipping plugin at {module.__file__}")
                return
        else:
            logger.warning(
                f"Found plugin at {module.__file__}, but it does not have a '{PLUGIN_NAME_FUNCTION_NAME}' function."
            )
            return

        # Check for presence of PLUGIN_VALID_FUNCTION_NAME function and skip if it returns False
        if hasattr(module, PLUGIN_VALID_FUNCTION_NAME):
            plugin_valid_function = getattr(module, PLUGIN_VALID_FUNCTION_NAME)
            if not plugin_valid_function():
                logger.warning(f"Plugin {plugin_name} is not valid. Skipping plugin at {module.__file__}")
                return

        try:
            self.register(module, plugin_name)
            logger.info(f"Registered plugin '{plugin_name}' at {module.__file__}")
        except ValueError:
            logger.warning(f"Plugin with name '{plugin_name}' already registered. Skipping plugin at {module.__file__}")

    def load_plugins_from_environment_variable(self, environment_variable: str = PLUGINS_ENV_VAR):
        """Loop through an environment variable and load all plugins from the directories specified in the environment."""
        env_value: str = os.getenv(environment_variable, "")
        env_values: List[str] = [env_value for env_value in env_value.split(os.pathsep) if env_value]
        if not env_values:
            logger.info(f"No plugin paths found in environment variable '{environment_variable}'")
            return

        plugin_root_dirs: List[Path] = [Path(env_value) for env_value in env_values if Path(env_value).is_dir()]
        for plugin_root_dir in plugin_root_dirs:
            logger.info(f"Loading plugins from directory: {plugin_root_dir}")
            self.load_plugins_from_dir(plugin_root_dir)


class HookBasedDocumentLoader(mx_render.DocumentLoader):
    """Document loader that uses plugin hooks instead of direct function callbacks."""
    
    def __init__(self, identifier: str, name: str, description: str, plugin_manager, plugin_instance):
        super().__init__(identifier, name, description)
        self._plugin_manager = plugin_manager
        self._plugin_instance = plugin_instance
        
    def supportedExtensions(self):
        """Get supported extensions from plugin hooks."""
        try:
            if hasattr(self._plugin_instance, 'supportedExtensions'):
                extensions = self._plugin_instance.supportedExtensions()
                return {str(ext) for ext in extensions} if extensions else set()
        except Exception as e:
            print(f"Error getting supported extensions: {e}")
        return set()
    
    def importDocument(self, uri: str) -> Optional[mx.Document]:
        """Import a document using plugin hooks."""
        try:
            if hasattr(self._plugin_instance, 'importDocument'):
                return self._plugin_instance.importDocument(uri)
        except Exception as e:
            print(f"Error importing document via hooks: {e}")
        return None
    
    def exportDocument(self, document: mx.Document, uri: str) -> bool:
        """Export a document using plugin hooks."""
        try:
            if hasattr(self._plugin_instance, 'exportDocument'):
                return self._plugin_instance.exportDocument(document, uri)
        except Exception as e:
            print(f"Error exporting document via hooks: {e}")
        return False

# Global plugin manager instance
_plugin_manager = None

def get_plugin_manager() -> MaterialXPluginManager:
    """Get the global plugin manager instance."""
    global _plugin_manager
    if _plugin_manager is None:
        _plugin_manager = MaterialXPluginManager()
        print(f"Using global MaterialX plugin manager instance")
    return _plugin_manager

def create_hook_based_loader(identifier: str, name: str, description: str, 
                           plugin_manager, plugin_instance) -> HookBasedDocumentLoader:
    """Create a hook-based document loader.
    
    Args:
        identifier: Unique loader identifier
        name: Human-readable loader name
        description: Loader description
        plugin_manager: The plugin manager instance
        plugin_instance: The plugin instance that implements the hooks
        
    Returns:
        HookBasedDocumentLoader instance
    """
    print(f"Manager: Creating hook-based document loader: {identifier}, {name}, {description}")
    try:
        loader = HookBasedDocumentLoader(identifier, name, description, plugin_manager, plugin_instance)
        print("Manager: Hook-based document loader created successfully")
        return loader
    except Exception as e:
        print(f"Error creating hook-based document loader: {e}")
        traceback.print_exc()
        raise

# Updated decorator to work with hook-based system
def plugin_hookimpl(func):
    """Decorator to mark a function as a hook implementation."""
    return hookimpl(func)

# Convenience function for plugin registration
def register_plugin_with_manager(plugin_instance, manager=None):
    """Register a plugin instance with the manager."""
    if manager is None:
        manager = get_plugin_manager()
    
    try:
        manager.register(plugin_instance)
        print(f"Manager: Registered plugin instance: {type(plugin_instance).__name__}")
    except Exception as e:
        print(f"Manager: Error registering plugin instance: {e}")
        traceback.print_exc()


