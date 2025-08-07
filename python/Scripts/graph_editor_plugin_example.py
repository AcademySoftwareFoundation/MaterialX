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
import time
import gc
import atexit

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
    
    # Timer for initialization
    logger.info("Starting plugin system initialization...")
    start_time = time.time()
    plugin_manager = initialize_plugin_system()
    init_time = time.time() - start_time
    logger.info(f"Plugin initialization completed in {init_time:.3f} seconds")
    
    if plugin_manager:    
        logger.info("Plugin manager is active and ready.")

        # Timer for plugin registration check
        logger.info("Starting plugin registration check...")
        check_start_time = time.time()
        json_supported = check_plugin_registration(plugin_manager)
        check_time = time.time() - check_start_time
        logger.info(f"Plugin registration check completed in {check_time:.3f} seconds")        # Timer for cleanup/delete
        logger.info("Starting plugin manager cleanup...")
        delete_start_time = time.time()
        del plugin_manager
        # Force garbage collection to ensure cleanup happens immediately
        gc.collect()
        delete_time = time.time() - delete_start_time
        logger.info(f"Plugin manager cleanup completed in {delete_time:.3f} seconds")
        # Total time summary
        total_time = init_time + check_time + delete_time
        logger.info(f"Total operation time: {total_time:.3f} seconds (init: {init_time:.3f}s, check: {check_time:.3f}s, cleanup: {delete_time:.3f}s)")
        
    logger.info("Exiting main function...")
    # Add a small delay to see if there's additional cleanup happening after main()
    time.sleep(0.1)


def cleanup_and_exit():
    """Function to handle final cleanup and track exit timing."""
    exit_start = time.time()
    print(f"[{time.time():.3f}] Starting final cleanup and exit process...")
    
    # Strategy 1: Explicit cleanup of MaterialX objects first
    print(f"[{time.time():.3f}] Cleaning up MaterialX objects explicitly...")
    mx_cleanup_start = time.time()
    try:
        # Clear any global MaterialX references
        if 'mx' in globals():
            # Force cleanup of MaterialX module internals
            print(f"[{time.time():.3f}] Clearing MaterialX references...")
            
        # Try to clean up the plugin manager more aggressively
        import sys
        if 'plugin_manager' in sys.modules:
            pm_module = sys.modules['plugin_manager']
            if hasattr(pm_module, '_instance'):
                print(f"[{time.time():.3f}] Clearing plugin manager singleton...")
                pm_module._instance = None
                
        # Clear MaterialX from sys.modules to force unload
        modules_to_clear = [mod for mod in sys.modules.keys() if 'MaterialX' in mod or 'materialx' in mod.lower()]
        for mod_name in modules_to_clear:
            print(f"[{time.time():.3f}] Clearing module: {mod_name}")
            del sys.modules[mod_name]
            
    except Exception as e:
        print(f"[{time.time():.3f}] Error during MaterialX cleanup: {e}")
    
    mx_cleanup_time = time.time() - mx_cleanup_start
    print(f"[{time.time():.3f}] MaterialX cleanup completed in {mx_cleanup_time:.3f} seconds")
    
    # Strategy 2: Force multiple garbage collection cycles
    gc_start = time.time()
    print(f"[{time.time():.3f}] Running aggressive garbage collection...")
    for i in range(3):  # Multiple GC cycles
        collected = gc.collect()
        print(f"[{time.time():.3f}] GC cycle {i+1}: collected {collected} objects")
    gc_time = time.time() - gc_start
    print(f"[{time.time():.3f}] Garbage collection completed in {gc_time:.3f} seconds")
    
    # Strategy 3: Flush and close all file handles
    flush_start = time.time()
    print(f"[{time.time():.3f}] Flushing all I/O...")
    try:
        sys.stdout.flush()
        sys.stderr.flush()
        for handler in logger.handlers:
            handler.flush()
            if hasattr(handler, 'close'):
                handler.close()
    except Exception as e:
        print(f"[{time.time():.3f}] Error flushing I/O: {e}")
    flush_time = time.time() - flush_start
    print(f"[{time.time():.3f}] I/O flushing completed in {flush_time:.3f} seconds")
    
    exit_time = time.time() - exit_start
    print(f"[{time.time():.3f}] Final cleanup completed in {exit_time:.3f} seconds (mx: {mx_cleanup_time:.3f}s, gc: {gc_time:.3f}s, io: {flush_time:.3f}s)")
    print(f"[{time.time():.3f}] Python interpreter shutdown starting...")    # Strategy 4: Choose your exit strategy
    # Option A: Immediate exit (bypasses all Python cleanup)
    print(f"[{time.time():.3f}] Forcing immediate exit to bypass Python shutdown delay...")
    os._exit(0)  # This skips all cleanup and exits immediately
    
    # Option B: Windows-specific fast termination (uncomment to use instead)
    # windows_fast_shutdown()
    
    # Option C: Let Python handle shutdown normally (comment out os._exit above)
    # This will show the 3-second delay you're experiencing


