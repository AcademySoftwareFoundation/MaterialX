//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Observer.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Observer", "[observer]")
{
    class DocObserver : public mx::Observer
    {
      public:
        DocObserver() :
            _beginUpdateCount(0),
            _endUpdateCount(0),
            _addElementCount(0),
            _removeElementCount(0),
            _setAttributeCount(0),
            _removeAttributeCount(0),
            _initializeCount(0),
            _readCount(0),
            _writeCount(0)
        {
        }

        void onBeginUpdate() override { _beginUpdateCount++; }
        void onEndUpdate() override { _endUpdateCount++; }
        void onAddElement(mx::ElementPtr parent, mx::ElementPtr elem) override { _addElementCount++; }
        void onRemoveElement(mx::ElementPtr parent, mx::ElementPtr elem) override { _removeElementCount++; }
        void onSetAttribute(mx::ElementPtr elem, const std::string&, const std::string&) override { _setAttributeCount++; }
        void onRemoveAttribute(mx::ElementPtr elem, const std::string&) override { _removeAttributeCount++; }
        void onInitialize() override { _initializeCount++; }
        void onRead() override { _readCount++; }
        void onWrite() override { _writeCount++; }

        void clear()
        {
            _beginUpdateCount = 0;
            _endUpdateCount = 0;
            _addElementCount = 0;
            _setAttributeCount = 0;
            _removeElementCount = 0;
            _removeAttributeCount = 0;
            _initializeCount = 0;
            _readCount = 0;
            _writeCount = 0;
        }

        void verifyCountsPreWrite()
        {
            REQUIRE(_beginUpdateCount == 22);
            REQUIRE(_endUpdateCount == 22);
            REQUIRE(_addElementCount == 10);
            REQUIRE(_setAttributeCount == 12);
            REQUIRE(_removeElementCount == 0);
            REQUIRE(_removeAttributeCount == 0);
            REQUIRE(_initializeCount == 0);
            REQUIRE(_readCount == 0);
            REQUIRE(_writeCount == 0);
        }

        void verifyCountsPostRead()
        {
            REQUIRE(_beginUpdateCount == 3);
            REQUIRE(_endUpdateCount == 3);
            REQUIRE(_addElementCount == 10);
            REQUIRE(_setAttributeCount == 13);
            REQUIRE(_removeElementCount == 3);
            REQUIRE(_removeAttributeCount == 1);
            REQUIRE(_initializeCount == 1);
            REQUIRE(_readCount == 1);
            REQUIRE(_writeCount == 1);
        }

        void verifyCountsDisabled()
        {
            REQUIRE(_beginUpdateCount == 0);
            REQUIRE(_endUpdateCount == 0);
            REQUIRE(_addElementCount == 0);
            REQUIRE(_setAttributeCount == 0);
            REQUIRE(_removeElementCount == 0);
            REQUIRE(_removeAttributeCount == 0);
            REQUIRE(_initializeCount == 0);
            REQUIRE(_readCount == 0);
            REQUIRE(_writeCount == 0);
        }

      protected:
        // Set of counts for verification.
        unsigned int _beginUpdateCount;
        unsigned int _endUpdateCount;
        unsigned int _addElementCount;
        unsigned int _removeElementCount;
        unsigned int _setAttributeCount;
        unsigned int _removeAttributeCount;
        unsigned int _initializeCount;
        unsigned int _readCount;
        unsigned int _writeCount;
    };

    // Create an observed document.
    mx::ObservedDocumentPtr doc = mx::Document::createDocument<mx::ObservedDocument>();

    // Register a single observer.
    std::shared_ptr<DocObserver> testObserver = std::make_shared<DocObserver>();
    doc->addObserver("testObserver", testObserver);

    // Create a node graph with a constant color output.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setParameterValue("value", mx::Color3(0.1f, 0.2f, 0.3f));
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Create a simple shader interface.
    mx::NodeDefPtr shader = doc->addNodeDef("shader1", "surfaceshader", "simpleSrf");
    shader->addInput("diffColor", "color3");
    shader->addInput("specColor", "color3");
    shader->addParameter("roughness", "float");

    // Create a material that instantiates the shader.
    mx::MaterialPtr material = doc->addMaterial();
    material->addShaderRef("", "simpleSrf");
    REQUIRE(material->getShaderRefs().size() == 1);

    // Check that observer tracked the correct number of changes of each type
    testObserver->verifyCountsPreWrite();
    testObserver->clear();

    // Serialize and deserialize the document.
    std::string xmlString = mx::writeToXmlString(doc);
    doc->initialize();
    mx::readFromXmlString(doc, xmlString);

    // Check that observer tracked the correct number of changes during initialize and read
    testObserver->verifyCountsPostRead();

    // Check observer callback disabling. All counts should be 0 after being cleared.
    testObserver->clear();
    doc->disableCallbacks();
    doc->initialize();
    mx::readFromXmlString(doc, xmlString);
    testObserver->verifyCountsDisabled();
}
