"""
Materialx Python Plugin Manager.
Requires that pluggy be insstalled as well as the MaterialX Core and Render bindings be installed.
"""

import os
import importlib.util
from pathlib import Path
from typing import Any, List, Union, Dict, Optional
from enum import Enum
import inspect

import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('MXpm')
logger.setLevel(logging.INFO)

try:
    import pluggy
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    logger.debug("MaterialX Python bindings and pluggy are available")
except ImportError as e:
    logger.info(f"Error importing required modules: {e}")
    logger.info("Make sure MaterialX Python bindings and pluggy are installed")
    raise

# The environment variable that contains the paths to the plugins
PLUGINS_ENV_VAR = "MATERIALX_PLUGIN_PATHS"

# The name of the plugin file
PLUGIN_FILE_NAME = "plugin"  # .py

# Necessary function name by each plugin to implement and return a plugin instance
PLUGIN_NAME_FUNCTION_NAME = "plugin_name"

# Necessary function name that each plugin must implement to return the type of plugin it is.
PLUGIN_TYPE_FUNCTION_NAME = "plugin_type"

# If a plugin decides it can be invalid (e.g. missing dependencies), it can implement this function
PLUGIN_VALID_FUNCTION_NAME = "is_valid"

hookspec = pluggy.HookspecMarker("materialx")
hookimpl = pluggy.HookimplMarker("materialx")

PathOrStr = Union[Path, str]

