//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXView/RenderPipeline.h>
#include <MaterialXView/Viewer.h>

#include <iostream>

RenderPipeline::RenderPipeline(Viewer* viewer)
{
    _viewer = viewer;
}

void RenderPipeline::renderScreenSpaceQuad(mx::MaterialPtr material)
{
    if (!_quadMesh)
        _quadMesh = mx::GeometryHandler::createQuadMesh();
    
    material->bindMesh(_quadMesh);
    material->drawPartition(_quadMesh->getPartition(0));
}
