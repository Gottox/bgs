/* See LICENSE file for copyright and license details.
 *
 * To understand bgs , start reading main().
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

/* macros */
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define BUTTONMASK      (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask) (mask & ~(numlockmask|LockMask))
#define LENGTH(x)       (sizeof x / sizeof x[0])
#define MAXTAGLEN       16
#define MOUSEMASK       (BUTTONMASK|PointerMotionMask)
#define TAGMASK         ((int)((1LL << LENGTH(tags)) - 1))
#define TEXTW(x)        (textnw(x, strlen(x)) + dc.font.height)
#define ISVISIBLE(x)    (x->tags & tagset[seltags])

/* typedefs */
typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct {
	int x, y;
	uint w, h;
} Monitor;

/* function declarations */
static void cleanup(void);
static void configurenotify(XEvent *e);
static void createbg(void);
static void die(const char *errstr, ...);
static void drawbg(void);
static void run(void);
static void setup(char *images[], int c);
static void updategeom(void);

/* variables */
int sx, sy, sw, sh;
Bool running = True;
Display *dpy;
Window root;
Visual *vis;
Colormap cm;
int nmonitor;
Monitor monitors[8];
Imlib_Image images[LENGTH(monitors)];
Imlib_Image buffer;
uint nimage;
int depth;
int screen;
int bgcolor;
Pixmap pm;

/* function implementations */
void
cleanup(void) {
	int i;

	for(i = 0; i < nimage; i++) {
		imlib_context_set_image(images[i]);
		imlib_free_image();
	}
	imlib_context_set_image(buffer);
	imlib_free_image();
	XFreePixmap(dpy, pm);
}

void
configurenotify(XEvent *e) {
	XConfigureEvent *ev = &e->xconfigure;

	if(ev->window == root) {
		sw = ev->width;
		sh = ev->height;
		updategeom();
		imlib_context_set_image(buffer);
		imlib_free_image();
		buffer = imlib_create_image(sw, sh);
		XFreePixmap(dpy, pm);
		pm = XCreatePixmap(dpy, root, sw, sh, depth);
		imlib_context_set_drawable(pm);
		createbg();
		drawbg();
	}
}

void
createbg() {
	int i, j, w, h, tmp;
	Imlib_Image tmpimg;

	imlib_context_set_blend(1);
	for(j = i = 0; i < nmonitor; i++, j = i % nimage) {
		imlib_context_set_image(images[j]);
		w = imlib_image_get_width();
		h = imlib_image_get_height();
		tmpimg = imlib_clone_image();
		imlib_context_set_image(tmpimg);
		if((monitors[i].w > monitors[i].h && w < h) || (monitors[i].w < monitors[i].h && w > h)) {
			imlib_image_orientate(1);
			tmp = w;
			w = h;
			h = tmp;
		}
		imlib_context_set_image(buffer);
		imlib_blend_image_onto_image(tmpimg, 0, 0, 0, w, h, 
				monitors[i].x, monitors[i].y, monitors[i].w, monitors[i].h);
	}
}

void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void drawbg() {
	imlib_context_set_image(buffer);
	imlib_context_set_drawable(root);
	imlib_render_image_on_drawable(0, 0);
	imlib_context_set_drawable(pm);
	imlib_render_image_on_drawable(0, 0);
	XSetWindowBackgroundPixmap(dpy, root, pm);
}

void
run(void) {
	XEvent ev;

	createbg();
	drawbg();
	while(running) {
		XNextEvent(dpy, &ev);
		if(ev.type == ConfigureNotify )
			configurenotify(&ev);
	}
}

void
setup(char *paths[], int c) {
	int i;

	/* Loading images */
	for(nimage = i = 0; i < c && i < LENGTH(images); i++) {
		if((images[nimage] = imlib_load_image(paths[i])))
			nimage++;
		else {
			fprintf(stderr, "Warning: Cannot load file `%s`. Ignoring.\n", paths[nimage]);
			continue;
		}
	}
	if(nimage == 0)
		die("Error: No image to draw.\n");

	/* set up X */
	screen = DefaultScreen(dpy);
	vis = DefaultVisual(dpy, screen);
	depth = DefaultDepth(dpy, screen);
	cm = DefaultColormap(dpy, screen);
	root = RootWindow(dpy, screen);
	XSelectInput(dpy, root, StructureNotifyMask);
	bgcolor = BlackPixel(dpy, screen);
	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	pm = XCreatePixmap(dpy, root, sw, sh, depth);
	updategeom();

	/* set up Imlib */
	imlib_context_set_display(dpy);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_drawable(pm);
	buffer = imlib_create_image(sw, sh);
}

void
updategeom(void) {
#ifdef XINERAMA
	int i;
	XineramaScreenInfo *info = NULL;

	/* window area geometry */
	if(XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &nmonitor);
		nmonitor = MIN(nmonitor, LENGTH(monitors));
		for(i = 0; i < nmonitor; i++) {
			monitors[i].x = info[i].x_org;
			monitors[i].y = info[i].y_org;
			monitors[i].w = info[i].width;
			monitors[i].h = info[i].height;
		}
		XFree(info);
	}
	else
#endif
	{
		nmonitor = 1;
		monitors[0].x = sx;
		monitors[0].y = sy;
		monitors[0].w = sw;
		monitors[0].h = sh;
	}
}

int
main(int argc, char *argv[]) {
	if(argc < 2 || !strcmp(argv[1], "-h"))
		die("usage: bgs [-v] [IMAGE]...\n");
	else if(!strcmp(argv[1], "-v"))
		die("bgs-"VERSION", ?? 2006-2008 bgs engineers, see LICENSE for details\n");

	if(!(dpy = XOpenDisplay(0)))
		die("bgs: cannot open display\n");

	setup(argv + 1, argc - 1);
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
