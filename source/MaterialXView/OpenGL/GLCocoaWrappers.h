#ifndef MATERIALX_GLCOCOAWRAPPERS_H
#define MATERIALX_GLCOCOAWRAPPERS_H

#include <MaterialXView/Window/HardwarePlatform.h>
#if defined(OSMac_)

/// Wrappers for calling into Objective-C Cocoa routines on Mac
#ifdef __cplusplus
extern "C" {
#endif

// Window routines
void*	NSOpenGLGetView(void* pWindow);
void*	NSOpenGLCreateWindow(unsigned int width, unsigned int height, char* title, bool batchMode);
void	NSOpenGLShowWindow(void* pWindow);
void	NSOpenGLHideWindow(void* pWindow);
void	NSOpenGLSetFocus(void* pWindow);
void	NSOpenGLDisposeWindow(void* pWindow);

// OpenGL specific routines
void*   NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType, int colorSize, int depthFormat,
                                        int stencilFormat, int auxBuffers, int accumSize, bool minimumPolicy,
                                        bool accelerated, bool mp_safe, bool stereo, bool supportMultiSample);
void	NSOpenGLReleasePixelFormat(void* pPixelFormat);
void	NSOpenGLReleaseContext(void* pContext);
void*	NSOpenGLCreateContextWrapper(void* pPixelFormat, void *pDummyContext);
void	NSOpenGLSetDrawable(void* pContext, void* pView);
void	NSOpenGLMakeCurrent(void* pContext);
void*	NSOpenGLGetCurrentContextWrapper();
void	NSOpenGLSwapBuffers(void* pContext);
void	NSOpenGLClearCurrentContext();
void	NSOpenGLDestroyContext(void** pContext);
void	NSOpenGLDestroyCurrentContext(void** pContext);
void	NSOpenGLCopyContext(void* pContext1, void* pContext2, GLuint mask);
void	NSOpenGLClearDrawable(void* pContext);
void	NSOpenGLDescribePixelFormat(void* pPixelFormat, int attrib, GLint* vals);
void	NSOpenGLGetInteger(void* pContext, int param, GLint* vals);
void	NSOpenGLUpdate(void* pContext);
void*	NSOpenGLCGLContextObj(void* pContext);
void*	NSOpenGLGetWindow(void* pView);
void	NSOpenGLInitializeGLLibrary();

#ifdef __cplusplus
}
#endif

#endif

#endif
