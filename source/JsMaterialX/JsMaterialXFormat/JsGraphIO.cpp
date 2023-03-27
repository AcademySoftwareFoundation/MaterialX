//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <JsMaterialX/Helpers.h>
#include "./StrContainerTypeRegistration.h"
#include <MaterialXFormat/GraphIo.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(graphio)
{
    ems::enum_<mx::NodeIO::NodeShape>("NodeShape")
        .value("BOX", mx::NodeIO::NodeShape::BOX)
        .value("ROUNDEDBOX", mx::NodeIO::NodeShape::ROUNDEDBOX)
        .value("DIAMOND", mx::NodeIO::NodeShape::DIAMOND);

    ems::enum_<mx::GraphIoGenOptions::Orientation>("GraphOrientation")
        .value("TOP_DOWN", mx::GraphIoGenOptions::Orientation::TOP_DOWN)
        .value("BOTTOM_UP", mx::GraphIoGenOptions::Orientation::BOTTOM_UP)
        .value("LEFT_RIGHT", mx::GraphIoGenOptions::Orientation::LEFT_RIGHT)
        .value("RIGHT_LEFT", mx::GraphIoGenOptions::Orientation::RIGHT_LEFT);

    ems::class_<mx::GraphIoGenOptions>("GraphIoGenOptions")
        .constructor<>()
        .function("setWriteGraphHeader", &mx::GraphIoGenOptions::setWriteGraphHeader)
        .function("getWriteGraphHeader", &mx::GraphIoGenOptions::getWriteGraphHeader)        
        .function("setWriteCategories", &mx::GraphIoGenOptions::setWriteCategories)
        .function("getWriteCategories", &mx::GraphIoGenOptions::getWriteCategories)
        .function("setWriteSubgraphs", &mx::GraphIoGenOptions::setWriteSubgraphs)
        .function("getWriteSubgraphs", &mx::GraphIoGenOptions::getWriteSubgraphs)
        .function("setOrientation", &mx::GraphIoGenOptions::setOrientation)
        .function("getOrientation", &mx::GraphIoGenOptions::getOrientation);

    ems::class_<mx::NodeIO>("NodeIO")
        .constructor<>()
        .property("identifier", &mx::NodeIO::identifier)
        .property("uilabel", &mx::NodeIO::uilabel)
        .property("category", &mx::NodeIO::category)
        .property("group", &mx::NodeIO::group)
        .property("uishape", &mx::NodeIO::uishape);

    ems::class_<mx::GraphIo>("GraphIo")
        .smart_ptr<std::shared_ptr<mx::GraphIo>>("GraphIo")
        .function("write", &mx::GraphIo::write, ems::pure_virtual())
        .function("supportsFormats", &mx::GraphIo::supportsFormats)
        .function("setGenOptions", &mx::GraphIo::setGenOptions)
        .function("getGenOptions", &mx::GraphIo::getGenOptions);

    // Need to register string vector to pass as argument
    ems::register_vector<std::string>("StringVec");

    ems::class_<mx::DotGraphIo, ems::base<mx::GraphIo>>("DotGraphIo")
        .smart_ptr<std::shared_ptr<mx::DotGraphIo>>("DotGraphIo")
        .class_function("create", &mx::DotGraphIo::create)
        .function("write", &mx::DotGraphIo::write);

    ems::class_<mx::MermaidGraphIo, ems::base<mx::GraphIo>>("MermaidGraphIo")
        .smart_ptr<std::shared_ptr<mx::MermaidGraphIo>>("MermaidGraphIo")
        .class_function("create", &mx::MermaidGraphIo::create)
        .function("write", &mx::MermaidGraphIo::write);
}

