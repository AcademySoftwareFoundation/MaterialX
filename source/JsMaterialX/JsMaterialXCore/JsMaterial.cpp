//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <JsMaterialX/VectorHelper.h>
#include <JsMaterialX/Helpers.h>

#include <MaterialXCore/Material.h>

#include <emscripten/bind.h>

#include <unordered_set>
#include <vector>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(material)
{
    ems::function("getShaderNodes", ems::optional_override([](mx::NodePtr materialNode) {
        std::vector<mx::NodePtr> set =  mx::getShaderNodes(materialNode);
        // Put all elements into an std::vector
        std::vector<mx::NodePtr> vec;
        vec.insert(vec.end(), set.begin(), set.end());
        return vec;
    }));
    ems::function("getShaderNodes", ems::optional_override([](mx::NodePtr materialNode,
                                                                const std::string& nodeType) {
        std::vector<mx::NodePtr> set = mx::getShaderNodes(materialNode, nodeType);
        // Put all elements into an std::vector
        std::vector<mx::NodePtr> vec;
        vec.insert(vec.end(), set.begin(), set.end());
        return vec;
    }));
    ems::function("getShaderNodes", ems::optional_override([](mx::NodePtr materialNode,
                                                                const std::string& nodeType,
                                                                const std::string& target) {
        std::vector<mx::NodePtr> set = mx::getShaderNodes(materialNode, nodeType, target);
        // Put all elements into an std::vector
        std::vector<mx::NodePtr> vec;
        vec.insert(vec.end(), set.begin(), set.end());
        return vec;
    }));

    ems::function("getConnectedOutputs", &mx::getConnectedOutputs);
}
