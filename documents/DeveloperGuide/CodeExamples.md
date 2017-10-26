# MaterialX Code Examples {#codeexamples}

The following examples demonstrate a handful of common MaterialX operations in C++ and Python.

### Building a MaterialX Document:

#### C++

~~~{.c}
#include <MaterialXFormat/Document.h>

namespace mx = MaterialX;

// Create a document.
mx::DocumentPtr doc = mx::createDocument();

// Create a node graph with a single image node and output.
mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
mx::NodePtr image = nodeGraph->addNode("image");
image->setParameterValue("file", "image1.tif", "filename");
mx::OutputPtr output = nodeGraph->addOutput();
output->setConnectedNode(image);

// Create a simple shader interface.
mx::NodeDefPtr shader = doc->addNodeDef("shader1", "surfaceshader", "simpleSrf");
mx::InputPtr diffColor = shader->setInputValue("diffColor", mx::Color3(1.0));
mx::InputPtr specColor = shader->setInputValue("specColor", mx::Color3(0.0));
mx::ParameterPtr roughness = shader->setParameterValue("roughness", 0.25f);

// Create a material that instantiates the shader.
mx::MaterialPtr material = doc->addMaterial();
mx::ShaderRefPtr shaderRef = material->addShaderRef("shaderRef1", "simpleSrf");

// Bind roughness to a new value within this material.
mx::BindParamPtr bindParam = shaderRef->addBindParam("roughness");
bindParam->setValue(0.5f);

// Display the value of roughness in the context of this material.
cout << roughness->getBoundValue(material)->getValueString();
~~~

#### Python

~~~{.py}
import MaterialX as mx

# Create a document.
doc = mx.createDocument()

# Create a node graph with a single image node and output.
nodeGraph = doc.addNodeGraph()
image = nodeGraph.addNode('image')
image.setParameterValue('file', 'image1.tif', 'filename')
output = nodeGraph.addOutput()
output.setConnectedNode(image)

# Create a simple shader interface.
shader = doc.addNodeDef('shader1', 'surfaceshader', 'simpleSrf')
diffColor = shader.setInputValue('diffColor', mx.Color3(1.0))
specColor = shader.setInputValue('specColor', mx.Color3(0.0))
roughness = shader.setParameterValue('roughness', 0.25)

# Create a material that instantiates the shader.
material = doc.addMaterial()
shaderRef = material.addShaderRef('shaderRef1', 'simpleSrf')

# Bind roughness to a new value within this material.
bindParam = shaderRef.addBindParam('roughness')
bindParam.setValue(0.5)

# Display the value of roughness in the context of this material.
print str(roughness.getBoundValue(material))
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
    // Display the filename of each image node.
    if (elem->isA<mx::Node>("image"))
    {
        mx::ParameterPtr param = elem->asA<mx::Node>()->getParameter("file");
        if (param)
        {
            string filename = param->getValueString();
            cout << "Image node " << elem->getName() <<
                    " references " << filename << endl;
        }
    }
}
~~~

#### Python

~~~{.py}
import MaterialX as mx

# Read a document from disk.
doc = mx.createDocument()
mx.readFromXmlFile(doc, 'ExampleFile.mtlx')

# Traverse the document tree in depth-first order.
for elem in doc.traverseTree():

    # Display the filename of each image node.
    if elem.isA(mx.Node, 'image'):
        param = elem.getParameter('file')
        if param:
            filename = param.getValueString()
            print 'Image node', elem.getName(), 'references', filename
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
    // For each shader parameter, compute its value in the context of this material.
    for (mx::ParameterPtr param : material->getPrimaryShaderParameters())
    {
        mx::ValuePtr value = param->getBoundValue(material);
        if (value)
        {
            cout << "Parameter " << param->getName() <<
                    " has bound value ", value->getValueString() << endl;
        }
    }

    // For each shader input, find all upstream images in the dataflow graph.
    for (mx::InputPtr input : material->getPrimaryShaderInputs())
    {
        for (mx::Edge edge : input->traverseGraph(material))
        {
            mx::ElementPtr elem = edge.getUpstreamElement();
            if (elem->isA<mx::Node>("image"))
            {
                cout << "Input " << input->getName() <<
                        " has upstream image node " << elem->getName() << endl;
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
mx.readFromXmlFile(doc, 'ExampleFile.mtlx')

# Iterate through materials.
for material in doc.getMaterials():

    # For each shader parameter, compute its value in the context of this material.
    for param in material.getPrimaryShaderParameters():
        value = param.getBoundValue(material)
        print 'Parameter', param.getName(), 'has value', str(value)

    # For each shader input, find all upstream images in the dataflow graph.
    for input in material.getPrimaryShaderInputs():
        for edge in input.traverseGraph(material):
            elem = edge.getUpstreamElement(material)
            if elem.isA(mx.Node, 'image'):
                print 'Input', input.getName(), 'has upstream image node', elem.getName()
~~~
