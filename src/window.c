#include <pthread.h>
#include <X11/Xlib.h>

static pthread_t win_thread;

/* creates the main window for this application */
static void * xiptv_create_window(void* arg){
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
    frame_window = XCreateWindow(display, XRootWindow(display, 0),
                                 0, 0, 1280, 720, 5, depth,
                                 InputOutput, visual, CWBackPixel,
                                 &frame_attributes);
    XStoreName(display, frame_window, "xiptv");
    XSelectInput(display, frame_window, ExposureMask | StructureNotifyMask);

    fontinfo = XLoadQueryFont(display, "10x20");
    gr_values.font = fontinfo->fid;
    gr_values.foreground = XWhitePixel(display, 0);
    graphical_context = XCreateGC(display, frame_window, 
                                  GCFont+GCForeground, &gr_values);
    XMapWindow(display, frame_window);

    while ( 1 ) {
        XNextEvent(display, (XEvent *)&event);
        switch ( event.type ) {
            case Expose:
            {
                XWindowAttributes window_attributes;
                XGetWindowAttributes(display, frame_window, &window_attributes);
                break;
            }
            default:
                break;
        }
    }
}

/* create and process the window in separate thread */
void xiptv_create_window_thread(){
	if(pthread_create(&win_thread,NULL,xiptv_create_window,NULL) != 0) {
		fprintf (stderr, "Failed to launch thread for main window.\n");
        exit (EXIT_FAILURE);
	}	
}
