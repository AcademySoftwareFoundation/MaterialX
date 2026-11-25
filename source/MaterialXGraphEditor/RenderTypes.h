//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERTYPES_H
#define MATERIALX_RENDERTYPES_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Types.h>
#include <MaterialXFormat/File.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/ImageHandler.h>

#include <string>
#include <vector>

namespace mx = MaterialX;

/// Description of one render pass exposed by a render pipeline.
struct RenderPassDesc
{
    std::string id;
    std::string label;
};

/// Capabilities advertised by a render pipeline so that the user interface can
/// expose only the controls that the active backend supports.
struct RenderCapabilities
{
    std::vector<RenderPassDesc> passes;
    bool denoising = false;
    int maxDenoisePasses = 0;
    bool adaptiveSampling = false;
    bool sampling = false;
    bool lightIntensity = false;
    bool uniformUpdates = false;
    bool meshReload = false;
    bool asynchronous = false;
};

/// Backend-neutral rendering options. Pipelines apply only the values covered
/// by their capabilities.
struct RenderSettings
{
    std::string renderPass = "combined";
    bool denoise = true;
    int denoiseStartSample = 16;
    int denoisePasses = 2;
    bool adaptiveSampling = false;
    float adaptiveThreshold = 0.01f;
    int adaptiveMinSamples = 0;
    int maxSamples = 4096;
    float lightIntensity = 0.75f;
};

/// Camera state shared between the render view and the active render pipeline.
/// The view owns the interactive state; pipelines use it to update their native
/// camera representation.
struct RenderCameraState
{
    mx::Vector3 position = mx::Vector3(0.0f, 0.0f, 5.0f);
    mx::Vector3 target = mx::Vector3(0.0f);
    mx::Vector3 up = mx::Vector3(0.0f, 1.0f, 0.0f);
    float viewAngle = 45.0f;
    float zoom = 1.0f;
    mx::Vector3 userTranslation = mx::Vector3(0.0f);
    unsigned int revision = 0;
};

#endif
