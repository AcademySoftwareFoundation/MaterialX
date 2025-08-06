"""
Simple JSON Plugin Example

This is a minimal example showing how to create a plugin that handles
.json files for MaterialX document import/export.

This demonstrates the simplest way to create a plugin using decorators.
"""

import json
import MaterialX as mx
from pathlib import Path

# Import the decorator from the plugin manager
try:
    from plugin_manager import document_loader
except ImportError:
    # Handle case where plugin_manager is in parent directory
    import sys
    import os
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from plugin_manager import document_loader


@document_loader(
    identifier="minimal_json_loader",
    name="Minimal JSON Loader",
    description="Minimal JSON loader for demonstration",
    extensions=[".json"],
    can_import=True,
    version="1.0.0"
)
def load_json_document(uri: str) -> mx.Document:
    """
    Load a JSON file and create a MaterialX document.
    
    This is a simple example that creates a basic document structure
    from JSON data.
    """
    print(f"Minimal JSON Plugin: Loading {uri}")
    
    try:
        # Read JSON file
        with open(uri, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # Create MaterialX document
        doc = mx.createDocument()
        doc.setSourceUri(uri)
        
        # Set basic metadata
        if isinstance(data, dict):
            # Handle structured JSON
            if 'materialx' in data:
                materialx_data = data['materialx']
                
                # Set document properties
                if 'version' in materialx_data:
                    doc.setVersionString(str(materialx_data['version']))
                if 'colorspace' in materialx_data:
                    doc.setColorSpace(str(materialx_data['colorspace']))
                
                # Create nodes from data
                if 'nodes' in materialx_data:
                    nodegraph = doc.addNodeGraph("imported_graph")
                    
                    for node_name, node_data in materialx_data['nodes'].items():
                        if isinstance(node_data, dict):
                            category = node_data.get('category', 'constant')
                            node = nodegraph.addNode(category, node_name)
                            
                            # Set node inputs
                            if 'inputs' in node_data:
                                for input_name, input_value in node_data['inputs'].items():
                                    node.setInputValue(input_name, input_value)
            else:
                # Handle generic JSON - create a metadata node
                nodegraph = doc.addNodeGraph("json_data")
                metadata_node = nodegraph.addNode("constant", "metadata")
                metadata_node.setInputValue("value", str(data)[:100] + "..." if len(str(data)) > 100 else str(data))
        
        print(f"Minimal JSON Plugin: Successfully loaded {uri}")
        return doc
        
    except Exception as e:
        print(f"Minimal JSON Plugin: Error loading {uri}: {e}")
        # Return empty document instead of None to avoid crashes
        doc = mx.createDocument()
        doc.setSourceUri(uri)
        return doc


@document_loader(
    identifier="minimal_json_exporter",
    name="Minimal JSON Exporter", 
    description="Minimal JSON exporter for demonstration",
    extensions=[".json"],
    can_export=True,
    version="1.0.0"
)
def export_json_document(document: mx.Document, uri: str) -> bool:
    """
    Export a MaterialX document to a JSON file.
    
    This creates a simple JSON representation of the document.
    """
    print(f"Minimal JSON Plugin: Exporting to {uri}")
    
    try:
        # Create JSON structure
        export_data = {
            "materialx": {
                "version": document.getVersionString(),
                "colorspace": document.getColorSpace(),
                "source_uri": document.getSourceUri(),
                "exported_by": "Minimal JSON Plugin v1.0.0",
                "nodegraphs": {},
                "materials": {}
            }
        }
        
        # Export node graphs
        for nodegraph in document.getNodeGraphs():
            ng_data = {
                "name": nodegraph.getName(),
                "type": nodegraph.getType(),
                "nodes": {}
            }
            
            for node in nodegraph.getNodes():
                node_data = {
                    "category": node.getCategory(),
                    "type": node.getType(),
                    "inputs": {}
                }
                
                # Export node inputs
                for input_elem in node.getInputs():
                    node_data["inputs"][input_elem.getName()] = input_elem.getValueString()
                
                ng_data["nodes"][node.getName()] = node_data
            
            export_data["materialx"]["nodegraphs"][nodegraph.getName()] = ng_data
        
        # Export materials
        for material in document.getMaterials():
            mat_data = {
                "name": material.getName(),
                "shader_refs": []
            }
            
            for shader_ref in material.getShaderRefs():
                ref_data = {
                    "name": shader_ref.getName(),
                    "node": shader_ref.getReferencedNode()
                }
                mat_data["shader_refs"].append(ref_data)
            
            export_data["materialx"]["materials"][material.getName()] = mat_data
        
        # Write to file
        with open(uri, 'w', encoding='utf-8') as f:
            json.dump(export_data, f, indent=2, ensure_ascii=False)
        
        print(f"Minimal JSON Plugin: Successfully exported to {uri}")
        return True
        
    except Exception as e:
        print(f"Minimal JSON Plugin: Error exporting to {uri}: {e}")
        return False


# Plugin info
PLUGIN_INFO = {
    "name": "Minimal JSON Plugin",
    "version": "1.0.0", 
    "description": "Simple JSON import/export for MaterialX documents",
    "author": "MaterialX Example",
    "supported_extensions": [".json"]
}


def get_plugin_info():
    """Return plugin information."""
    return PLUGIN_INFO


if __name__ == "__main__":
    print("Minimal JSON Plugin loaded")
    print(f"Plugin info: {PLUGIN_INFO}")
    
    # Test the plugin
    test_data = {
        "materialx": {
            "version": "1.38",
            "colorspace": "lin_rec709",
            "nodes": {
                "test_constant": {
                    "category": "constant",
                    "inputs": {
                        "value": 0.5
                    }
                }
            }
        }
    }
    
    # Write test file
    test_file = "test_minimal.json"
    with open(test_file, 'w') as f:
        json.dump(test_data, f, indent=2)
    
    print(f"Created test file: {test_file}")
    
    # Test import
    doc = load_json_document(test_file)
    print(f"Import test: {len(doc.getNodeGraphs())} node graphs created")
    
    # Test export
    result = export_json_document(doc, "test_output.json")
    print(f"Export test: {'SUCCESS' if result else 'FAILED'}")
    
    # Cleanup
    try:
        Path(test_file).unlink()
        Path("test_output.json").unlink()
        print("Cleaned up test files")
    except:
        pass
