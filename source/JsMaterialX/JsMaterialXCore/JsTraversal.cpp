#include "../helpers.h"
#include <MaterialXCore/Traversal.h>

#include <MaterialXCore/Material.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(traversal)
    {
        ems::class_<mx::Edge>("Edge")
            .smart_ptr_constructor("Edge", &std::make_shared<mx::Edge, mx::ElementPtr, mx::ElementPtr, mx::ElementPtr>)
            .function("getDownstreamElement", &mx::Edge::getDownstreamElement)
            .function("getConnectingElement", &mx::Edge::getConnectingElement)
            .function("getUpstreamElement", &mx::Edge::getUpstreamElement)
            .function("getName", &mx::Edge::getName);

        ems::class_<mx::TreeIterator>("TreeIterator")
            .smart_ptr_constructor("TreeIterator", &std::make_shared<mx::TreeIterator, mx::ElementPtr>)
            .function("getElement", &mx::TreeIterator::getElement)
            .function("getElementDepth", &mx::TreeIterator::getElementDepth)
            .function("setPruneSubtree", &mx::TreeIterator::setPruneSubtree)
            .function("getPruneSubtree", &mx::TreeIterator::getPruneSubtree)
            .function("__iter__", ems::optional_override([](mx::TreeIterator &it) -> mx::TreeIterator & {
                return it.begin(1);
            }))
            .function("next", ems::optional_override([](mx::TreeIterator &it) {
                if (++it == it.end())
                    throw mx::Exception("Could not get the next element.");
                return *it;
            }));

        ems::class_<mx::GraphIterator>("GraphIterator")
            .smart_ptr_constructor("GraphIterator", &std::make_shared<mx::GraphIterator, mx::ElementPtr>)
            .function("getDownstreamElement", &mx::GraphIterator::getDownstreamElement)
            .function("getConnectingElement", &mx::GraphIterator::getConnectingElement)
            .function("getUpstreamElement", &mx::GraphIterator::getUpstreamElement)
            .function("getUpstreamIndex", &mx::GraphIterator::getUpstreamIndex)
            .function("getElementDepth", &mx::GraphIterator::getElementDepth)
            .function("getNodeDepth", &mx::GraphIterator::getNodeDepth)
            .function("setPruneSubgraph", &mx::GraphIterator::setPruneSubgraph)
            .function("getPruneSubgraph", &mx::GraphIterator::getPruneSubgraph)
            .function("__iter__", ems::optional_override([](mx::GraphIterator &it) -> mx::GraphIterator & {
                return it.begin(1);
            }))
            .function("next", ems::optional_override([](mx::GraphIterator &it) {
                if (++it == it.end())
                    throw mx::Exception("Could not get the next element.");
                    
                return *it;
            }));

        ems::class_<mx::InheritanceIterator>("InheritanceIterator")
            .smart_ptr_constructor("InheritanceIterator", &std::make_shared<mx::InheritanceIterator, mx::ConstElementPtr>)

            .function("__iter__", ems::optional_override([](mx::InheritanceIterator &it) -> mx::InheritanceIterator & {
                return it.begin(1);
            }))
            .function("next", ems::optional_override([](mx::InheritanceIterator &it) {
                if (++it == it.end())
                    throw mx::Exception("Could not get the next element.");
                return *it;
            }));
    }
}