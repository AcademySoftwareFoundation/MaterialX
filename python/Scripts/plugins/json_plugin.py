"""
Example JSON Plugin for MaterialX Document Import/Export

This plugin demonstrates how to create document loaders for custom formats.
It provides JSON import/export capabilities for MaterialX documents.

The plugin is automatically discovered by the MaterialX plugin system when
placed in a "plugins" directory.
"""

import json
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
from typing import Dict, Any, Optional

# Import the plugin system components
try:
    from plugin_manager import get_plugin_manager, document_loader, create_document_loader, hookimpl, MaterialXHookSpec
except ImportError:
    # If running from a different location, try relative import
    import sys
    import os
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from plugin_manager import get_plugin_manager, document_loader, create_document_loader, hookimpl, MaterialXHookSpec


class JSONDocumentPlugin:
    """
    Example plugin that handles JSON import/export for MaterialX documents.
    
    This demonstrates the plugin pattern using the pluggy framework.
    """
    
    @hookimpl
    def register_document_loaders(self):
        """Hook implementation to register our document loaders."""
        
        # Create JSON import loader
        json_importer = create_document_loader(
            identifier="json_importer",
            name="JSON Document Importer",
            description="Import MaterialX documents from JSON format",
            extensions=[".json"],
            import_func=self.import_json_document,
            version="1.0.0"
        )
        
        # Create JSON export loader  
        json_exporter = create_document_loader(
            identifier="json_exporter", 
            name="JSON Document Exporter",
            description="Export MaterialX documents to JSON format",
            extensions=[".json"],
            export_func=self.export_json_document,
            version="1.0.0"
        )
        
        print("JSON Plugin: Registered JSON import/export loaders")
        return [json_importer, json_exporter]
    
    def import_json_document(self, uri: str) -> Optional[mx.Document]:
        """
        Import a MaterialX document from JSON format.
        
        Args:
            uri: Path to the JSON file to import
            
        Returns:
            MaterialX Document object or None on failure
        """
        print(f"JSON Plugin: Importing document from {uri}")
        
        try:
            with open(uri, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            # Create a new MaterialX document
            doc = mx.createDocument()
            doc.setSourceUri(uri)
            
            # Parse the JSON structure and populate the document
            self._parse_json_to_document(data, doc)
            
            print(f"JSON Plugin: Successfully imported document from {uri}")
            return doc
            
        except Exception as e:
            print(f"JSON Plugin: Error importing {uri}: {e}")
            return None
    
    def export_json_document(self, document: mx.Document, uri: str) -> bool:
        """
        Export a MaterialX document to JSON format.
        
        Args:
            document: The MaterialX document to export
            uri: Path where to save the JSON file
            
        Returns:
            True on success, False on failure
        """
        print(f"JSON Plugin: Exporting document to {uri}")
        
        try:
            # Convert document to JSON structure
            data = self._document_to_json(document)
            
            # Write to file
            with open(uri, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            
            print(f"JSON Plugin: Successfully exported document to {uri}")
            return True
            
        except Exception as e:
            print(f"JSON Plugin: Error exporting to {uri}: {e}")
            return False
    
    def _parse_json_to_document(self, data: Dict[str, Any], doc: mx.Document) -> None:
        """Parse JSON data and populate MaterialX document."""
        
        # Set document metadata
        if 'metadata' in data:
            metadata = data['metadata']
            if 'version' in metadata:
                doc.setVersionString(metadata['version'])
            if 'colorspace' in metadata:
                doc.setColorSpace(metadata['colorspace'])
        
        # Add node graphs
        if 'nodegraphs' in data:
            for ng_data in data['nodegraphs']:
                self._create_nodegraph_from_json(ng_data, doc)
        
        # Add materials
        if 'materials' in data:
            for mat_data in data['materials']:
                self._create_material_from_json(mat_data, doc)
        
        # Add other elements as needed
        print(f"JSON Plugin: Parsed document with {len(doc.getNodeGraphs())} node graphs and {len(doc.getMaterials())} materials")
    
    def _create_nodegraph_from_json(self, ng_data: Dict[str, Any], doc: mx.Document) -> None:
        """Create a node graph from JSON data."""
        
        name = ng_data.get('name', 'unnamed_nodegraph')
        nodegraph = doc.addNodeGraph(name)
        
        # Set attributes
        if 'type' in ng_data:
            nodegraph.setType(ng_data['type'])
        
        # Add nodes
        if 'nodes' in ng_data:
            for node_data in ng_data['nodes']:
                self._create_node_from_json(node_data, nodegraph)
    
    def _create_node_from_json(self, node_data: Dict[str, Any], parent) -> None:
        """Create a node from JSON data."""
        
        name = node_data.get('name', 'unnamed_node')
        category = node_data.get('category', 'constant')
        
        node = parent.addNode(category, name)
        
        # Set node type
        if 'type' in node_data:
            node.setType(node_data['type'])
        
        # Set inputs
        if 'inputs' in node_data:
            for input_name, input_data in node_data['inputs'].items():
                if isinstance(input_data, dict):
                    # Complex input with type/value
                    input_elem = node.addInput(input_name, input_data.get('type', 'string'))
                    if 'value' in input_data:
                        input_elem.setValueString(str(input_data['value']))
                else:
                    # Simple value
                    node.setInputValue(input_name, input_data)
    
    def _create_material_from_json(self, mat_data: Dict[str, Any], doc: mx.Document) -> None:
        """Create a material from JSON data."""
        
        name = mat_data.get('name', 'unnamed_material')
        material = doc.addMaterial(name)
        
        # Add shader refs
        if 'shaderrefs' in mat_data:
            for ref_data in mat_data['shaderrefs']:
                ref_name = ref_data.get('name', 'shader_ref')
                node_ref = ref_data.get('node', '')
                shader_ref = material.addShaderRef(ref_name, node_ref)
                
                # Set bindings
                if 'bindings' in ref_data:
                    for binding_data in ref_data['bindings']:
                        binding = shader_ref.addBindInput(
                            binding_data.get('name', ''),
                            binding_data.get('type', 'string')
                        )
                        if 'value' in binding_data:
                            binding.setValueString(str(binding_data['value']))
    
    def _document_to_json(self, doc: mx.Document) -> Dict[str, Any]:
        """Convert MaterialX document to JSON structure."""
        
        data = {
            'metadata': {
                'version': doc.getVersionString(),
                'colorspace': doc.getColorSpace(),
                'exported_by': 'MaterialX JSON Plugin v1.0.0'
            },
            'nodegraphs': [],
            'materials': []
        }
        
        # Export node graphs
        for nodegraph in doc.getNodeGraphs():
            ng_data = self._nodegraph_to_json(nodegraph)
            data['nodegraphs'].append(ng_data)
        
        # Export materials
        for material in doc.getMaterials():
            mat_data = self._material_to_json(material)
            data['materials'].append(mat_data)
        
        return data
    
    def _nodegraph_to_json(self, nodegraph) -> Dict[str, Any]:
        """Convert node graph to JSON."""
        
        ng_data = {
            'name': nodegraph.getName(),
            'type': nodegraph.getType(),
            'nodes': []
        }
        
        # Export nodes
        for node in nodegraph.getNodes():
            node_data = self._node_to_json(node)
            ng_data['nodes'].append(node_data)
        
        return ng_data
    
    def _node_to_json(self, node) -> Dict[str, Any]:
        """Convert node to JSON."""
        
        node_data = {
            'name': node.getName(),
            'category': node.getCategory(),
            'type': node.getType(),
            'inputs': {}
        }
        
        # Export inputs
        for input_elem in node.getInputs():
            input_data = {
                'type': input_elem.getType(),
                'value': input_elem.getValueString()
            }
            node_data['inputs'][input_elem.getName()] = input_data
        
        return node_data
    
    def _material_to_json(self, material) -> Dict[str, Any]:
        """Convert material to JSON."""
        
        mat_data = {
            'name': material.getName(),
            'shaderrefs': []
        }
        
        # Export shader refs
        for shader_ref in material.getShaderRefs():
            ref_data = {
                'name': shader_ref.getName(),
                'node': shader_ref.getReferencedNode(),
                'bindings': []
            }
            
            # Export bindings
            for binding in shader_ref.getBindInputs():
                binding_data = {
                    'name': binding.getName(),
                    'type': binding.getType(),
                    'value': binding.getValueString()
                }
                ref_data['bindings'].append(binding_data)
            
            mat_data['shaderrefs'].append(ref_data)
        
        return mat_data


# Alternative decorator-based approach (simpler for single loaders)
@document_loader(
    identifier="simple_json_loader",
    name="Simple JSON Loader", 
    description="Simple JSON document loader using decorator",
    extensions=[".json"],
    can_import=True,
    version="1.0.0"
)
def simple_json_import(uri: str) -> mx.Document:
    """
    Simple JSON importer using the decorator approach.
    
    This is an alternative to the plugin class approach above.
    Both methods work - choose based on your needs.
    """
    print(f"Simple JSON Loader: Importing {uri}")
    
    try:
        with open(uri, 'r') as f:
            data = json.load(f)
        
        # Create a simple document
        doc = mx.createDocument()
        doc.setSourceUri(uri)
        
        # Add a simple node graph with the JSON data as metadata
        nodegraph = doc.addNodeGraph("imported_data")
        
        # Create a constant node with some data from JSON
        if 'title' in data:
            node = nodegraph.addNode("constant", "title_node")
            node.setInputValue("value", str(data['title']))
        
        print(f"Simple JSON Loader: Successfully imported {uri}")
        return doc
        
    except Exception as e:
        print(f"Simple JSON Loader: Error importing {uri}: {e}")
        # Return an empty document rather than None
        doc = mx.createDocument()
        doc.setSourceUri(uri)
        return doc


# Plugin registration function (called by plugin system)
def register_plugin():
    """
    Register this plugin with the MaterialX plugin system.
    
    This function is called automatically when the plugin is discovered.
    """
    print("JSON Plugin: Registering plugin...")
    
    # Create plugin instance
    plugin = JSONDocumentPlugin()
    
    # Register with plugin manager
    pm = get_plugin_manager()
    pm.register_plugin(plugin)
    
    print("JSON Plugin: Registration complete")


# Auto-register when module is imported
if __name__ == "__main__":
    # If run directly, register the plugin
    register_plugin()
else:
    # If imported, auto-register
    try:
        register_plugin()
    except Exception as e:
        print(f"JSON Plugin: Error during auto-registration: {e}")


# Example usage and testing
def test_json_plugin():
    """Test the JSON plugin functionality."""
    
    # Create a test document
    doc = mx.createDocument()
    doc.setVersionString("1.38")
    doc.setColorSpace("lin_rec709")
    
    # Add a simple node graph
    nodegraph = doc.addNodeGraph("test_graph")
    node = nodegraph.addNode("constant", "test_node")
    node.setType("color3")
    node.setInputValue("value", mx.Color3(1.0, 0.5, 0.0))
    
    # Test export
    plugin = JSONDocumentPlugin()
    success = plugin.export_json_document(doc, "test_export.json")
    print(f"Export test: {'SUCCESS' if success else 'FAILED'}")
    
    if success:
        # Test import
        imported_doc = plugin.import_json_document("test_export.json")
        print(f"Import test: {'SUCCESS' if imported_doc else 'FAILED'}")
        
        if imported_doc:
            print(f"Imported document has {len(imported_doc.getNodeGraphs())} node graphs")


if __name__ == "__main__":
    # Run tests if executed directly
    print("Running JSON Plugin tests...")
    test_json_plugin()
