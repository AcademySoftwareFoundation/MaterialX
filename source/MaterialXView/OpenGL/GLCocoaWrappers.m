
#include "Platform.h"
#if defined (OSMac_)

#import <Cocoa/Cocoa.h>
#import <AppKit/NSApplication.h>
#import "CocoaWrappers.h"

void* NSOpenGLGetView(void* pWindow)
{
	NSWindow* window = pWindow;
	NSView* view =  [window contentView];
	return (void*)view;
}

void* NSOpenGLCreateWindow(unsigned int width, unsigned int height, char* title, bool batchMode)
{
	// In batch mode, ensure that Cocoa is initialized
	if (batchMode)
	{
		NSApplicationLoad();
	}

	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
		styleMask:NSTitledWindowMask | NSClosableWindowMask	| NSMiniaturizableWindowMask | NSResizableWindowMask
		backing:NSBackingStoreBuffered defer:NO];
	NSString *string = [NSString stringWithUTF8String:title];

	[window setTitle:string];
	[window setAlphaValue:0.0];

	// Free up memory
	[pool release];

	return (void*)window;
}

void NSOpenGLShowWindow(void* pWindow)
{
	NSWindow* window = pWindow;
	[window orderFront:window];
}

void NSOpenGLHideWindow(void* pWindow)
{
	NSWindow* window = pWindow;
	[window orderOut:window];
}

void NSOpenGLSetFocus(void* pWindow)
{
	NSWindow* window = pWindow;
	[window makeKeyAndOrderFront:window];
}

void NSOpenGLDisposeWindow(void* pWindow)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSWindow* window = pWindow;
	[window close];

	// Free up memory
	[pool release];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void* NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType, int colorSize, int depthFormat,
								int stencilFormat, int auxBuffers, int accumSize, bool minimumPolicy,
								bool accelerated, bool mp_safe,  bool stereo, bool supportMultiSample)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSOpenGLPixelFormatAttribute list[50];
	int i = 0;
	if (allRenders)
		list[i++] = NSOpenGLPFAAllRenderers;
	if (bufferType == 1) //kOnScreen
		list[i++] = NSOpenGLPFADoubleBuffer;
	if (colorSize != 0)
		list[i++] = NSOpenGLPFAColorSize; list[i++] = colorSize;
	if (depthFormat != 0)
		list[i++] = NSOpenGLPFADepthSize; list[i++] = depthFormat;
	if (stencilFormat != 0)
		list[i++] = NSOpenGLPFAStencilSize; list[i++] = stencilFormat;
	if (auxBuffers != 0)
		list[i++] = NSOpenGLPFAAuxBuffers; list[i++] = auxBuffers;
	if (accumSize != 0)
		list[i++] = NSOpenGLPFAAccumSize; list[i++] = accumSize;
	if (minimumPolicy)
	{
		list[i++] = NSOpenGLPFAMinimumPolicy;
	}
	if (accelerated)
	{
		list[i++] = NSOpenGLPFAAccelerated;
	}
	if (mp_safe)
	{
		list[i++] = NSOpenGLPFAMPSafe;
	}
	if (stereo)
	{
		list[i++] = NSOpenGLPFAStereo;
	}

	// Add multisample support to the list of attributes if supported
	//
	int multiSampleAttrIndex = i;
	if (supportMultiSample)
	{
		// TODO: Will need a better way of determining sampling numbers.
		// Currently fixed at 4 samples.
		//
		list[i++] = NSOpenGLPFASampleBuffers; list[i++] = TRUE;
		list[i++] = NSOpenGLPFASamples; list[i++] = 4;
	}

	list[ i++] = 0 ;

	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:list];
	if (!pixelFormat)
	{
		// Try again without multisample, if previous try failed
		//
		list[multiSampleAttrIndex++] = 0;	// NSOpenGLPFASampleBuffers
		list[multiSampleAttrIndex++] = 0;	// NSOpenGLPFASampleBuffers value
		list[multiSampleAttrIndex++] = 0;	// NSOpenGLPFASamplesB
		list[multiSampleAttrIndex++] = 0;	// NSOpenGLPFASamples value

		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:list];
	}

	// Free up memory
	[pool release];

	return pixelFormat;
}

