//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXRender/PluginManager.h>
#include <MaterialXCore/Document.h>

#include <iostream>

namespace mx = MaterialX;

// Test document loader implementation
class TestDocumentLoader : public mx::DocumentLoader
{
  public:
    TestDocumentLoader(const std::string& identifier, const std::string& name, const std::string& description) :
        mx::DocumentLoader(identifier, name, description)
    {
    }

    mx::StringSet supportedExtensions() const override
    {
        return { ".test", ".testloader" };
    }

    mx::DocumentPtr importDocument(const std::string& uri) override
    {
        _lastImportUri = uri;
        auto doc = mx::createDocument();
        doc->setSourceUri(uri);
        
        // Add a simple node to verify the document was created by this loader
        auto nodeGraph = doc->addNodeGraph("test_graph");
        auto node = nodeGraph->addNode("constant", "test_node");
        node->setInputValue("value", 1.0f);
        
        return doc;
    }

    bool exportDocument(mx::ConstDocumentPtr document, const std::string& uri) override
    {
        _lastExportUri = uri;
        _lastExportDocument = document;
        return true; // Simulate successful export
    }

    // Test accessors
    const std::string& getLastImportUri() const { return _lastImportUri; }
    const std::string& getLastExportUri() const { return _lastExportUri; }
    mx::ConstDocumentPtr getLastExportDocument() const { return _lastExportDocument; }

  private:
    std::string _lastImportUri;
    std::string _lastExportUri;
    mx::ConstDocumentPtr _lastExportDocument;
};

// Test callback for registration events
class TestRegistrationCallback
{
  public:
    void operator()(const std::string& identifier, bool registered)
    {
        REQUIRE(!identifier.empty());
        std::cout << "Registration event: " << identifier << " - " << (registered ? "Registered" : "Unregistered") << std::endl;
        events.push_back({identifier, registered});
    }

    struct Event
    {
        std::string identifier;
        bool registered;
    };

    std::vector<Event> events;
};