def atexit_handler():
    """Handler called when Python interpreter is shutting down."""
    shutdown_start = time.time()
    print(f"[{time.time():.3f}] Python interpreter shutdown started")
    
    # Force one final garbage collection during shutdown
    print(f"[{time.time():.3f}] Running shutdown garbage collection...")
    gc.collect()
    
    # Try to measure how long the shutdown process takes
    # Note: This might not capture the full shutdown time as atexit handlers
    # run early in the shutdown process
    shutdown_time = time.time() - shutdown_start
    print(f"[{time.time():.3f}] atexit_handler completed in {shutdown_time:.3f} seconds")
    print(f"[{time.time():.3f}] Waiting for Python interpreter final cleanup...")


# Register the atexit handler to see when Python actually starts shutting down
atexit.register(atexit_handler)


# Alternative strategies to reduce cleanup time:

def minimal_mode_main():
    """Minimal mode that loads fewer dependencies for faster cleanup."""
    print("Running in minimal mode...")
    
    # Only import what's absolutely necessary
    start_time = time.time()
    
    try:
        # Lazy import MaterialX only when needed
        import MaterialX as mx
        from plugin_manager import get_plugin_manager
        
        # Create plugin manager without extensive testing
        pm = get_plugin_manager()
        print(f"Plugin manager created: {type(pm).__name__}")
        
        # Skip plugin loading and testing for speed
        if pm:
            plugin_info = pm.get_plugin_info()
            print(f"Found {plugin_info['count']} plugins")
        
        # Clean up immediately
        del pm
        del mx
        
    except ImportError as e:
        print(f"Import error: {e}")
    
    total_time = time.time() - start_time
    print(f"Minimal mode completed in {total_time:.3f} seconds")


def fast_exit_main():
    """Version that uses os._exit() for immediate termination."""
    print("Running with fast exit...")
    
    try:
        # Run normal operations
        main()
        
    finally:
        print("Using fast exit to skip cleanup...")
        import os
        os._exit(0)  # Skip all Python cleanup


def windows_fast_shutdown():
    """Windows-specific fast shutdown that terminates the process immediately."""
    try:
        if os.name == 'nt':  # Windows
            import ctypes
            print(f"[{time.time():.3f}] Using Windows TerminateProcess for immediate shutdown...")
            # Get current process handle and terminate it
            handle = ctypes.windll.kernel32.GetCurrentProcess()
            ctypes.windll.kernel32.TerminateProcess(handle, 0)
        else:
            print(f"[{time.time():.3f}] Non-Windows system, using os._exit()...")
            os._exit(0)
    except Exception as e:
        print(f"[{time.time():.3f}] Fast shutdown failed: {e}, falling back to os._exit()")
        os._exit(0)


# Command line argument parsing
if __name__ == "__main__":
    import sys
    
    mode = "fast"
    if len(sys.argv) > 1:
        mode = sys.argv[1]
    
    if mode == "minimal":
        minimal_mode_main()
    elif mode == "fast":
        fast_exit_main()
    elif mode == "normal":
        try:
            main()
        finally:
            cleanup_and_exit()
    else:
        print("Usage: python script.py [normal|minimal|fast]")
        print("  normal  - Full functionality with detailed cleanup timing")
        print("  minimal - Reduced functionality for faster cleanup")  
        print("  fast    - Normal functionality but immediate exit (skips cleanup)")
        sys.exit(1)
