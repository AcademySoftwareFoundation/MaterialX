"""
Materialx Python Plugin Manager.
Requires that pluggy be insstalled as well as the MaterialX Core and Render bindings be installed.
"""

try:
    import pluggy
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    from typing import Dict, List, Optional, Any
    from pathlib import Path
    import sys
    import traceback
    import inspect
    print("Manager: MaterialX Python bindings and pluggy are available")
except ImportError as e:
    print(f"Error importing required modules: {e}")
    print("Make sure MaterialX Python bindings and pluggy are installed")
    raise

# Plugin hook specifications
# In pluggy, two special markers are used to manage plugins:
# - `hookspec`: Attach this to methods in a "hook specification" class to define what plugins can do.
#   Think of these as the official list of plugin extension points.
# - `hookimpl`: Attach this to functions in plugin modules to show they provide code for a hook.
#   These are the actual plugin features.
# The plugin manager finds all `hookimpl` functions and calls them when the
# corresponding hook is invoked, allowing plugins to extend or customize behavior at runtime.
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
        #print('*** PythonDocumentLoader initialized:', identifier, name, description, extensions, version)
    
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
        self._registered_modules = set()  # Track registered modules
        
        # Set up callback for plugin registration events
        try:
            mx_render.setRegistrationCallback(self._on_plugin_registration)
            #print("MaterialXPluginManager initialized with C++ callback")
        except Exception as e:
            print(f"Warning: Could not set registration callback: {e}")            #print("MaterialXPluginManager initialized without C++ callback")
    
    def _on_plugin_registration(self, plugin_id: str, registered: bool):
        """Callback for plugin registration events."""
        if registered:
            print(f"Manager: Plugin registered: {plugin_id}")
        else:
            print(f"Manager: Plugin unregistered: {plugin_id}")
    
    def register_plugin(self, plugin_module):
        """Register a plugin module."""        # Check if module is already registered
        module_id = getattr(plugin_module, '__name__', str(plugin_module))
        if module_id in self._registered_modules:
            print(f"Manager: Module {module_id} already registered, skipping")
            return
        
        self._pm.register(plugin_module)
        self._load_plugins_from_module(plugin_module)
        self._registered_modules.add(module_id)
    
    def _load_plugins_from_module(self, plugin_module):
        """Load plugins from a specific module."""
        # Get the plugin instance(s) that were just registered from this module
        registered_plugins = self._pm.get_plugins()
        
        # Find plugins that belong to this module
        module_name = getattr(plugin_module, '__name__', str(plugin_module))
        module_plugins = []
        
        for plugin in registered_plugins:
            plugin_module_name = getattr(plugin, '__module__', None)
            if plugin_module_name == module_name:
                module_plugins.append(plugin)
        
        # Only call register_document_loaders on plugins from this specific module
        for plugin in module_plugins:
            if hasattr(plugin, 'register_document_loaders'):
                try:
                    loaders = plugin.register_document_loaders()
                    if loaders:
                        for loader in loaders:
                            if loader.getIdentifier() not in self._registered_plugins:
                                try:
                                    result = mx_render.registerDocumentLoader(loader)
                                    if result:
                                        self._registered_plugins.add(loader.getIdentifier())
                                    else:
                                        print(f"Manager: Warning: Failed to register loader {loader.getIdentifier()}")
                                except Exception as e:
                                    print(f"Manager: Error registering loader {loader.getIdentifier()}: {e}")
                except Exception as e:
                    print(f"Manager: Error getting loaders from plugin: {e}")
    
    def discover_plugins(self, plugin_dir: str = None):
        """Discover and load plugins from a directory."""
        if plugin_dir is None:
            plugin_dir = Path.cwd() / "plugins"
        else:
            plugin_dir = Path(plugin_dir)
        
        if not plugin_dir.exists():
            print(f"Manager: Plugin directory {plugin_dir} does not exist")
            return
        else:
            print(f"Manager: Scanning for plugins in {plugin_dir}...")
        
        # Add plugin directory to path temporarily
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
                    print(f"Manager: Loaded plugin: {module_name}")
                except Exception as e:
                    print(f"Manager: Failed to load plugin {module_name}: {e}")
        finally:
            # Remove plugin directory from path
            sys.path.remove(str(plugin_dir))
    
    def import_document(self, uri: str) -> Optional[mx.Document]:
        """Import a document using the appropriate loader."""
        try:
            return mx_render.importDocument(uri)
        except Exception as e:
            print(f"Error importing document {uri}: {e}")
            return None
    
    def export_document(self, document: mx.Document, uri: str) -> bool:
        """Export a document using the appropriate loader."""
        try:
            return mx_render.exportDocument(document, uri)
        except Exception as e:
            print(f"Error exporting document to {uri}: {e}")
            return False

# Global plugin manager instance
_plugin_manager = None

def get_plugin_manager() -> MaterialXPluginManager:
    """Get the global plugin manager instance."""
    global _plugin_manager
    if _plugin_manager is None:
        _plugin_manager = MaterialXPluginManager()
        #print(f"Using global MaterialX plugin manager instance {_plugin_manager}")
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
    print(f"Manager: Creating document loader: {identifier}, {name}, {description}, {extensions}, {version}")
    try:
        loader = PythonDocumentLoader(identifier, name, description, extensions, import_func, export_func, version)
        print("Manager: Document loader created successfully")
        return loader
    except Exception as e:
        print(f"Error creating document loader: {e}")
        traceback.print_exc()
        raise

# Convenience decorators for creating document loaders
def document_loader(identifier: str, name: str, description: str, 
                   extensions: List[str], version: str = "1.0.0", 
                   can_import: bool = True, can_export: bool = False):

    """Decorator for creating document loaders with error handling."""
    def decorator(func):
        print('Manager: Registering document loader:', identifier, name, description, extensions, version)
        # Determine if this is an import or export function based on signature
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
        try:
            result = mx_render.registerDocumentLoader(loader)
            if result:
                print(f"- Manager: Registered document loader: {identifier}")
            else:
                print(f"- Failed to register document loader: {identifier} (returned False)")
        except Exception as e:
            print(f"- Error registering document loader {identifier}: {e}")
            print("  This might happen if MaterialX libraries are not fully built")
            print("  The script will continue...")

        print("="*50)
        return func
    return decorator


