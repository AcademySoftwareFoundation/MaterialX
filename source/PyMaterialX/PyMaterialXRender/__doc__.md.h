//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXRender module

#define PyMaterialXRender_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Core rendering support for MaterialX.

Core Rendering Classes
----------------------

.. autosummary::
    :toctree: core-rendering

    ShaderRenderer
    Camera
    LightHandler

Geometry Classes
----------------

.. autosummary::
    :toctree: geometry

    GeometryHandler
    GeometryLoader
    CgltfLoader
    TinyObjLoader
    Mesh
    MeshPartition
    MeshStream

Image Classes
--------------

.. autosummary::
    :toctree: images

    ImageHandler
    ImageLoader
    StbImageLoader
    Image
    ImageBufferDeallocator
    ImageSamplingProperties

Image Functions
---------------

.. autofunction:: createImageStrip
.. autofunction:: createUniformImage
.. autofunction:: getMaxDimensions

Enumeration Classes
-------------------

.. autosummary::
    :toctree: enumerations

    BaseType

Exception Classes
-----------------

.. autosummary::
    :toctree: exceptions

    ExceptionRenderError
)docstring");
