//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("Element cast ownership", "[element]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::ElementPtr element = doc->addNode("constant", "node", "float");
    mx::NodePtr node = element->asA<mx::Node>();
    mx::ConstElementPtr constElement = element;
    mx::ConstNodePtr constNode = constElement->asA<mx::Node>();
    REQUIRE(node.get() == element.get());
    REQUIRE(constNode.get() == node.get());
    REQUIRE_FALSE(element.owner_before(node));
    REQUIRE_FALSE(node.owner_before(element));
    REQUIRE_FALSE(element.owner_before(constNode));
    REQUIRE_FALSE(constNode.owner_before(element));
    REQUIRE(node->asA<mx::Element>() == element);
    REQUIRE(node->asA<mx::InterfaceElement>() == element);
    REQUIRE_FALSE(element->asA<mx::NodeGraph>());
    REQUIRE_FALSE(constElement->asA<mx::NodeGraph>());

    std::weak_ptr<mx::Element> lifetime = element;
    doc->removeChild("node");
    doc.reset();
    element.reset();
    constElement.reset();
    node.reset();
    REQUIRE_FALSE(lifetime.expired());
    REQUIRE(constNode->asA<mx::Node>() == constNode);
    REQUIRE_FALSE(constNode->asA<mx::NodeGraph>());
    constNode.reset();
    REQUIRE(lifetime.expired());
}
