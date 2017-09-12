//modified by: Derrick Alden
//date:
//
//program: lab3 assignment
//author:  Gordon Griesel
//purpose: Fill areas with color
//
//handle the following program modes...
//
//mode:
// 0 = standby
// 1 = fill screen
// 2 = fill a disk (circle)
// 3 = fill a ring
// 4 = fill a rectangle
// 5 = fill triangle using 3 half-spaces
// 6 = fill triangle using odd-even method
// 7 = calculate PI by inscribing a circle within a square
// 8 = fill screen with checkerboard pattern
// 9 = graded color triangle using barycentric coordinates
// a = fill triangle with a texture-map using barycentric coordinates
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
//type defines
typedef double Flt;
typedef Flt Vec[3];
//some macros
#define random(a) rand()%(a)
#define rnd() (double)rand()/(double)RAND_MAX
#define SWAP(a,b) (a)^=(b);(b)^=(a);(a)^=(b)
#define ABS(a) (((a)<0)?(-(a)):(a))
#define SGN(a) (((a)<0)?(-1):(1))

class Global {
public:
	int xres, yres;
	int mode;
	int nPixels;
	Vec scale;
	Vec center;
  Vec center2;
	Flt radius;
  Flt radius2;
  Flt checkx;
  Flt checky;
	//triangle_vertices;
	Vec tri[3];
	Global() {
		srand((unsigned)time(NULL));
		xres = 800; yres = 800;
		mode = 0;
		nPixels = 1;
	}
} g;

