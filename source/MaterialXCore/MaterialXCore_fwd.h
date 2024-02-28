//
// Created by Lee Kerley on 5/23/24.
//

#ifndef MATERIALX_MATERIALXCORE_FWD_H
#define MATERIALX_MATERIALXCORE_FWD_H

#include <MaterialXCore/Export.h>


MATERIALX_NAMESPACE_BEGIN

extern MX_CORE_API const string COLOR_SEMANTIC;
extern MX_CORE_API const string SHADER_SEMANTIC;

class NodeDef;
class Implementation;
class TypeDef;
class TargetDef;
class Member;
class Unit;
class UnitDef;
class UnitTypeDef;
class AttributeDef;

/// A shared pointer to a NodeDef
using NodeDefPtr = shared_ptr<NodeDef>;
/// A shared pointer to a const NodeDef
using ConstNodeDefPtr = shared_ptr<const NodeDef>;

/// A shared pointer to an Implementation
using ImplementationPtr = shared_ptr<Implementation>;
/// A shared pointer to a const Implementation
using ConstImplementationPtr = shared_ptr<const Implementation>;

/// A shared pointer to a TypeDef
using TypeDefPtr = shared_ptr<TypeDef>;
/// A shared pointer to a const TypeDef
using ConstTypeDefPtr = shared_ptr<const TypeDef>;

/// A shared pointer to a TargetDef
using TargetDefPtr = shared_ptr<TargetDef>;
/// A shared pointer to a const TargetDef
using ConstTargetDefPtr = shared_ptr<const TargetDef>;

/// A shared pointer to a Member
using MemberPtr = shared_ptr<Member>;
/// A shared pointer to a const Member
using ConstMemberPtr = shared_ptr<const Member>;

/// A shared pointer to a Unit
using UnitPtr = shared_ptr<Unit>;
/// A shared pointer to a const Unit
using ConstUnitPtr = shared_ptr<const Unit>;

/// A shared pointer to a UnitDef
using UnitDefPtr = shared_ptr<UnitDef>;
/// A shared pointer to a const UnitDef
using ConstUnitDefPtr = shared_ptr<const UnitDef>;

/// A shared pointer to a UnitTypeDef
using UnitTypeDefPtr = shared_ptr<UnitTypeDef>;
/// A shared pointer to a const UnitTypeDef
using ConstUnitTypeDefPtr = shared_ptr<const UnitTypeDef>;

/// A shared pointer to an AttributeDef
using AttributeDefPtr = shared_ptr<AttributeDef>;
/// A shared pointer to a const AttributeDef
using AttributeDefDefPtr = shared_ptr<const AttributeDef>;

MATERIALX_NAMESPACE_END

#endif // MATERIALX_MATERIALXCORE_FWD_H
