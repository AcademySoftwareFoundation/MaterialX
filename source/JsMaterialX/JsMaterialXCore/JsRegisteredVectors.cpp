#include "../helpers.h"
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Variant.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Definition.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(registered_vectors)
    {
        ems::register_vector<std::string>("vector<std::string>");
        ems::register_vector<int>("vector<int>");
        ems::register_vector<mx::VariantPtr>("vector<mx::VariantPtr>");
        ems::register_vector<mx::VariantAssignPtr>("vector<mx::VariantAssignPtr>");
        ems::register_vector<mx::VariantSetPtr>("vector<mx::VariantSetPtr>");
        ems::register_vector<mx::ElementPtr>("vector<mx::ElementPtr>");
        ems::register_vector<mx::PortElementPtr>("vector<mx::PortElementPtr>");
        ems::register_vector<mx::ValueElementPtr>("vector<mx::ValueElementPtr>");
        ems::register_vector<mx::InterfaceElementPtr>("vector<mx::InterfaceElementPtr>");
        ems::register_vector<mx::BackdropPtr>("vector<mx::BackdropPtr>");
        ems::register_vector<mx::MaterialAssignPtr>("vector<mx::MaterialAssignPtr>");
        ems::register_vector<mx::PropertyAssignPtr>("vector<mx::PropertyAssignPtr>");
        ems::register_vector<mx::PropertySetAssignPtr>("vector<mx::PropertySetAssignPtr>");
        ems::register_vector<mx::VisibilityPtr>("vector<mx::VisibilityPtr>");
        ems::register_vector<mx::InputPtr>("vector<mx::InputPtr>");
        ems::register_vector<mx::OutputPtr>("vector<mx::OutputPtr>");
        ems::register_vector<mx::TokenPtr>("vector<mx::TokenPtr>");
        ems::register_vector<mx::NodePtr>("vector<mx::NodePtr>");
        ems::register_vector<mx::NodeGraphPtr>("vector<mx::NodeGraphPtr>");
        ems::register_vector<mx::NodeDefPtr>("vector<mx::NodeDefPtr>");
        ems::register_vector<mx::TypeDefPtr>("vector<mx::TypeDefPtr>");
        ems::register_vector<mx::ImplementationPtr>("vector<mx::ImplementationPtr>");
        ems::register_vector<mx::UnitTypeDefPtr>("vector<mx::UnitTypeDefPtr>");
        ems::register_vector<mx::MemberPtr>("vector<mx::MemberPtr>");
        ems::register_vector<mx::UnitPtr>("vector<mx::UnitPtr>");
        ems::register_vector<mx::UnitDefPtr>("vector<mx::UnitDefPtr>");
        ems::register_vector<mx::GeomInfoPtr>("vector<mx::GeomInfoPtr>");
        ems::register_vector<mx::GeomPropDefPtr>("vector<mx::GeomPropDefPtr>");
        ems::register_vector<mx::LookPtr>("vector<mx::LookPtr>");
        ems::register_vector<mx::LookGroupPtr>("vector<mx::LookGroupPtr>");
        ems::register_vector<mx::CollectionPtr>("vector<mx::CollectionPtr>");
        ems::register_vector<mx::PropertySetPtr>("vector<mx::PropertySetPtr>");
    }
}