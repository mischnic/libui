// 9 september 2015
#import "uipriv_darwin.h"

// 10.8 fixups
#define NSEventModifierFlags NSUInteger

@interface openGLAreaView : uiprivAreaCommonView {
	uiOpenGLArea *libui_a;
}
- (id)initWithFrame:(NSRect)r area:(uiOpenGLArea *)a attributes:(uiOpenGLAttributes *)attribs;
@end

#define ATTRIBUTE_LIST_SIZE	256

struct uiOpenGLArea {
	uiDarwinControl c;
	NSView *view;			// either sv or area depending on whether it is scrolling
	uiOpenGLAreaHandler *ah;
	NSScrollView *sv;
	NSEvent *dragevent;
	BOOL scrolling;
	uiprivScrollViewData *d;
	
	openGLAreaView *area;

	NSOpenGLPixelFormat *pixFmt;
	NSOpenGLContext *ctx;
	BOOL initialized;
};

@implementation openGLAreaView

// This functionality is wrapped up here to guard against buffer overflows in the attribute list.
static void assignNextPixelFormatAttribute(NSOpenGLPixelFormatAttribute *as, unsigned int *ai, NSOpenGLPixelFormatAttribute a)
{
	if (*ai >= ATTRIBUTE_LIST_SIZE)
		uiprivImplBug("Too many pixel format attributes; increase ATTRIBUTE_LIST_SIZE!");
	as[*ai] = a;
	(*ai)++;
}

- (id)initWithFrame:(NSRect)r area:(uiOpenGLArea *)a attributes:(uiOpenGLAttributes *)attribs
{
	self = [super initWithFrame:r area:(uiArea *)a];
	if (self) {
		self->libui_a = a;
		NSOpenGLPixelFormatAttribute attrs[ATTRIBUTE_LIST_SIZE];
		unsigned int attrIndex = 0;
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAColorSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->RedBits + attribs->GreenBits + attribs->BlueBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAAlphaSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->AlphaBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFADepthSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->DepthBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAStencilSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->StencilBits);
		if (attribs->Stereo)
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAStereo);
		if (attribs->Samples > 0) {
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAMultisample);
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFASamples);
			assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->Samples);
		}
		if (attribs->DoubleBuffer)
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFADoubleBuffer);
		if (attribs->MajorVersion < 3) {
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAOpenGLProfile);
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLProfileVersionLegacy);
		} else {
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAOpenGLProfile);
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLProfileVersion3_2Core);
		}
		assignNextPixelFormatAttribute(attrs, &attrIndex, 0); // "a 0-terminated array"

		self->libui_a->pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
		if (self->libui_a->pixFmt == nil)
			uiprivUserBug("No available pixel format!");

		self->libui_a->ctx = [[NSOpenGLContext alloc] initWithFormat:self->libui_a->pixFmt shareContext:nil];
		if(self->libui_a->ctx == nil)
			uiprivUserBug("Couldn't create OpenGL context!");

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewBoundsDidChange:) name:NSViewFrameDidChangeNotification object:self->libui_a->area];
	}
	return self;
}

- (void)viewBoundsDidChange:(NSNotification *)notification
{
	NSLog(@"%@", self->libui_a->area);
	[self->libui_a->ctx setView:self->libui_a->area];
	[self->libui_a->ctx update];
}

- (void)drawRect:(NSRect)r
{
	uiOpenGLArea *a = self->libui_a;
	uiOpenGLAreaMakeCurrent(a);

	// NSSize s = a->scrolling ? [[a->sv documentView] frame].size : [self frame].size;
	NSSize s = a->scrolling ? NSZeroSize : [self frame].size;

	if (!a->initialized) {
		(*(a->ah->InitGL))(a->ah, a);
		a->initialized = YES;
	}
	(*(a->ah->DrawGL))(a->ah, a, s.width, s.height);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiOpenGLArea, view)

static void uiOpenGLAreaDestroy(uiControl *c)
{
	uiOpenGLArea *a = uiOpenGLArea(c);

	if (a->scrolling)
		uiprivScrollViewFreeData(a->sv, a->d);
	[a->area release];
	if (a->scrolling)
		[a->sv release];

	[a->ctx release];
	[a->pixFmt release];
	uiFreeControl(uiControl(a));
}

void uiOpenGLAreaSetSize(uiOpenGLArea *a, int width, int height)
{
	if (!a->scrolling)
		uiprivUserBug("You cannot call uiOpenGLAreaSetSize() on a non-scrolling uiOpenGLArea. (area: %p)", a);
	[a->area setScrollingSize:NSMakeSize(width, height)];
}

void uiOpenGLAreaScrollTo(uiOpenGLArea *a, double x, double y, double width, double height)
{
	if (!a->scrolling)
		uiprivUserBug("You cannot call uiOpenGLAreaScrollTo() on a non-scrolling uiOpenGLArea. (area: %p)", a);
	[a->area scrollRectToVisible:NSMakeRect(x, y, width, height)];
	// don't worry about the return value; it just says whether scrolling was needed
}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int si)
{
	[a->ctx setValues:&si forParameter: NSOpenGLCPSwapInterval];
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	[a->area setNeedsDisplay:YES];
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	[a->ctx makeCurrentContext];
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	[a->ctx flushBuffer];
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiDarwinNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->scrolling = NO;

	a->area = [[openGLAreaView alloc] initWithFrame:NSZeroRect area:a attributes:attribs];

	a->view = a->area;

	return a;
}

uiOpenGLArea *uiNewScrollingOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs, int width, int height)
{
	uiOpenGLArea *a;
	uiprivScrollViewCreateParams p;

	uiDarwinNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->scrolling = YES;

	a->area = [[openGLAreaView alloc] initWithFrame:NSMakeRect(0, 0, width, height)
		area:a attributes:attribs];



	memset(&p, 0, sizeof (uiprivScrollViewCreateParams));
	p.DocumentView = a->area;
	p.BackgroundColor = [NSColor controlColor];
	p.DrawsBackground = 1;
	p.Bordered = NO;
	p.HScroll = YES;
	p.VScroll = YES;
	a->sv = uiprivMkScrollView(&p, &(a->d));

	[[a->sv contentView] setPostsBoundsChangedNotifications:YES];
	[[NSNotificationCenter defaultCenter] addObserver:a->area selector:@selector(viewBoundsDidChange:) name:NSViewBoundsDidChangeNotification object:[a->sv contentView]]; 

	a->view = a->sv;

	return a;
}
