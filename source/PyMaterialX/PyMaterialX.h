//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PYMATERIALX_H
#define MATERIALX_PYMATERIALX_H

//
// This header is used to include PyBind11 headers consistently across the
// translation units in the PyMaterialX library, and it should be the first
// include within any PyMaterialX source file.
//

#ifdef _MSC_VER
#define HAVE_ACOSH
#define HAVE_ASINH
#define HAVE_ATANH
#define HAVE_HYPOT
#define HAVE_LOG1P
#define HAVE_ROUND
#endif

#include <pybind11/operators.h>
#include <pybind11/stl.h>

#endif
