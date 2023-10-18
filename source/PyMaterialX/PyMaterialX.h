//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PYMATERIALX_H
#define MATERIALX_PYMATERIALX_H

//
// This header is used to include PyBind11 headers consistently across the
// translation units in the PyMaterialX library, and it should be the first
// include within any PyMaterialX source file.
//

// Set a flag to allow PyBind11 to provide more helpful error messages
#define PYBIND11_DETAILED_ERROR_MESSAGES 1

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#endif
