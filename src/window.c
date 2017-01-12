#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <X11/Xlib.h>

static pthread_t win_thread;
static int xiptv_window_id;

/* creates the main window for this application */
static void * iptvx_create_window(void* window_argument){
    Display                 *display;
    Visual                  *visual;
    int                     depth;
    int                     text_x;
    int                     text_y;
    XSetWindowAttributes    frame_attributes;
    Window                  frame_window;
    XFontStruct             *fontinfo;
    XGCValues               gr_values;
    GC                      graphical_context;
    XKeyEvent               event;

    display = XOpenDisplay(NULL);
    visual = DefaultVisual(display, 0);
    depth  = DefaultDepth(display, 0);
    
    frame_attributes.background_pixel = XBlackPixel(display, 0);
    /* create the application window */
    frame_window = XCreateWindow(display, XRootWindow(display, 0),
                                 0, 0, 1280, 720, 5, depth,
                                 InputOutput, visual, CWBackPixel,
                                 &frame_attributes);
    
    /* set the window id */
    xiptv_window_id = frame_window;

    XStoreName(display, frame_window, "iptvx");
    XSelectInput(display, frame_window, ExposureMask | StructureNotifyMask);

    fontinfo = XLoadQueryFont(display, "10x20");
    gr_values.font = fontinfo->fid;
    gr_values.foreground = XBlackPixel(display, 0);
    graphical_context = XCreateGC(display, frame_window, 
                                  GCFont+GCForeground, &gr_values);
    XMapWindow(display, frame_window);

    while ( 1 ) {
        XNextEvent(display, (XEvent *)&event);
        switch ( event.type ) {
            case Expose:
            {
                break;
            }
            default:
                break;
        }
    }
}

/* create and process the window in separate thread */
void iptvx_create_window_thread(){
	if(pthread_create(&win_thread,NULL,iptvx_create_window,NULL) != 0) {
		fprintf (stderr, "Failed to launch thread for main window.\n");
        exit (EXIT_FAILURE);
	}	
}
