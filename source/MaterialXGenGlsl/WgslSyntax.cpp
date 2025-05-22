//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslSyntax.h>

MATERIALX_NAMESPACE_BEGIN

WgslSyntax::WgslSyntax(TypeSystemPtr typeSystem) : VkSyntax(typeSystem)
{
    // Add in WGSL specific keywords
    registerReservedWords( { 
    	"type" 
    	} );
}


MATERIALX_NAMESPACE_END
