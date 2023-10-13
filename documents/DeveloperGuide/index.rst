MaterialX Developer Guide
=========================

.. toctree::
    :maxdepth: 1

    MainPage.md
    GraphEditor.md
    Viewer.md
    ShaderGeneration.md


MaterialX Python Modules
------------------------

.. autosummary::
    :toctree: generated
    :caption: MaterialX Python Modules:

    PyMaterialXCore
    PyMaterialXFormat
    PyMaterialXGenGlsl
    PyMaterialXGenMdl
    PyMaterialXGenMsl
    PyMaterialXGenOsl
    PyMaterialXGenShader
    PyMaterialXRender
    PyMaterialXRenderGlsl
    PyMaterialXRenderMsl
    PyMaterialXRenderOsl


Import Order Dependencies
-------------------------

Note that the order in which the MaterialX Python modules are imported matters, as there are dependencies between them:

- :py:mod:`PyMaterialXCore` -- typically always needed
    - :py:mod:`PyMaterialXFormat` -- needed for file I/O
    - :py:mod:`PyMaterialXRender` -- needed for core rendering
        - :py:mod:`PyMaterialXRenderGlsl` -- render using OpenGL Shading Language
        - :py:mod:`PyMaterialXRenderMsl` -- render using Metal Shading Language
        - :py:mod:`PyMaterialXRenderOsl` -- render using Open Shading Language
- :py:mod:`PyMaterialXGenShader` -- needed for core shader generation
    - :py:mod:`PyMaterialXGenGlsl` -- generating shaders using OpenGL Shading Language
    - :py:mod:`PyMaterialXGenMdl` -- generating shaders using Material Definition Language
    - :py:mod:`PyMaterialXGenMsl` -- generating shaders using Metal Shading Language
    - :py:mod:`PyMaterialXGenOsl` -- generating shaders using Open Shading Language

That is to say, for example:

- :py:mod:`PyMaterialXCore` needs to be imported before :py:mod:`PyMaterialXFormat` and :py:mod:`PyMaterialXRender`
- :py:mod:`PyMaterialXGenShader` needs to be imported before :py:mod:`PyMaterialXGenMsl`
