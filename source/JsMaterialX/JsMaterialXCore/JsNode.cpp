#include "../helpers.h"
#include <MaterialXCore/Node.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(node)
    {
        ems::class_<mx::Node, ems::base<mx::InterfaceElement>>("Node")
            .smart_ptr_constructor("Node", &std::make_shared<mx::Node, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Node>>("Node")
            .function("setConnectedNode", &mx::Node::setConnectedNode)
            .function("getConnectedNode", &mx::Node::getConnectedNode)
            .function("setConnectedNodeName", &mx::Node::setConnectedNodeName)
            .function("getConnectedNodeName", &mx::Node::getConnectedNodeName)
            .function("getNodeDef", &mx::Node::getNodeDef)
            .function("getImplementation", &mx::Node::getImplementation)
            .function("getDownstreamPorts", &mx::Node::getDownstreamPorts)
            .class_property("CATEGORY", &mx::Node::CATEGORY);

        ems::class_<mx::GraphElement, ems::base<mx::InterfaceElement>>("GraphElement")
            .smart_ptr<std::shared_ptr<mx::GraphElement>>("GraphElement")
            .smart_ptr<std::shared_ptr<const mx::GraphElement>>("GraphElement")
            .function("addNode", &mx::GraphElement::addNode)
            .function("addNodeInstance", &mx::GraphElement::addNodeInstance)
            .function("getNode", &mx::GraphElement::getNode)
            .function("getNodes", &mx::GraphElement::getNodes)
            .function("removeNode", &mx::GraphElement::removeNode)
            .function("addBackdrop", &mx::GraphElement::addBackdrop)
            .function("getBackdrop", &mx::GraphElement::getBackdrop)
            .function("getBackdrops", &mx::GraphElement::getBackdrops)
            .function("removeBackdrop", &mx::GraphElement::removeBackdrop)
            .function("flattenSubgraphs", &mx::GraphElement::flattenSubgraphs)
            .function("topologicalSort", &mx::GraphElement::topologicalSort)
            .function("asStringDot", &mx::GraphElement::asStringDot);

        ems::class_<mx::NodeGraph, ems::base<mx::GraphElement>>("NodeGraph")
            .smart_ptr_constructor("NodeGraph", &std::make_shared<mx::NodeGraph, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::NodeGraph>>("NodeGraph")
            .function("setNodeDef", &mx::NodeGraph::setNodeDef)
            .function("getNodeDef", &mx::NodeGraph::getNodeDef)
            .class_property("CATEGORY", &mx::NodeGraph::CATEGORY);

        ems::class_<mx::Backdrop, ems::base<mx::Element>>("Backdrop")
            .smart_ptr_constructor("Backdrop", &std::make_shared<mx::Backdrop, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Backdrop>>("Backdrop")
            .function("setContainsString", &mx::Backdrop::setContainsString)
            .function("hasContainsString", &mx::Backdrop::hasContainsString)
            .function("getContainsString", &mx::Backdrop::getContainsString)
            .function("setWidth", &mx::Backdrop::setWidth)
            .function("hasWidth", &mx::Backdrop::hasWidth)
            .function("getWidth", &mx::Backdrop::getWidth)
            .function("setHeight", &mx::Backdrop::setHeight)
            .function("hasHeight", &mx::Backdrop::hasHeight)
            .function("getHeight", &mx::Backdrop::getHeight)
            .function("setContainsElements", &mx::Backdrop::setContainsElements)
            .function("getContainsElements", &mx::Backdrop::getContainsElements)
            .class_property("CATEGORY", &mx::Backdrop::CATEGORY)
            .class_property("CONTAINS_ATTRIBUTE", &mx::Backdrop::CONTAINS_ATTRIBUTE)
            .class_property("WIDTH_ATTRIBUTE", &mx::Backdrop::WIDTH_ATTRIBUTE)
            .class_property("HEIGHT_ATTRIBUTE", &mx::Backdrop::HEIGHT_ATTRIBUTE);
    }
}