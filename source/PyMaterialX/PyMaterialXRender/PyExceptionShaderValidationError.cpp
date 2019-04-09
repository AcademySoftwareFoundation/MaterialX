//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ShaderValidators/ExceptionShaderValidationError.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyExceptionShaderValidationError(py::module& mod)
{
    static py::exception<mx::ExceptionShaderValidationError> pyExceptionShaderValidationError(mod, "ExceptionShaderValidationError");

    py::register_exception_translator(
        [](std::exception_ptr errPtr)
        {
            try
            {
                if (errPtr != NULL)
                    std::rethrow_exception(errPtr);
            }
            catch (const mx::ExceptionShaderValidationError& err)
            {
                std::string errorMsg = err.what();
                for (std::string error : err.errorLog())
                {
                    errorMsg += "\n" + error;
                }
                PyErr_SetString(PyExc_LookupError, errorMsg.c_str());
            }
        }
    );
}
