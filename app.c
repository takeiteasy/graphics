//
//  app.c
//  app
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#include "app.h"

#define RGB2INT(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b
#define XYSET(s, x, y, v) (s->buf[(y) * s->w + (x)] = (v))
#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

surface_t* create_surface(unsigned int w, unsigned int h) {
  surface_t* ret = malloc(sizeof(surface_t));
  if (!ret) {
    fprintf(stderr, "ERROR! malloc() failed.\n");
    return NULL;
  }
  
  ret->w = w;
  ret->h = h;
  ret->buf = malloc(w * h * sizeof(unsigned int) + 1);
  if (!ret->buf) {
    fprintf(stderr, "ERROR! malloc() failed.\n");
    return NULL;
  }
  
  return ret;
}

void free_surface(surface_t** s) {
  free((*s)->buf);
  (*s)->buf = NULL;
  free(*s);
  *s = NULL;
}

void fill_surface(surface_t* s, int r, int g, int b) {
  for (int x = 0; x < s->w; ++x)
    for (int y = 0; y < s->h; ++y)
      XYSET(s, x, y, RGB2INT(r, g, b));
}

bool pset(surface_t* s, int x, int y, int r, int g, int b) {
  if (x > s->w || y > s->h) {
    fprintf(stderr, "ERROR! pset() failed! x/y outside of bounds.\n");
    return false;
  }
  XYSET(s, x, y, RGB2INT(r, g, b));
  return true;
}

int pget(surface_t* s, int x, int y) {
  if (x > s->w || y > s->h) {
    fprintf(stderr, "ERROR! pget() failed! x/y outside of bounds.\n");
    return 0;
  }
  return XYGET(s, x, y);
}

bool blit_surface(surface_t* dst, point_t* p, surface_t* src, rect_t* r) {
  if (!src || !dst) {
    fprintf(stderr, "ERROR! blit_surface() failed! src and dst must not be null.\n");
    return false;
  }
  
  int offset_x = 0,      offset_y = 0,
      from_x   = 0,      from_y   = 0,
      width    = src->w, height   = src->h;
  if (p) {
    offset_x = p->x;
    offset_y = p->y;
  }
  if (r) {
    from_x = r->x;
    from_y = r->y;
    width  = r->w;
    height = r->h;
  }
  int to_x = offset_x + width, to_y = offset_y + height;
  if (to_x > dst->w || to_y > dst->h) {
    fprintf(stderr, "ERROR! blit_surface() failed! src w/h outside bounds of dst.\n");
    return false;
  }
  
  int x, y;
  for (x = 0; x < width; ++x)
    for (y = 0; y < height; ++y)
      XYSET(dst, offset_x + x, offset_y + y, XYGET(src, from_x + x, from_y + y));
  return true;
}

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>

@interface osx_app_t : NSWindow {
  NSView* view;
  @public bool closed;
}
@end

@interface osx_view_t : NSView {}
@end

static surface_t* buffer;
static osx_app_t* app;

@implementation osx_view_t
extern surface_t* buffer;

-(NSRect)resizeRect {
  NSRect v = [[self window] contentRectForFrameRect:[[self window] frame]];
  return NSMakeRect(NSMaxX(v) + 5.5, NSMinY(v) - 16.0 - 5.5, 16.0, 16.0);
}

-(void)drawRect:(NSRect)r {
  (void)r;
  
  if (!buffer)
    return;
  
  CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
  
  CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, buffer->buf, buffer->w * buffer->h * 4, NULL);
  CGImageRef img = CGImageCreate(buffer->w, buffer->h, 8, 32, buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, false, kCGRenderingIntentDefault);
  
  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);
  
  CGContextDrawImage(ctx, CGRectMake(0, 0, buffer->w, buffer->h), img);
  
  CGImageRelease(img);
}
@end

@implementation osx_app_t
-(id)initWithContentRect:(NSRect)r
               styleMask:(NSWindowStyleMask)s
                 backing:(NSBackingStoreType)t
                   defer:(BOOL)d {
  self = [super initWithContentRect:r
                          styleMask:s
                            backing:t
                              defer:d];
  if (self) {
    [self setOpaque:YES];
    [self setBackgroundColor:[NSColor clearColor]];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_changed:)
                                                 name:NSWindowDidBecomeMainNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_changed:)
                                                 name:NSWindowDidResignMainNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_close)
                                                 name:NSWindowWillCloseNotification
                                               object:self];
    
    closed = false;
  }
  return self;
}