void NSOpenGLReleasePixelFormat(void* pPixelFormat)
{
	NSOpenGLPixelFormat *pixelFormat = pPixelFormat;
	[pixelFormat release];
}

void NSOpenGLReleaseContext(void* pContext)
{
	NSOpenGLContext *context = pContext;
	[context release];
}

void* NSOpenGLCreateContextWrapper(void* pPixelFormat, void *pDummyContext)
{
	NSOpenGLPixelFormat *pixelFormat = pPixelFormat;
	NSOpenGLContext *dummyContext = (NSOpenGLContext*)pDummyContext;
	NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
						shareContext:dummyContext];

	return context;
}

void NSOpenGLSetDrawable(void* pContext, void* pWindow)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSOpenGLContext *context = (NSOpenGLContext*)pContext;
	NSWindow *window = (NSWindow*)pWindow;
	NSView *view = [window contentView];
	[context setView:view];

	// Free up memory
	[pool release];
}

void NSOpenGLMakeCurrent(void* pContext)
{
	NSOpenGLContext* context = pContext;
	[context makeCurrentContext];
}

void* NSOpenGLGetCurrentContextWrapper()
{
	return [NSOpenGLContext currentContext];
}

void NSOpenGLSwapBuffers(void* pContext)
{
	NSOpenGLContext* context = pContext;
	[context flushBuffer];
}

void NSOpenGLClearCurrentContext()
{
	[NSOpenGLContext clearCurrentContext];
}

void NSOpenGLDestroyContext(void** pContext)
{
	NSOpenGLContext* context = *pContext;
	[context release];
	*pContext = NULL;
}

void NSOpenGLDestroyCurrentContext(void** pContext)
{
	[NSOpenGLContext clearCurrentContext];
	NSOpenGLContext* context = *pContext;
	[context release];
	*pContext = NULL;
}

//pContext1 is srource. pContext2 is destination
void NSOpenGLCopyContext(void* pContext1, void* pContext2, GLuint mask)
{
	NSOpenGLContext* context1 = pContext1;
	NSOpenGLContext* context2 = pContext2;
	[context2 copyAttributesFromContext:context1 withMask:mask];
}

void NSOpenGLClearDrawable(void* pContext)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSOpenGLContext* context = pContext;
	[context clearDrawable];

	// Free up memory
	[pool release];
}

void NSOpenGLDescribePixelFormat(void* pPixelFormat, int attrib, GLint* vals)
{
	NSOpenGLPixelFormat *pixelFormat = (NSOpenGLPixelFormat*)pPixelFormat;
	[pixelFormat getValues:vals forAttribute:attrib forVirtualScreen:0];
}

void NSOpenGLGetInteger(void* pContext, int param, GLint* vals)
{
	NSOpenGLContext* context = pContext;
	[context getValues:vals forParameter:param];
}

void NSOpenGLUpdate(void* pContext)
{
	NSOpenGLContext* context = pContext;
	[context update];
}

void* NSOpenGLCGLContextObj(void* pContext)
{
	NSOpenGLContext *context = pContext;
	NSOpenGLContextAuxiliary* contextAuxiliary =  [context CGLContextObj];
	return contextAuxiliary;
}

void* NSOpenGLGetWindow(void* pView)
{
	NSView *view = pView;
	return [view window];
}

void NSOpenGLInitializeGLLibrary()
{
	// Create local autorelease pool for any objects that need to be autoreleased (needed in batch mode).
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSOpenGLPixelFormatAttribute attrib[] = {NSOpenGLPFAAllRenderers, NSOpenGLPFADoubleBuffer, 0};
	NSOpenGLPixelFormat *dummyPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrib];
	if (nil != dummyPixelFormat)
    {
		[dummyPixelFormat release];
	}
	[pool release];
}

#endif
