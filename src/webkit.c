#include <webkit2/webkit2.h>

static GtkWidget *iptvx_gtk_window;
static GtkWidget *iptvx_gtk_webview;
static bool iptvx_webkit_ready;

static void iptvx_webkit_snapshotfinished_callback(WebKitWebView *webview,GAsyncResult *res,char *destfile)
{
  GError *err = NULL;
  cairo_surface_t *surface = webkit_web_view_get_snapshot_finish(WEBKIT_WEB_VIEW(webview),res,&err);
  if (err) {
    printf("An error happened generating the snapshot: %s\n",err->message);
  }

  cairo_surface_write_to_png (surface, "/home/jan/Bilder/iptvx-webkit.png");
}

static void iptvx_webkit_loadchanged_callback (WebKitWebView *webview, WebKitLoadEvent status, char *destfile) {
  if (status != WEBKIT_LOAD_FINISHED) {
  	iptvx_webkit_ready = false;
    return;
  }

  iptvx_webkit_ready = true;

  webkit_web_view_get_snapshot(webview,
         WEBKIT_SNAPSHOT_REGION_FULL_DOCUMENT,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,
         NULL,
         (GAsyncReadyCallback)iptvx_webkit_snapshotfinished_callback,destfile);
}

void iptvx_webkit_start(char *file){
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

	char *url = str_concat("file://",file);
	webkit_web_view_load_uri (WEBKIT_WEB_VIEW (iptvx_gtk_webview),url);
	gtk_widget_show_all (iptvx_gtk_window);
	gtk_main ();
}