-(void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [super dealloc];
}

-(void)setContentSize:(NSSize)s {
  NSSize sizeDelta = s;
  NSSize childBoundsSize = [view bounds].size;
  sizeDelta.width -= childBoundsSize.width;
  sizeDelta.height -= childBoundsSize.height;
  
  osx_view_t* fv = [super contentView];
  NSSize ns  = [fv bounds].size;
  ns.width  += sizeDelta.width;
  ns.height += sizeDelta.height;
  
  [super setContentSize:ns];
}

-(void)setContentView:(NSView *)v {
  if ([view isEqualTo:v])
    return;
  
  NSRect b = [self frame];
  b.origin = NSZeroPoint;
  osx_view_t* fv = [super contentView];
  if (!fv) {
    fv = [[[osx_view_t alloc] initWithFrame:b] autorelease];
    [super setContentView:fv];
  }
  
  if (view)
    [view removeFromSuperview];
  
  view = v;
  [view setFrame:[self contentRectForFrameRect:b]];
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [fv addSubview:view];
}

-(void)win_changed:(NSNotification *)n { (void)n; }
-(void)win_close                       { closed = true; }
-(NSView*)contentView                  { return view; }
-(BOOL)canBecomeKeyWindow              { return YES;  }
-(BOOL)canBecomeMainWindow             { return YES;  }

-(NSRect)contentRectForFrameRect:(NSRect)f {
  f.origin = NSZeroPoint;
  return NSInsetRect(f, 0, 0);
}
+(NSRect)frameRectForContentRect:(NSRect)r
                       styleMask:(NSWindowStyleMask)s {
  (void)s;
  return NSInsetRect(r, 0, 0);
}
@end

surface_t* app_open(const char* t, int w, int h) {
  if (!(buffer = create_surface(w, h)))
    return NULL;
  buffer->w = w;
  buffer->h = h;
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  
  app = [[osx_app_t alloc] initWithContentRect:NSMakeRect(0, 0, w, h)
                                     styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskTitled
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
  if (!app)
    return NULL;
  
  [app setTitle:[NSString stringWithUTF8String:t]];
  [app setReleasedWhenClosed:NO];
  [app performSelectorOnMainThread:@selector(makeKeyAndOrderFront:) withObject:nil waitUntilDone:YES];
  [app center];
  
  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];
  
  return buffer;
}

bool app_update() {
  bool ret = true;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSEvent* e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                  untilDate:[NSDate distantPast]
                                     inMode:NSDefaultRunLoopMode
                                    dequeue:YES];
  if (e) {
    switch ([e type]) {
      case NSEventTypeKeyDown:
      case  NSEventTypeKeyUp:
        ret = false;
        break;
      default :
        [NSApp sendEvent:e];
    }
  }
  [pool release];
  
  if (app->closed)
    ret = false;
  [[app contentView] setNeedsDisplay:YES];

  return ret;
}

void app_close() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  if (app)
  	[app close];
  [pool drain];
}
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

static WNDCLASS wc;
static HWND wnd;
static int close = 0;
static int width;
static int height;
static HDC hdc;
static void* buffer;
static BITMAPINFO* bitmapInfo;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT res = 0;
  switch (message) {
    case WM_PAINT:
      if (buffer) {
        StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, buffer, bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(hWnd, NULL);
      }
      break;
    case WM_KEYDOWN:
      if ((wParam&0xFF) == 27)
        close = 1;
      break;
    case WM_CLOSE:
      close = 1;
      break;
    default:
      res = DefWindowProc(hWnd, message, wParam, lParam);
  }
  return res;
}

