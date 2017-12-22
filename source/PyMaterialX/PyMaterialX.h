//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PYMATERIALX_H
#define MATERIALX_PYMATERIALX_H

//
// This header contains Python.h, and should be the first include for any
// translation unit within the PyMaterialX library.
//

#ifdef _MSC_VER
#define HAVE_ACOSH
#define HAVE_ASINH
#define HAVE_ATANH
#define HAVE_LOG1P
#define HAVE_HYPOT
#endif

#include <PyBind11/pybind11.h>

#endif
