//
// Copyright (c) 2023 Apple Inc.
// Licensed under the Apache License v2.0
//

#ifndef COMPILEMSL_H
#define COMPILEMSL_H

#if __APPLE__
void CompileMSLShader(const char* pShaderFilePath, const char* pEntryFuncName);
#else
void CompileMSLShader(const char*, const char* ) {}
#endif

#endif // COMPILEMSL_H