int app_open(const char* title, int width, int height) {
  wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.lpszClassName = title;
  RegisterClass(&wc);
  
  RECT rect    = { 0 };
  rect.right   = width;
  rect.bottom  = height;
  AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU | WS_CAPTION, 0);
  rect.right  -= rect.left;
  rect.bottom -= rect.top;
  
  width  = width;
  height = height;
  
  wnd = CreateWindowEx(0, title, title,
                       WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       rect.right, rect.bottom,
                       0, 0, 0, 0);
  
  if (!wnd)
    return 0;
  
  ShowWindow(wnd, SW_NORMAL);
  
  bitmapInfo = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3);
  bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmapInfo->bmiHeader.biPlanes = 1;
  bitmapInfo->bmiHeader.biBitCount = 32;
  bitmapInfo->bmiHeader.biCompression = BI_BITFIELDS;
  bitmapInfo->bmiHeader.biWidth = width;
  bitmapInfo->bmiHeader.biHeight = -height;
  bitmapInfo->bmiColors[0].rgbRed = 0xff;
  bitmapInfo->bmiColors[1].rgbGreen = 0xff;
  bitmapInfo->bmiColors[2].rgbBlue = 0xff;
  
  hdc = GetDC(wnd);
  
  return 1;
}

int app_update(void* buffer) {
  MSG msg;
  buffer = buffer;
  
  InvalidateRect(wnd, NULL, TRUE);
  SendMessage(wnd, WM_PAINT, 0, 0);
  while (PeekMessage(&msg, wnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  if (close == 1)
    return 0;
  
  return 1;
}

void app_close() {
  buffer = 0;
  free(bitmapInfo);
  ReleaseDC(wnd, hdc);
  DestroyWindow(wnd);
}
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display* display;
static int screen;
static int width;
static int height;
static Window window;
static GC gc;
static XImage *ximage;

int app_open(const char* title, int width, int height) {
  int depth, i, formatCount, convDepth = -1;
  XPixmapFormatValues* formats;
  XSetWindowAttributes windowAttributes;
  XSizeHints sizeHints;
  Visual* visual;
  
  display = XOpenDisplay(0);
  
  if (!display)
    return -1;
  
  screen  = DefaultScreen(display);
  visual  = DefaultVisual(display, screen);
  formats = XListPixmapFormats(display, &formatCount);
  depth   = DefaultDepth(display, screen);
  Window defaultRootWindow = DefaultRootWindow(display);
  
  for (i = 0; i < formatCount; ++i) {
    if (depth == formats[i].depth) {
      convDepth = formats[i].bitper_pixel;
      break;
    }
  }
  XFree(formats);
  
  if (convDepth != 32) {
    XCloseDisplay(display);
    return -1;
  }
  
  int screenWidth  = DisplayWidth(display, screen);
  int screenHeight = DisplayHeight(display, screen);
  
  windowAttributes.border_pixel     = BlackPixel(display, screen);
  windowAttributes.background_pixel = BlackPixel(display, screen);
  windowAttributes.backing_store    = NotUseful;
  
  window = XCreateWindow(display, defaultRootWindow, (screenWidth - width) / 2,
                         (screenHeight - height) / 2, width, height, 0, depth, InputOutput,
                         visual, CWBackPixel | CWBorderPixel | CWBackingStore,
                         &windowAttributes);
  if (!window)
    return 0;
  
  XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
  XStoreName(display, window, title);
  
  sizeHints.flags = PPosition | PMinSize | PMaxSize;
  sizeHints.x = 0;
  sizeHints.y = 0;
  sizeHints.min_width = width;
  sizeHints.max_width = width;
  sizeHints.min_height = height;
  sizeHints.max_height = height;
  
  XSetWMNormalHints(display, window, &sizeHints);
  XClearWindow(display, window);
  XMapRaised(display, window);
  XFlush(display);
  
  gc = DefaultGC(display, screen);
  ximage = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, NULL, width, height, 32, width * 4);
  
  width  = width;
  height = height;
  
  return 1;
}

int app_update(void* buffer) {
  ximage->data = (char*)buffer;
  
  XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
  XFlush(display);
  
  if (!XPending(display))
    return 0;
  
  XEvent event;
  XNextEvent(display, &event);
  KeySym sym = XLookupKeysym(&event.xkey, 0);
  
  return 1;
}

void app_close (void) {
  ximage->data = NULL;
  XDestroyImage(ximage);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}
#endif