class X11 {
private:
	Display *dpy;
	Window win;
	GC gc;
public:
	X11() {
		//constructor
		if (!(dpy=XOpenDisplay(NULL))) {
			fprintf(stderr, "ERROR: could not open display\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}
		int scr = DefaultScreen(dpy);
		win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 1, 1,
			g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
		XStoreName(dpy, win, "3480 filling areas");
		gc = XCreateGC(dpy, win, 0, NULL);
		XMapWindow(dpy, win);
		XSelectInput(dpy, win, ExposureMask | StructureNotifyMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			KeyPressMask | KeyReleaseMask);
	}
	~X11() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void flush() { XFlush(dpy); }
	void check_resize(XEvent *e) {
		//ConfigureNotify is sent when window size changes.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		g.xres = xce.width;
		g.yres = xce.height;
	}
	void set_title() {
		char ts[200];
		sprintf(ts, "3480 filling areas  nPixels: %i  mode: %i",
			g.nPixels, g.mode);
		XStoreName(dpy, win, ts);
	}
	void clear_screen() {
		XClearWindow(dpy, win);
	}
	void setColor3i(int r, int g, int b) {
		XSetForeground(dpy, gc, ((r<<16) + (g<<8) + b));
	}
	bool getXPending() {
		return (XPending(dpy));
	}
	void getXNextEvent(XEvent *e) {
		XNextEvent(dpy, e);
	}
	void drawPixel(int x, int y) {
		XDrawPoint(dpy, win, gc, x, y);
	}
	void drawLine(int x0, int y0, int x1, int y1) {
		XDrawLine(dpy, win, gc, x0, y0, x1, y1);
	}
	void fillRectangle(int x, int y, int w, int h) {
		XFillRectangle(dpy, win, gc, x, y, w, h);
	}
	void drawRectangle(int x, int y, int w, int h) {
		XDrawRectangle(dpy, win, gc, x, y, w, h);
	}
	void drawText(int x, int y, const char *text) {
		XDrawString(dpy, win, gc, x, y, text, strlen(text));
	}
} x11;

//function prototypes
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

int main()
{
	int done = 0;
	while (!done) {
		//Check the event queue
		while (x11.getXPending()) {
			//Handle them one-by-one
			XEvent e;
			x11.getXNextEvent(&e);
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		for (int i=0; i<g.nPixels; i++)
			render();
		x11.flush();
		usleep(99);
	}
	return 0;
}

void getPerpendicular(double *v1, double *v2)
{
	v2[0] =  v1[1];
	v2[1] = -v1[0];
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	int mx = e->xbutton.x;
	int my = e->xbutton.y;
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		//A mouse button was pressed.
		if (e->xbutton.button==1) {
			//Left button pressed
		}
		if (e->xbutton.button==3) {
			//Right button pressed. make a point.
		}
	}
	if (savex != mx || savey != my) {
		//mouse moved
		savex = mx;
		savey = my;
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyRelease)
		return 0;
	switch (key) {
		case XK_0:
			g.mode = 0;
			break;
		case XK_1:
			g.mode = 1;

      x11.clear_screen();
			break;
		case XK_2:
			g.mode = 2;
      //set x to random value
      g.center[0] = rnd() * g.xres;
      //set y to random value
      g.center[1] = rnd() * g.yres;
      //set radius to random value
      g.radius = rnd() * 100.0 + 20;
      x11.clear_screen();
			break;
		case XK_3:
      //ring
			g.mode = 3;
      g.center[0] = rnd() * g.xres;
      g.center[1] = rnd() * g.yres;
      g.center2[0] = g.center[0];
      g.center2[1] = g.center[1];
      g.radius = rnd() * 100.0 + 20;
      g.radius2 = g.radius - 10;
      x11.clear_screen();
			break;
    case XK_r:
      //crecent
      g.mode = 11;
      g.center[0] = rnd() * g.xres;
      g.center[1] = rnd() * g.yres;
      g.center2[0] = g.center[0] * rnd();
      g.center2[1] = g.center[1] * rnd();
      g.radius = rnd() * 100.0 + 20;
      g.radius2 = g.radius - 10;
      x11.clear_screen();
      break;
    case XK_4:
      //rectangle
      g.mode = 4;
      g.center[0] = rnd() * g.xres;
      g.center[1] = rnd() * g.yres;
      x11.clear_screen();
      break;
    case XK_8:
      g.mode = 8;
     // g.checkx = 100;
     // g.checky = 100;    
      g.checkx = rnd() * g.xres;
      g.checky = rnd() * g.yres;


      x11.clear_screen();
      break;
    
		case XK_c:
			x11.clear_screen();
			break;
		case XK_m:
			void showMenu();
			showMenu();
			break;
		case XK_minus:
			if (g.nPixels > 0)
				--g.nPixels;
			break;
		case XK_equal:
			++g.nPixels;
			break;
		case XK_Escape:
			//end the program
			return 1;
	}
	x11.set_title();
	return 0;
}

void physics()
{
	//no physics yet.
}

void showMenu()
{
	int i;
	int x=4, y=20;
	const char *text[] = {
	"Program Options",
	"0 = standby",
	"1 = fill screen",
	"2 = fill a disk (circle)",
	"3 = fill a ring",
	"4 = fill a rectangle",
	"5 = fill triangle using 3 half-spaces",
	"6 = fill triangle using odd-even method",
	"7 = calculate PI",
	"8 = fill screen with checkerboard pattern",
	"9 = graded color triangle using barycentric coordinates",
	"a = fill triangle with a texture-map using barycentric coordinates",
  "r = fill crecent"
	};
	x11.setColor3i(255,255,255);
	for (i=0; i<13; i++) {
		x11.drawText(x, y, text[i]);
		y+=16;
	}
	return;
}

void render()
{
  //render() makes random pixle
	//printf("render mode: %i\n", g.mode);
	int x = random(g.xres);
	int y = random(g.yres);
	x11.setColor3i(255, 255, 0);
	//
	switch(g.mode) {
	case 0:
		return;
	case 1:
		g.mode=1;
		break;
	case 2: {
		//circle
    //distance squr() + squrt()
    //differnce in x
    Flt dx = x - g.center[0];
    //diff in y
    Flt dy = y - g.center[1];
    //distance between circle 
    Flt distance = sqrt(dx*dx + dy*dy);
    if (distance < g.radius) {
       x11.setColor3i(255,0,0);
    }

		break;
          }
   case 3: {
 		//disk
    //distance squr() + squrt()
    Flt dx = x - g.center[0];
    Flt dx2 = x - g.center2[0];
    //diff in y
    Flt dy = y - g.center[1];
    Flt dy2 = y - g.center2[1];
    //distance between circle 
    Flt distance = sqrt(dx*dx + dy*dy);
    Flt distance2 = sqrt(dx2*dx2 + dy2*dy2);
    if (distance < g.radius) {
       x11.setColor3i(255,0,0);
    }
    if (distance2 < g.radius2) {
        x11.setColor3i(255,255,0);
    }
    break;
           }
	case 11: {
		//crescent
    //distance squr() + squrt()
    //differnce in x
    Flt dx = x - g.center[0];
    Flt dx2 = x - g.center2[0];
    //diff in y
    Flt dy = y - g.center[1];
    Flt dy2 = y - g.center2[1];
    //distance between circle 
    Flt distance = sqrt(dx*dx + dy*dy);
    Flt distance2 = sqrt(dx2*dx2 + dy2*dy2);
    if (distance < g.radius) {
       x11.setColor3i(255,0,0);
      if (distance2 < g.radius) {
        x11.setColor3i(0,255,0);
      }
    }
		break;
          }
	case 4: {
		//rectangle
    Flt dx = x - g.center[0];
    Flt dy = y - g.center[1];
    //distance between rec 
    Flt area = dx * dy;
    Flt distance = dx * dx;
    Flt distance2 = dy * dy;
    if (distance < g.center[0]) {
      if (distance2 < g.center[1]) {
          x11.setColor3i(255,0,0);
      }
    }
		break;
          }
	case 5:
		//Fill triangle using 3 half-spaces
		//if (threeHalfSpaces(g.tri, x, y))
		//	setColor3i(255,120,40);
		break;
	case 6:
		//Fill triangle using Odd-even rule
		//Determine if point at x,y is inside a triangle
		//Does a horizontal line from x,y intersect any triangle sides?
		break;
	case 7:
		//Calculate PI by inscribing circle in a square.
		break;
	case 8: {
    //Fill whole screen with checkerboard pattern

    Flt dx = x / (g.checkx*g.checky);
    Flt dy = y / g.checky;
        if(x % 2 == 0) {
        x11.setColor3i(0,0,0);
          if(y % 2 == 0) {
          x11.setColor3i(255,255,255);
          }
        }
      
   
    break;
          }
		//Fill whole screen with a checkerboard pattern.
    /*
    Flt dx = x / g.checkx;
    Flt dy = y / g.checky;
    Flt tempx = fmod(dx,dy);
    Flt tempy = fmod(dy,dx);
    if ( tempx == 1 ) {
      if ( tempy == 1) {
        x11.setColor3i(0,0,0);
      }
    }else {
    
      x11.setColor3i(255,255,255);
    }

		break;
          }
          */
	case 9:
		//Fill triangle using barycentric coordinates.
		//Flt u,v,w;
		//if (getBarycentricCoordinates(g.tri, x, y, &u, &v)) {
		//	//hint: you must calculate the w value here.
		//	setColor3f(u,v,w);
		break;
	case 10:
		//Fill triangle with texture map using barycentric coordinates
		//Flt u,v,w;
		//getBarycentricCoordinates(g.tri, x, y, &u, &v);
		//
		//Now use the u, v, and w values to get a color from
		//the image texture map.

		break;
	}
	x11.drawPixel(x, y);
}


























