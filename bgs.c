/* See LICENSE file for copyright and license details.
 *
 * To understand bgs , start reading main().
 */
#include <getopt.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

/* macros */
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define LENGTH(x)       (sizeof x / sizeof x[0])

/* image modes */
enum { ModeCenter, ModeZoom, ModeScale, ModeLast };

/* typedefs */
typedef struct {
	int x, y, w, h;
} Monitor;

/* function declarations */
static void cleanup(void);                                /* frees images before exit. */
static void die(const char *errstr);                      /* prints errstr to strerr and exits. */
static void drawbg(void);                                 /* draws background to root. */
static void run(void);                                    /* main loop */
static void setup(char *paths[], int c, const char *col); /* sets up imlib and X */
static void updategeom(void);                             /* updates screen and/or Xinerama
                                                             dimensions */

/* variables */
static int sx, sy, sw, sh;	/* geometry of the screen */
static unsigned int mode = ModeScale;	/* image mode */
static Bool rotate = True;
static Bool running = False;
static Display *dpy;
static Window root;
static int nmonitor, nimage;	/* Amount of monitors/available background
				   images */
static Monitor monitors[8];
static Imlib_Image images[LENGTH(monitors)];

/* function implementations */
void
cleanup(void) {
	int i;

	for(i = 0; i < nimage; i++) {
		imlib_context_set_image(images[i]);
		imlib_free_image_and_decache();
	}
}

void
die(const char *errstr) {
	fputs(errstr, stderr);
	exit(EXIT_FAILURE);
}

void
drawbg(void) {
	int i, w, h, nx, ny, nh, nw, tmp;
	double factor;
	Pixmap pm;
	Imlib_Image tmpimg, buffer;

	pm = XCreatePixmap(dpy, root, sw, sh, DefaultDepth(dpy,
				DefaultScreen(dpy)));
	if(!(buffer = imlib_create_image(sw, sh)))
		die("Error: Cannot allocate buffer.\n");
	imlib_context_set_image(buffer);
	imlib_image_fill_rectangle(0, 0, sw, sh);
	imlib_context_set_blend(1);
	for(i = 0; i < nmonitor; i++) {
		imlib_context_set_image(images[i % nimage]);
		w = imlib_image_get_width();
		h = imlib_image_get_height();
		if(!(tmpimg = imlib_clone_image()))
			die("Error: Cannot clone image.\n");
		imlib_context_set_image(tmpimg);
		if(rotate && ((monitors[i].w > monitors[i].h && w < h) ||
				(monitors[i].w < monitors[i].h && w > h))) {
			imlib_image_orientate(1);
			tmp = w;
			w = h;
			h = tmp;
		}
		imlib_context_set_image(buffer);
		switch(mode) {
		case ModeCenter:
			nw = (monitors[i].w - w) / 2;
			nh = (monitors[i].h - h) / 2;
			nx = monitors[i].x + (monitors[i].w - nw) / 2;
			ny = monitors[i].y + (monitors[i].h - nh) / 2;
			break;
		case ModeZoom:
			nw = monitors[i].w;
			nh = monitors[i].h;
			if(w > h && (w / h > (monitors[i].w / monitors[i].h))) {
				nx = monitors[i].x + (monitors[i].w - nw) / 2;
				ny = monitors[i].y + (int)ceil(h * nx / w) / 2;
			}
			else {
				ny = monitors[i].y + (monitors[i].h - nh) / 2;
				nx = monitors[i].x + (int)ceil(w * ny / h) / 2;
			}
			break;
		default: /* ModeScale */
			factor = MAX((double)w / monitors[i].w,
					(double)h / monitors[i].h);
			nw = w / factor;
			nh = h / factor;
			nx = monitors[i].x + (monitors[i].w - nw) / 2;
			ny = monitors[i].y + (monitors[i].h - nh) / 2;
		}
		imlib_blend_image_onto_image(tmpimg, 0, 0, 0, w, h,
				nx, ny, nw, nh);
		imlib_context_set_image(tmpimg);
		imlib_free_image();
	}
	imlib_context_set_blend(0);
	imlib_context_set_image(buffer);
	imlib_context_set_drawable(root);
	imlib_render_image_on_drawable(0, 0);
	imlib_context_set_drawable(pm);
	imlib_render_image_on_drawable(0, 0);
	XSetWindowBackgroundPixmap(dpy, root, pm);
	imlib_context_set_image(buffer);
	imlib_free_image_and_decache();
	XFreePixmap(dpy, pm);
}

void
run(void) {
	XEvent ev;


	for(;;) {
		updategeom();
		drawbg();
		if(!running)
			break;
		imlib_flush_loaders();
		XNextEvent(dpy, &ev);
		if(ev.type == ConfigureNotify) {
			sw = ev.xconfigure.width;
			sh = ev.xconfigure.height;
			imlib_flush_loaders();
		}
	}
}

void
setup(char *paths[], int c, const char *col) {
	Visual *vis;
	Colormap cm;
	XColor color;
	int i, screen;

	/* Loading images */
	for(nimage = i = 0; i < c && i < LENGTH(images); i++) {
		if((images[nimage] = imlib_load_image_without_cache(paths[i])))
			nimage++;
		else {
			fprintf(stderr, "Warning: Cannot load file `%s`. "
					"Ignoring.\n", paths[nimage]);
			continue;
		}
	}
	if(nimage == 0)
		die("Error: No image to draw.\n");

	/* set up X */
	screen = DefaultScreen(dpy);
	vis = DefaultVisual(dpy, screen);
	cm = DefaultColormap(dpy, screen);
	root = RootWindow(dpy, screen);
	XSelectInput(dpy, root, StructureNotifyMask);
	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);

	if(!XAllocNamedColor(dpy, cm, col, &color, &color))
		die("Error: Cannot allocate color.\n");

	/* set up Imlib */
	imlib_context_set_display(dpy);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_color(color.red, color.green, color.blue, 255);
}

void
updategeom(void) {
#ifdef XINERAMA
	int i;
	XineramaScreenInfo *info = NULL;

	if(XineramaIsActive(dpy) &&
			(info = XineramaQueryScreens(dpy, &nmonitor))) {
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
	int opt;
	const char *col = NULL;

	while((opt = getopt(argc, argv, "cC:Rvxz")) != -1)
		switch(opt) {
		case 'c':
			mode = ModeCenter; break;
		case 'C':
			col = optarg; break;
		case 'R':
			rotate = False; break;
		case 'v':
			printf("bgs-"VERSION", © 2010 bgs engineers, see "
					"LICENSE for details\n");
			return 0;
		case 'x':
			running = True; break;
		case 'z':
			mode = ModeZoom; break;
		case '?': /* Fallthrough */
		default:
			die("usage: bgs [-v] [-c] [-C hex] [-z] [-R] [-x] [IMAGE]...\n");
		}
	argc -= optind;
	argv += optind;

	if(!col)
		col = "#000000";
	if(!(dpy = XOpenDisplay(NULL)))
		die("bgs: cannot open display\n");
	setup(argv, argc, col);
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