TEST_CASE("PluginManager", "[pluginmanager]")
{
    SECTION("Singleton behavior")
    {
        auto& manager1 = mx::PluginManager::getInstance();
        auto& manager2 = mx::PluginManager::getInstance();
        
        // Should return the same instance
        REQUIRE(&manager1 == &manager2);
    }

    SECTION("Document loader registration and unregistration")
    {
        auto& manager = mx::PluginManager::getInstance();
        
        // Create test loader
        auto loader = std::make_shared<TestDocumentLoader>("test_loader", "Test Loader", "A test document loader");
        
        SECTION("Register valid loader")
        {
            bool result = manager.registerDocumentLoader(loader);
            REQUIRE(result == true);
            
            // Verify loader was registered by attempting to unregister
            bool unregisterResult = manager.unregisterDocumentLoader("test_loader");
            REQUIRE(unregisterResult == true);
        }
        
        SECTION("Register null loader")
        {
            bool result = manager.registerDocumentLoader(nullptr);
            REQUIRE(result == false);
        }
        
        SECTION("Register loader with empty identifier")
        {
            auto emptyLoader = std::make_shared<TestDocumentLoader>("", "Empty Loader", "Loader with empty identifier");
            bool result = manager.registerDocumentLoader(emptyLoader);
            REQUIRE(result == false);
        }
        
        SECTION("Unregister non-existent loader")
        {
            bool result = manager.unregisterDocumentLoader("non_existent_loader");
            REQUIRE(result == false);
        }
        
        SECTION("Unregister with empty identifier")
        {
            bool result = manager.unregisterDocumentLoader("");
            REQUIRE(result == false);
        }
    }

    SECTION("Document import and export")
    {
        auto& manager = mx::PluginManager::getInstance();
        
        // Register test loader
        auto loader = std::make_shared<TestDocumentLoader>("test_loader", "Test Loader", "A test document loader");
        manager.registerDocumentLoader(loader);
        
        SECTION("Import document")
        {
            std::string testUri = "test://example.test";
            auto document = manager.importDocument(testUri);
            
            REQUIRE(document != nullptr);
            REQUIRE(document->getSourceUri() == testUri);
            
            // Verify the document was created by our test loader
            auto nodeGraph = document->getNodeGraph("test_graph");
            REQUIRE(nodeGraph != nullptr);
            auto node = nodeGraph->getNode("test_node");
            REQUIRE(node != nullptr);
            REQUIRE(node->getCategory() == "constant");
            
            // Verify our loader was called
            REQUIRE(loader->getLastImportUri() == testUri);
        }
        
        SECTION("Export document")
        {
            auto document = mx::createDocument();
            std::string testUri = "test://export.test";
            
            bool result = manager.exportDocument(document, testUri);
            REQUIRE(result == true);
            
            // Verify our loader was called
            REQUIRE(loader->getLastExportUri() == testUri);
            REQUIRE(loader->getLastExportDocument() == document);
        }
        
        SECTION("Export null document")
        {
            bool result = manager.exportDocument(nullptr, "test://export.test");
            REQUIRE(result == false);
        }
        
        SECTION("Export with empty URI")
        {
            auto document = mx::createDocument();
            bool result = manager.exportDocument(document, "");
            REQUIRE(result == false);
        }
        
        // Clean up
        manager.unregisterDocumentLoader("test_loader");
    }    SECTION("Registration callback")
    {
        auto& manager = mx::PluginManager::getInstance();
        TestRegistrationCallback callback;
        
        // Add the callback with an identifier
        bool callbackAdded = manager.addRegistrationCallback("test_callback", [&callback](const std::string& id, bool registered) {
            callback(id, registered);
        });
        REQUIRE(callbackAdded == true);
        
        SECTION("Callback on registration")
        {
            auto loader = std::make_shared<TestDocumentLoader>("callback_test", "Callback Test", "Test callback functionality");
            
            callback.events.clear();
            manager.registerDocumentLoader(loader);
            
            REQUIRE(callback.events.size() == 1);
            if (!callback.events.empty())
            {
                REQUIRE(callback.events[0].identifier == "callback_test");
                REQUIRE(callback.events[0].registered == true);
            }
            
            // Test unregistration callback
            manager.unregisterDocumentLoader("callback_test");
            
            REQUIRE(callback.events.size() == 2);
            if (callback.events.size() >= 2)
            {
                REQUIRE(callback.events[1].identifier == "callback_test");
                REQUIRE(callback.events[1].registered == false);
            }
        }
        
        SECTION("No callback on failed registration")
        {
            callback.events.clear();
            
            // Try to register null loader (should fail)
            manager.registerDocumentLoader(nullptr);
            
            // No callback should be triggered
            REQUIRE(callback.events.size() == 0);
        }
          // Clean up callback
        bool callbackRemoved = manager.removeRegistrationCallback("test_callback");
        REQUIRE(callbackRemoved == true);
    }

    SECTION("Multiple registration callbacks")
    {
        auto& manager = mx::PluginManager::getInstance();
        
        TestRegistrationCallback callback1, callback2, callback3;
        
        // Add multiple callbacks
        bool added1 = manager.addRegistrationCallback("callback1", [&callback1](const std::string& id, bool registered) {
            callback1(id, registered);
        });
        bool added2 = manager.addRegistrationCallback("callback2", [&callback2](const std::string& id, bool registered) {
            callback2(id, registered);
        });
        bool added3 = manager.addRegistrationCallback("callback3", [&callback3](const std::string& id, bool registered) {
            callback3(id, registered);
        });
        
        REQUIRE(added1 == true);
        REQUIRE(added2 == true);
        REQUIRE(added3 == true);
        
        // Test all callbacks are triggered
        auto loader = std::make_shared<TestDocumentLoader>("multi_callback_test", "Multi Callback Test", "Test multiple callbacks");
        
        callback1.events.clear();
        callback2.events.clear();
        callback3.events.clear();
        
        manager.registerDocumentLoader(loader);
        
        // All callbacks should be triggered
        REQUIRE(callback1.events.size() == 1);
        REQUIRE(callback2.events.size() == 1);
        REQUIRE(callback3.events.size() == 1);
        
        // Remove one callback
        bool removed2 = manager.removeRegistrationCallback("callback2");
        REQUIRE(removed2 == true);
        
        // Test unregistration - only callbacks 1 and 3 should be triggered
        manager.unregisterDocumentLoader("multi_callback_test");
        
        REQUIRE(callback1.events.size() == 2);
        REQUIRE(callback2.events.size() == 1); // Should not have increased
        REQUIRE(callback3.events.size() == 2);
        
        // Clean up remaining callbacks
        manager.removeRegistrationCallback("callback1");
        manager.removeRegistrationCallback("callback3");
    }

    SECTION("Callback edge cases")
    {
        auto& manager = mx::PluginManager::getInstance();
        TestRegistrationCallback callback;
        
        SECTION("Add callback with empty identifier")
        {
            bool result = manager.addRegistrationCallback("", [&callback](const std::string& id, bool registered) {
                callback(id, registered);
            });
            REQUIRE(result == false);
        }
        
        SECTION("Add callback with null function")
        {
            bool result = manager.addRegistrationCallback("test_null", nullptr);
            REQUIRE(result == false);
        }
        
        SECTION("Remove non-existent callback")
        {
            bool result = manager.removeRegistrationCallback("non_existent");
            REQUIRE(result == false);
        }
        
        SECTION("Remove callback with empty identifier")
        {
            bool result = manager.removeRegistrationCallback("");
            REQUIRE(result == false);
        }
        
        SECTION("Replace callback with same identifier")
        {
            TestRegistrationCallback callback2;
            
            // Add first callback
            bool added1 = manager.addRegistrationCallback("replace_test", [&callback](const std::string& id, bool registered) {
                callback(id, registered);
            });
            REQUIRE(added1 == true);
            
            // Replace with second callback
            bool added2 = manager.addRegistrationCallback("replace_test", [&callback2](const std::string& id, bool registered) {
                callback2(id, registered);
            });
            REQUIRE(added2 == true);
            
            // Test that only the second callback is called
            auto loader = std::make_shared<TestDocumentLoader>("replace_test_loader", "Replace Test", "Test callback replacement");
            
            callback.events.clear();
            callback2.events.clear();
            
            manager.registerDocumentLoader(loader);
            
            REQUIRE(callback.events.size() == 0);  // Original callback should not be called
            REQUIRE(callback2.events.size() == 1); // Replacement callback should be called
            
            // Clean up
            manager.unregisterDocumentLoader("replace_test_loader");
            manager.removeRegistrationCallback("replace_test");
        }
    }

    SECTION("Multiple loaders with different extensions")
    {
        auto& manager = mx::PluginManager::getInstance();
        
        auto loader1 = std::make_shared<TestDocumentLoader>("loader1", "Loader 1", "First test loader");
        auto loader2 = std::make_shared<TestDocumentLoader>("loader2", "Loader 2", "Second test loader");
        
        // Register both loaders
        REQUIRE(manager.registerDocumentLoader(loader1) == true);
        REQUIRE(manager.registerDocumentLoader(loader2) == true);
        
        // Test that both can import
        auto doc1 = manager.importDocument("file1.test");
        auto doc2 = manager.importDocument("file2.testloader");
        
        REQUIRE(doc1 != nullptr);
        REQUIRE(doc2 != nullptr);
        
        // Clean up
        manager.unregisterDocumentLoader("loader1");
        manager.unregisterDocumentLoader("loader2");
    }

    SECTION("Stress test - multiple registrations and unregistrations")
    {
        auto& manager = mx::PluginManager::getInstance();
        
        const int numLoaders = 100;
        std::vector<std::shared_ptr<TestDocumentLoader>> loaders;
        
        // Register many loaders
        for (int i = 0; i < numLoaders; ++i)
        {
            std::string id = "stress_loader_" + std::to_string(i);
            auto loader = std::make_shared<TestDocumentLoader>(id, "Stress Loader " + std::to_string(i), "Stress test loader");
            loaders.push_back(loader);
            
            bool result = manager.registerDocumentLoader(loader);
            REQUIRE(result == true);
        }
        
        // Unregister all loaders
        for (int i = 0; i < numLoaders; ++i)
        {
            std::string id = "stress_loader_" + std::to_string(i);
            bool result = manager.unregisterDocumentLoader(id);
            REQUIRE(result == true);
        }
    }
}
