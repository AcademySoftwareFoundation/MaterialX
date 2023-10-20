//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PYMATERIALX_H
#define MATERIALX_PYMATERIALX_H

//
// This header is used to include pybind11 headers consistently across the
// translation units in the PyMaterialX library, and it should be the first
// include within any PyMaterialX source file.
//

// Set a flag to allow pybind11 to provide more helpful error messages
// (see `pybind11/detail/common.h`)
#define PYBIND11_DETAILED_ERROR_MESSAGES 1

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

// Define a macro to import a PyMaterialX module, e.g. `PyMaterialXCore`,
// either within the `MaterialX` Python package, e.g. in `installed/python/`,
// or as a standalone module, e.g. in `lib/`
#define PYMATERIALX_IMPORT_MODULE(MODULE_NAME)                               \
    try {                                                                    \
        pybind11::module::import("MaterialX." #MODULE_NAME);                 \
    }                                                                        \
    catch (const py::error_already_set&) {                                   \
        pybind11::module::import(#MODULE_NAME);                              \
    }

// Define a macro to process a block of docstring text
// (currently, we strip the first character from the given text, so that we can
// write the first line of the docstring on a new line, rather than on the same
// line that starts the docstring)
#define PYMATERIALX_DOCSTRING(TEXT) std::string(TEXT).erase(0, 1).c_str()

#endif