class MaterialXHookSpec:
    """Interfaces for MaterialX plugins."""

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
    
    # Types of plugins. For now there are only document loaders.
    class PluginType(Enum):
        DOCUMENT_LOADER = 0    

    def __init__(self):
        super().__init__("materialx")
        self.add_hookspecs(MaterialXHookSpec)
        self._registered_plugins = set()
        self._registered_modules = set()  # Track registered modules
          # Set up callback for plugin registration events
        try:
            mx_render.addRegistrationCallback("plugin_manager", self._on_plugin_registration)
            logger.info("MaterialXPluginManager initialized")
        except Exception as e:
            logger.info(f"Warning: Could not set registration callback: {e}")

    def __del__(self):
        """Destructor to clean up the plugin manager."""
        self._registered_plugins.clear()
        self._registered_modules.clear()
        try:
            mx_render.removeRegistrationCallback("plugin_manager")
            logger.info("MaterialXPluginManager destroyed and callback removed")
        except Exception as e:
            logger.info(f"Warning: Could not remove registration callback: {e}")        

        # perform any pluggy cleanup
        self.unregister_all()
        logger.info("MaterialXPluginManager cleaned up")

    def _on_plugin_registration(self, plugin_id: str, registered: bool):
        """Callback for plugin registration events."""
        if registered:
            logger.info(f"Plugin registered: {plugin_id}")
        else:
            logger.info(f"Plugin unregistered: {plugin_id}")
  
    
    def import_document(self, uri: str) -> Optional[mx.Document]:
        """Import a document using plugin hooks."""
        try:
            # Try each plugin's import hook
            results = self.hook.importDocument(uri=uri)
            for result in results:
                if result is not None:
                    return result
        except Exception as e:
            logger.info(f"Error importing document via hooks {uri}: {e}")
        
        # Fallback to C++ loader registry
        try:
            return mx_render.importDocument(uri)
        except Exception as e:
            logger.info(f"Error importing document {uri}: {e}")
            return None
    
    def export_document(self, document: mx.Document, uri: str) -> bool:
        """Export a document using plugin hooks."""
        try:
            # Try each plugin's export hook
            results = self.hook.exportDocument(document=document, uri=uri)
            for result in results:
                if result is True:
                    return True
        except Exception as e:
            logger.info(f"Error exporting document via hooks to {uri}: {e}")
        
        # Fallback to C++ loader registry
        try:
            return mx_render.exportDocument(document, uri)
        except Exception as e:
            logger.info(f"Error exporting document to {uri}: {e}")
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
        
        if hasattr(module, PLUGIN_TYPE_FUNCTION_NAME):
            plugin_type_function = getattr(module, PLUGIN_TYPE_FUNCTION_NAME)
            plugin_type: Any = plugin_type_function()
            if not isinstance(plugin_type, MaterialXPluginManager.PluginType):
                logger.warning(f"Plugin type {plugin_type} is not valid. Expected PluginType enum. Skipping plugin at {module.__file__}")
                return
        else:
            logger.warning(
                f"Found plugin at {module.__file__}, but it does not have a '{PLUGIN_TYPE_FUNCTION_NAME}' function."
            )
            return

        # Check for presence of PLUGIN_VALID_FUNCTION_NAME function and skip if it returns False
        if hasattr(module, PLUGIN_VALID_FUNCTION_NAME):
            plugin_valid_function = getattr(module, PLUGIN_VALID_FUNCTION_NAME)
            if not plugin_valid_function():
                logger.warning(f"Plugin {plugin_name} is not valid. Skipping plugin at {module.__file__}")
                return            
        # Register the plugin - look for plugin classes with hookimpl decorators
        self._register_plugin(module, plugin_name, plugin_type)

    def _has_hookmplementations(self, module) -> bool:
        """Check if a module has any hook implementations."""
        for name in dir(module):
            obj = getattr(module, name)
            # Check for class methods decorated with hookimpl
            if inspect.isclass(obj) and self._has_hookimpl_methods(obj):
                return obj(), name
            # Check for functions decorated with hookimpl
            elif inspect.isfunction(obj) and hasattr(obj, 'materialx_impl'):
                return obj(), name
        return None, None

    def _register_plugin(self, module, plugin_name: str, plugin_type : PluginType):
        """Discover and register plugin instances from a module."""

        try:
            # Look for classes and functions in the module that have hookimpl methods
            plugin_instance, name = self._has_hookmplementations(module)
            if plugin_instance:
                if plugin_type == MaterialXPluginManager.PluginType.DOCUMENT_LOADER:
                    self.register(plugin_instance, plugin_name)
                    logger.info(f"Registered plugin class '{name}' as '{plugin_name}' from {module.__file__}")
                    found_plugin = True
                else:
                    logger.warning(f"Unsupported plugin type '{plugin_type}' for class '{name}' in {module.__file__}")
                    found_plugin = True

            # If no plugin class or function found, register the module itself
            if not plugin_instance:
                self.register(module, plugin_name)
                logger.info(f"Registered plugin module '{plugin_name}' '{plugin_type}' from {module.__file__}")            
            
        except ValueError:
            logger.warning(f"Plugin with name '{plugin_name}' already registered. Skipping plugin at {module.__file__}")
        except Exception as e:
            logger.warning(f"Error registering plugin '{plugin_name}' '{plugin_type}': {e}")

    def _has_hookimpl_methods(self, cls):
        """Check if a class has methods decorated with hookimpl."""
        for method_name in dir(cls):
            if not method_name.startswith('_'):
                method = getattr(cls, method_name)
                if callable(method) and hasattr(method, 'materialx_impl'):
                    return True
        return False

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

    def get_all_supported_extensions(self) -> List[str]:
        """Get all supported file extensions from all registered plugins.
        
        Returns:
            List of unique file extensions supported by all plugins
        """
        all_extensions = []
        try:
            # Get extensions from all plugins via hooks
            extension_results = self.hook.supportedExtensions()
            for extensions in extension_results:
                if extensions:
                    all_extensions.extend(extensions)
        except Exception as e:
            logger.info(f"Error getting supported extensions from hooks: {e}")
          # Return unique extensions
        return list(set(all_extensions))
    
    def get_plugin_info(self) -> Dict[str, Any]:
        """Get information about all registered plugins.
        
        Returns:
            Dictionary containing plugin information
        """
        plugin_info = {
            'count': 0,
            'extensions': self.get_all_supported_extensions(),
            'plugins': []
        }
        
        try:
            # Get plugin names and capabilities
            names = self.hook.getUIName()
            import_capabilities = self.hook.canImport()
            export_capabilities = self.hook.canExport()
            extension_lists = self.hook.supportedExtensions()
            
            plugin_info['count'] = len(names)
            
            for i, (name, can_import, can_export, extensions) in enumerate(
                zip(names, import_capabilities, export_capabilities, extension_lists)
            ):
                plugin_info['plugins'].append({
                    'name': name,
                    'can_import': can_import,
                    'can_export': can_export,
                    'extensions': extensions or []
                })
        except Exception as e:
            logger.info(f"Error getting plugin information: {e}")
        
        return plugin_info

# Global plugin manager instance
_plugin_manager = None

def get_plugin_manager() -> MaterialXPluginManager:
    """Get the global plugin manager instance."""
    global _plugin_manager
    if _plugin_manager is None:
        _plugin_manager = MaterialXPluginManager()
        logger.info(f"Using global MaterialX plugin manager instance")
    return _plugin_manager




