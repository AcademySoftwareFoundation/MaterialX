#ifndef MATERIALX_WINDOWCOCOAWRAPPERS_H
#define MATERIALX_WINDOWCOCOAWRAPPERS_H

#include <MaterialXView/Window/HardwarePlatform.h>
#if defined(OSMac_)

/// Wrappers for calling into Objective-C Cocoa routines on Mac for Windowing
#ifdef __cplusplus
extern "C" {
#endif

// Window routines
void*	NSUtilGetView(void* pWindow);
void*	NSUtilCreateWindow(unsigned int width, unsigned int height, char* title, bool batchMode);
void	NSUtilShowWindow(void* pWindow);
void	NSUtilHideWindow(void* pWindow);
void	NSUtilSetFocus(void* pWindow);
void	NSUtilDisposeWindow(void* pWindow);

#ifdef __cplusplus
}
#endif

#endif

#endif
