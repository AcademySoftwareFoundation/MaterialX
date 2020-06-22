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
mx::NodeDefPtr simpleSrf = doc->addNodeDef("ND_simpleSrf", "surfaceshader", "simpleSrf");
mx::InputPtr diffColor = simpleSrf->setInputValue("diffColor", mx::Color3(1.0));
mx::InputPtr specColor = simpleSrf->setInputValue("specColor", mx::Color3(0.0));
mx::ParameterPtr roughness = simpleSrf->setParameterValue("roughness", 0.25f);

// Create a material that instantiates the shader.
mx::MaterialPtr material = doc->addMaterial();
mx::ShaderRefPtr refSimpleSrf = material->addShaderRef("SR_simpleSrf", "simpleSrf");

// Bind roughness to a new value within this material.
mx::BindParamPtr bindParam = refSimpleSrf->addBindParam("roughness");
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
simpleSrf = doc.addNodeDef('ND_simpleSrf', 'surfaceshader', 'simpleSrf')
diffColor = simpleSrf.setInputValue('diffColor', mx.Color3(1.0))
specColor = simpleSrf.setInputValue('specColor', mx.Color3(0.0))
roughness = simpleSrf.setParameterValue('roughness', 0.25)

# Create a material that instantiates the shader.
material = doc.addMaterial()
refSimpleSrf = material.addShaderRef('SR_simpleSrf', 'simpleSrf')

# Bind roughness to a new value within this material.
bindParam = refSimpleSrf.addBindParam('roughness')
bindParam.setValue(0.5)

# Display the value of roughness in the context of this material.
print str(roughness.getBoundValue(material))
~~~

#### JavaScript

~~~{.js}
import Module from './JsMaterialX.js';

Module().then((_module) => {
    // Get the MaterialX namespace.
    const mx = _module.getMaterialX();

    // Create a document.
    const doc = mx.createDocument();

    // Create a node graph with a single image node and output.
    const nodeGraph = doc.addNodeGraph();
    const image = nodeGraph.addNode('image');
    image.setParameterValuestring('file', 'image1.tif', 'filename');
    const output = nodeGraph.addOutput();
    output.setConnectedNode(image);

    // Create a simple shader interface.
    const simpleSrf = doc.addNodeDef('ND_simpleSrf', 'surfaceshader', 'simpleSrf');
    const diffColor = simpleSrf.setInputValuecolor3('diffColor', new mx.Color3(1.0, 1.0, 1.0));
    const specColor = simpleSrf.setInputValuecolor3('specColor', new mx.Color3(0.0, 0.0, 0.0));
    const roughness = simpleSrf.setParameterValuefloat('roughness', 0.25);

    // Create a material that instantiates the shader.
    const material = doc.addMaterial();
    const refSimpleSrf = material.addShaderRef('SR_simpleSrf', 'simpleSrf');

    // Bind roughness to a new value within this material.
    const bindParam = refSimpleSrf.addBindParam('roughness');
    bindParam.setValuefloat(0.5);

    // Display the value of roughness in the context of this material.
    console.log(roughness.getBoundValue(material).getValueString());

});

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

#### JavaScript

~~~{.js}
import Module from './JsMaterialX.js';

Module().then((_module) => {
    // Get the MaterialX namespace.
    const mx = _module.getMaterialX();

    // Read a document from disk.
    const doc = mx.createDocument();
    // Note: The xmlStr should be defined.
    mx.readFromXmlString(doc, xmlStr);

    // Traverse the document tree in depth-first order.
    const elements = doc.traverseTree();
    let elem = elements.next();
    while(elem) {                
        // Display the filename of each image node.
        if (elem instanceof mx.Node) {
            const param = elem.getParameter('file');
            if (param) {
                filename = param.getValueString();
                console.log('Image node ' + elem.getName() + ' references ' + filename);
            }
        }
        elem = elements.next();
    }
}
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

#### JavaScript

~~~{.js}
import Module from './JsMaterialX.js';

Module().then((_module) => {
    // Get the MaterialX namespace.
    const mx = _module.getMaterialX();

    // Read a document from disk.
    const doc = mx.createDocument();
    // Note: The xmlStr should be defined.
    mx.readFromXmlString(doc, xmlStr);

    // Iterate through materials.
    const materials = doc.getMaterials();
    materials.forEach((material) => {
        // For each shader parameter, compute its value in the context of this material.
        const primaryShaderParams = material.getPrimaryShaderParameters();
        primaryShaderParams.forEach((param) => {
            const value = param.getBoundValue(material);
            console.log('Parameter', param.getName(), 'has value', value.getData());
        });

        // For each shader input, find all upstream images in the dataflow graph.
        const primaryShaderInputs = material.getPrimaryShaderInputs();
        primaryShaderInputs.forEach((input) => {
            const graphIter = input.traverseGraph(material);
            let edge = graphIter.next();
            while (edge) {
                const elem = edge.getUpstreamElement(material)
                if (elem instanceof mx.Node) {
                    console.log('Input', input.getName(), 'has upstream image node', elem.getName());
                }
                edge = graphIter.next();
            }
        });
    });
}
~~~