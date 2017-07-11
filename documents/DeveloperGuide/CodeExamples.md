# MaterialX Code Examples {#codeexamples}

The following examples demonstrate a handful of common MaterialX operations in C++ and Python.

### Building a MaterialX Document:

#### C++

~~~{.c}
#include <MaterialXFormat/Document.h>

namespace mx = MaterialX;

// Create a document.
mx::DocumentPtr doc = mx::createDocument();

// Create a node graph with a constant color output.
mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
mx::NodePtr constant = nodeGraph->addNode("constant");
constant->setParameterValue("value", mx::Color3(0.5f, 0.5f, 0.5f));
mx::OutputPtr output = nodeGraph->addOutput();
output->setConnectedNode(constant);

// Create a simple shader interface.
mx::NodeDefPtr shader = doc->addNodeDef("shader1", "surfaceshader", "simpleSrf");
mx::InputPtr diffColor = shader->addInput("diffColor", "color3");
mx::InputPtr specColor = shader->addInput("specColor", "color3");
mx::ParameterPtr roughness = shader->addParameter("roughness", "float");

// Create a material that instantiates the shader.
mx::MaterialPtr material = doc->addMaterial();
mx::ShaderRefPtr shaderRef = material->addShaderRef("shaderRef1", "simpleSrf");

// Bind the diffuse color input to the constant color output.
mx::BindInputPtr bindInput = shaderRef->addBindInput("diffColor");
bindInput->setConnectedOutput(output);
~~~

#### Python

~~~{.py}
import MaterialX as mx

# Create a document.
doc = mx.createDocument()

# Create a node graph with a constant color output.
nodeGraph = doc.addNodeGraph()
constant = nodeGraph.addNode('constant')
constant.setParameterValue('value', mx.Color3(0.5, 0.5, 0.5))
output = nodeGraph.addOutput()
output.setConnectedNode(constant)

# Create a simple shader interface.
shader = doc.addNodeDef('shader1', 'surfaceshader', 'simpleSrf')
diffColor = shader.addInput('diffColor', 'color3')
specColor = shader.addInput('specColor', 'color3')
roughness = shader.addParameter('roughness', 'float')

# Create a material that instantiates the shader.
material = doc.addMaterial()
shaderRef = material.addShaderRef('shaderRef1', 'simpleSrf')

# Bind the diffuse color input to the constant color output.
bindInput = shaderRef.addBindInput('diffColor')
bindInput.setConnectedOutput(output)
~~~

### Traversing a Document Tree:

#### C++

~~~{.c}
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

// Read a document from disk.
mx::DocumentPtr doc = mx::createDocument();
mx::readFromXmlFile(doc, "ExampleFile.mtlx");

// Traverse the document tree in depth-first order.
for (mx::ElementPtr elem : doc->traverseTree())
{
    if (elem->isA<mx::Node>("constant"))
    {
        std::cout << "Constant node " << elem->getName() << std::endl;
    }
    else if (elem->isA<mx::Node>("image"))
    {
        std::cout << "Image node " << elem->getName() << std::endl;
    }
}
~~~

#### Python

~~~{.py}
import MaterialX as mx

# Read a document from disk.
doc = mx.createDocument()
mx.readFromXmlFile(doc, "ExampleFile.mtlx")

# Traverse the document tree in depth-first order.
for elem in doc.traverseTree():
    if elem.isA(mx.Node, 'constant'):
        print 'Constant node', elem.getName()
    elif elem.isA(mx.Node, 'image'):
        print 'Image node', elem.getName()
~~~

### Traversing a Dataflow Graph:

#### C++

~~~{.c}
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

// Read a document from disk.
mx::DocumentPtr doc = mx::createDocument();
mx::readFromXmlFile(doc, "ExampleFile.mtlx");

// Iterate through materials.
for (mx::MaterialPtr material : doc->getMaterials())
{
    // Iterate through each referenced shader interface.
    for (mx::NodeDefPtr shaderDef : material->getReferencedShaderDefs())
    {
        // For each shader input, traverse the dataflow graph to its sources.
        for (mx::InputPtr input : shaderDef->getInputs())
        {
            for (mx::Edge edge : input->traverseGraph(material))
            {
                mx::ElementPtr elem = edge.getUpstreamElement();
                if (elem->isA<mx::Node>("constant"))
                {
                    std::cout << "Constant node " << elem->getName() << std::endl;
                }
                else if (elem->isA<mx::Node>("image"))
                {
                    std::cout << "Image node " << elem->getName() << std::endl;
                }
            }
        }
    }
}
~~~

#### Python

~~~{.py}
import MaterialX as mx

# Read a document from disk.
doc = mx.createDocument()
mx.readFromXmlFile(doc, "ExampleFile.mtlx")

# Iterate through materials.
for material in self.doc.getMaterials():

    # Iterate through each referenced shader interface.
    for shaderDef in material.getReferencedShaderDefs():

        # For each shader input, traverse the dataflow graph to its sources.
        for input in shaderDef.getInputs():
            for edge in input.traverseGraph(material):
                elem = edge.getUpstreamElement()
                if elem.isA(mx.Node, 'constant'):
                    print 'Constant node', elem.getName()
                elif elem.isA(mx.Node, 'image'):
                    print 'Image node', elem.getName()
~~~
