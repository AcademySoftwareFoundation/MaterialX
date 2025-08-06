# MaterialX Plugins Directory

This directory contains example plugins for the MaterialX plugin system.

## Available Plugins

### 1. json_plugin.py
A comprehensive JSON plugin that demonstrates:
- Full plugin class implementation using the pluggy framework
- JSON import/export with complete MaterialX document structure
- Plugin hook implementation
- Both decorator and class-based approaches

**Features:**
- Imports/exports MaterialX documents to/from JSON format
- Handles node graphs, materials, shader references, and bindings
- Comprehensive error handling
- Metadata preservation

### 2. minimal_json_plugin.py
A simple example showing the minimal approach:
- Decorator-based plugin registration
- Basic JSON import/export functionality
- Good starting point for new plugins

**Features:**
- Simple JSON document structure
- Minimal code for quick understanding
- Basic MaterialX document creation

### 3. example_material.json
A sample JSON file demonstrating the expected format for JSON plugins.

**Contents:**
- Example surface shader node graph
- Texture processing node graph  
- Material definition with shader references
- Proper MaterialX JSON structure

## Usage

### Automatic Discovery
Place plugin files in this directory and they will be automatically discovered when the plugin system runs:

```python
from plugin_manager import get_plugin_manager

pm = get_plugin_manager()
pm.discover_plugins("path/to/plugins")
```

### Manual Registration
You can also register plugins manually:

```python
import plugins.json_plugin as jp
jp.register_plugin()
```

### Testing Plugins
Each plugin includes test functionality that can be run directly:

```bash
python plugins/json_plugin.py
python plugins/minimal_json_plugin.py
```

## Creating New Plugins

### Decorator Approach (Simple)
```python
from plugin_manager import document_loader

@document_loader(
    identifier="my_loader",
    name="My Document Loader",
    description="Loads my custom format",
    extensions=[".myformat"],
    can_import=True
)
def load_my_format(uri: str) -> mx.Document:
    # Implementation here
    return document
```

### Class Approach (Advanced)
```python
class MyPlugin:
    @hookimpl
    def register_document_loaders(self):
        loader = create_document_loader(...)
        return [loader]

# Register the plugin
pm = get_plugin_manager()
pm.register_plugin(MyPlugin())
```

## File Extensions

The JSON plugins handle `.json` files. To add support for other formats:

1. Change the `extensions` parameter in the `@document_loader` decorator
2. Implement the appropriate parsing logic for your format
3. Follow the same error handling patterns

## Integration with GraphEditor

Plugins registered through this system are automatically available to:
- Python MaterialX applications
- C++ GraphEditor applications  
- MaterialXView applications

All use the same shared PluginManager singleton.

## Environment Variables

Set `MATERIALX_PLUGIN_DIR` to specify additional plugin directories:

```bash
export MATERIALX_PLUGIN_DIR="/path/to/my/plugins;/another/plugin/dir"
```

The plugin system will automatically discover and load plugins from these directories.
