#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <webkit2/webkit2.h>

SDL_Thread *webkit_thread;

static GtkWidget *iptvx_gtk_window;
static GtkWidget *iptvx_gtk_webview;
static bool iptvx_webkit_ready;

struct png_data{
    unsigned char* data;
    unsigned int length;
} typedef png_data;

static png_data overlay_data;
GByteArray* png_byte_data;
GByteArray* old_png_byte_data;

/* returns overlay png data link */
void* iptvx_get_overlay_ptr(){
  return &overlay_data;
}

/* returns ptr to bool indicating if busy */
bool* iptvx_get_overlay_ready_ptr(){
  return &iptvx_webkit_ready;
}

/* sends a key to the browser */
extern void iptvx_webkit_sendkey(int keyCode){
  char scriptKeyEvent[512];

  char keyName[2];
  keyName[0] = (char)keyCode;
  keyName[1] = '\0';
  char* xkeyName = "a";
  sprintf(scriptKeyEvent,"var e = new Event(\"keydown\");"
        "e.key=\"%s\";e.keyCode=%d;"
        "e.which=%d;e.altKey=false;e.ctrlKey=false;"
        "e.shiftKey=false;e.metaKey=false;e.bubbles=true;"
        "window.dispatchEvent(e);",keyName,keyCode,keyCode);

  webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(iptvx_gtk_webview),
                    scriptKeyEvent,NULL,NULL,NULL);
}

static cairo_status_t iptvx_webkit_snapshot_write_png (void *closure, const unsigned char *data, unsigned int length){
  g_byte_array_append (closure, data, length);
  return CAIRO_STATUS_SUCCESS;
}

static void iptvx_webkit_snapshotfinished_callback(WebKitWebView *webview,GAsyncResult *res,char *destfile)
{
  GError *err = NULL;
  cairo_surface_t *surface = webkit_web_view_get_snapshot_finish(WEBKIT_WEB_VIEW(webview),res,&err);
  if (err) {
    printf("An error happened generating the snapshot: %s\n",err->message);
  }

  iptvx_webkit_ready = false;

  old_png_byte_data = png_byte_data;  
  png_byte_data = g_byte_array_new();
  cairo_surface_write_to_png_stream(surface,iptvx_webkit_snapshot_write_png,png_byte_data);
  overlay_data.data = png_byte_data->data;
  overlay_data.length = png_byte_data->len;

  /* free the surface */
  cairo_surface_destroy(surface);

  iptvx_webkit_ready = true;

  usleep(1000);

  g_byte_array_free(old_png_byte_data,true);

  webkit_web_view_get_snapshot(webview,WEBKIT_SNAPSHOT_REGION_VISIBLE,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,NULL,
         (GAsyncReadyCallback)iptvx_webkit_snapshotfinished_callback,destfile);
}

static void iptvx_webkit_loadchanged_callback (WebKitWebView *webview, WebKitLoadEvent status, char *destfile) {
  if (status != WEBKIT_LOAD_FINISHED) {
  	iptvx_webkit_ready = false;
    return;
  }

  webkit_web_view_get_snapshot(webview,WEBKIT_SNAPSHOT_REGION_VISIBLE,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,NULL,
         (GAsyncReadyCallback)iptvx_webkit_snapshotfinished_callback,destfile);
}

int iptvx_webkit_start(void* file){
	iptvx_webkit_ready = false;

	gtk_init(NULL,NULL);
	iptvx_gtk_window = gtk_offscreen_window_new ();
	gtk_window_set_default_size(GTK_WINDOW(iptvx_gtk_window), 1280, 720);
	iptvx_gtk_webview = webkit_web_view_new();
	gtk_container_add (GTK_CONTAINER (iptvx_gtk_window), iptvx_gtk_webview);
	g_signal_connect (iptvx_gtk_webview, "load-changed",
	                G_CALLBACK (iptvx_webkit_loadchanged_callback), "");

	// Ignore any SSL errors. Don't makes sense to abort taking a snapshoot because of a bad cert.
	webkit_web_context_set_tls_errors_policy(webkit_web_view_get_context(WEBKIT_WEB_VIEW (iptvx_gtk_webview)),
	                                       WEBKIT_TLS_ERRORS_POLICY_IGNORE);

  char *filename = (char*)file;
	char *url = g_strjoin("","file://",filename,NULL);
	webkit_web_view_load_uri (WEBKIT_WEB_VIEW (iptvx_gtk_webview),url);
	gtk_widget_show_all (iptvx_gtk_window);
	gtk_main ();
}

void iptvx_webkit_start_thread(char *file){
  iptvx_webkit_ready = false;
  webkit_thread = SDL_CreateThread(iptvx_webkit_start,file);
}