#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mutex.h>

#include <webkit2/webkit2.h>

#include <vlc/vlc.h>

#define WIDTH 1280
#define HEIGHT 720

#define VIDEOWIDTH 1280
#define VIDEOHEIGHT 720

struct ctx
{
    SDL_Surface *surf;
    SDL_mutex *mutex;
};

typedef struct
{
    unsigned char *pos;
    unsigned char *end;
} closure_t;

pthread_t webkit_thread;

GtkWidget *gtk_window;
GtkWidget *gtk_webview;
unsigned char overlay_png_data[5000];
bool webkit_is_writing;

static void *lock(void *data, void **p_pixels)
{
    struct ctx *ctx = data;

    SDL_LockMutex(ctx->mutex);
    SDL_LockSurface(ctx->surf);
    *p_pixels = ctx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
    struct ctx *ctx = data;

    SDL_UnlockSurface(ctx->surf);
    SDL_UnlockMutex(ctx->mutex);

    assert(id == NULL); /* picture identifier, not needed here */
}

static void display(void *data, void *id)
{
    /* VLC wants to display the video */
    (void) data;
    assert(id == NULL);
}

static cairo_status_t webkit_write_png_array(GString * string, const unsigned char *data, unsigned int length)
{
    //g_string_append_len(string, (const gchar *) data, length);
    return CAIRO_STATUS_SUCCESS;
}

static void webkit_snapshotfinished_callback(WebKitWebView *webview,GAsyncResult *res,char *destfile)
{
    GError *err = NULL;
    cairo_surface_t *surface = webkit_web_view_get_snapshot_finish(WEBKIT_WEB_VIEW(webview),res,&err);
    if (err) {
    printf("An error happened generating the snapshot: %s\n",err->message);
    }

    webkit_is_writing = true;
    GString * imgStr = g_string_sized_new(102400);
    cairo_surface_write_to_png(surface,"/tmp/vlcoverlay.png");
    webkit_is_writing = false;
    //cairo_surface_write_to_png_stream(surface,(cairo_write_func_t)webkit_write_png_array,&imgStr);

    usleep(20000);

    webkit_web_view_get_snapshot(webview,
         WEBKIT_SNAPSHOT_REGION_FULL_DOCUMENT,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,
         NULL,
         (GAsyncReadyCallback)webkit_snapshotfinished_callback,destfile);
}

static void webkit_loadchanged_callback (WebKitWebView *webview, WebKitLoadEvent status, char *destfile) {
  if (status != WEBKIT_LOAD_FINISHED) {
    return;
  }

  webkit_web_view_get_snapshot(webview,
         WEBKIT_SNAPSHOT_REGION_FULL_DOCUMENT,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,
         NULL,
         (GAsyncReadyCallback)webkit_snapshotfinished_callback,destfile);
}

static void * webkit_start(void* file){
    webkit_is_writing = true;
    gtk_init(NULL,NULL);
    gtk_window = gtk_offscreen_window_new ();
    gtk_window_set_default_size(GTK_WINDOW(gtk_window), 1280, 720);
    gtk_webview = webkit_web_view_new();
    gtk_container_add (GTK_CONTAINER (gtk_window), gtk_webview);
    g_signal_connect (gtk_webview, "load-changed",
                    G_CALLBACK (webkit_loadchanged_callback), "");

    // Ignore any SSL errors. Don't makes sense to abort taking a snapshoot because of a bad cert.
    webkit_web_context_set_tls_errors_policy(webkit_web_view_get_context(WEBKIT_WEB_VIEW (gtk_webview)),
                                           WEBKIT_TLS_ERRORS_POLICY_IGNORE);

  char *filename = (char*)file;
    char *url = g_strjoin("","file://",filename,NULL);
    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (gtk_webview),url);
    gtk_widget_show_all (gtk_window);
    gtk_main ();
}

void webkit_start_thread(char *file){
  if(pthread_create(&webkit_thread,NULL,webkit_start,(void *)file) != 0) {
    fprintf (stderr, "Failed to launch thread for WebKit.\n");
    exit (EXIT_FAILURE);
  } 
}

int main(int argc, char *argv[])
{
    webkit_start_thread("/home/jan/Development/VLC/overlay.html");

    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] =
    {
        "--no-xlib", /* tell VLC to not use Xlib */
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

    SDL_Surface *screen, *overlay, *background;
    SDL_Event event;
    SDL_Rect rect;
    int done = 0, action = 0, pause = 0, n = 0;

    struct ctx ctx;

    if(argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*
     *  Initialise libSDL
     */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
    {
        printf("cannot initialize SDL\n");
        return EXIT_FAILURE;
    }


    ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT,
                                    16, 0x001f, 0x07e0, 0xf800, 0);

    ctx.mutex = SDL_CreateMutex();

    int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;

    screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, options);
    if(!screen)
    {
        printf("cannot set video mode\n");
        return EXIT_FAILURE;
    }

    IMG_Init( IMG_INIT_PNG );
    SDL_Surface* loadedBgSurface = IMG_Load("/home/jan/Development/VLC/background.png");
    background = SDL_ConvertSurface( loadedBgSurface, screen->format, 0 );

    /*
     *  Initialise libVLC
     */
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    m = libvlc_media_new_path(libvlc, argv[1]);
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    libvlc_video_set_callbacks(mp, lock, unlock, display, &ctx);
    libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    libvlc_media_player_play(mp);

    /*
     *  Main loop
     */
    rect.w = 0;
    rect.h = 0;

    while(!done)
    { 
        action = 0;

        /* Keys: enter (fullscreen), space (pause), escape (quit) */
        while( SDL_PollEvent( &event ) ) 
        { 
            switch(event.type)
            {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_KEYDOWN:
                action = event.key.keysym.sym;
                break;
            }
        }

        switch(action)
        {
        case SDLK_ESCAPE:
            done = 1;
            break;
        case SDLK_RETURN:
            options ^= SDL_FULLSCREEN;
            screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
            break;
        case ' ':
            pause = !pause;
            break;
        }

        rect.x = 0;
        rect.y = 0;

        if(!pause)
            n++;

        while(webkit_is_writing){
            usleep(1000);
        }

        SDL_RWops *overlay_rwops = SDL_RWFromFile("/tmp/vlcoverlay.png","rb");
        //SDL_RWops *overlay_rwops = SDL_RWFromMem(overlay_png_data,sizeof(overlay_png_data));
        overlay = IMG_LoadPNG_RW(overlay_rwops);

        /* Blitting the surface does not prevent it from being locked and
         * written to by another thread, so we use this additional mutex. */
        SDL_LockMutex(ctx.mutex);
        SDL_BlitSurface(background, NULL, screen, NULL);
        SDL_BlitSurface(ctx.surf, NULL, screen, NULL);
        SDL_BlitSurface(overlay, NULL, screen, NULL);
        SDL_UnlockMutex(ctx.mutex);

        SDL_Flip(screen);
        SDL_Delay(10);
    }

    /*
     * Stop stream and clean up libVLC
     */
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    /*
     * Close window and clean up libSDL
     */
    SDL_DestroyMutex(ctx.mutex);
    SDL_FreeSurface(background);
    SDL_FreeSurface(ctx.surf);
    SDL_FreeSurface(overlay);

    SDL_Quit();

    return 0;
}