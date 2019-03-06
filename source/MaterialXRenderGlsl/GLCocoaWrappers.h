//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLCOCOAWRAPPERS_H
#define MATERIALX_GLCOCOAWRAPPERS_H

#include <MaterialXRender/HardwarePlatform.h>
#if defined(OSMac_)

/// Wrappers for calling into OpenGL related Objective-C Cocoa routines on Mac
#ifdef __cplusplus
extern "C" {
#endif

// OpenGL specific routines
void* NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType, int colorSize, int depthFormat,
                                      int stencilFormat, int auxBuffers, int accumSize, bool minimumPolicy,
                                      bool accelerated, bool mp_safe, bool stereo, bool supportMultiSample);
void NSOpenGLReleasePixelFormat(void* pPixelFormat);
void NSOpenGLReleaseContext(void* pContext);
void* NSOpenGLCreateContextWrapper(void* pPixelFormat, void *pDummyContext);
void NSOpenGLSetDrawable(void* pContext, void* pView);
void NSOpenGLMakeCurrent(void* pContext);
void* NSOpenGLGetCurrentContextWrapper();
void NSOpenGLSwapBuffers(void* pContext);
void NSOpenGLClearCurrentContext();
void NSOpenGLDestroyContext(void** pContext);
void NSOpenGLDestroyCurrentContext(void** pContext);
void NSOpenGLClearDrawable(void* pContext);
void NSOpenGLDescribePixelFormat(void* pPixelFormat, int attrib, int* vals);
void NSOpenGLGetInteger(void* pContext, int param, int* vals);
void NSOpenGLUpdate(void* pContext);
void* NSOpenGLCGLContextObj(void* pContext);
void* NSOpenGLGetWindow(void* pView);
void NSOpenGLInitializeGLLibrary();

#ifdef __cplusplus
}
#endif

#endif

#endif
