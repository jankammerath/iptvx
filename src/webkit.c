/*

   Copyright 2018   Jan Kammerath

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <webkit2/webkit2.h>

SDL_Thread *webkit_thread;

static GtkWidget *iptvx_gtk_window;
static GtkWidget *iptvx_gtk_webview;
static WebKitUserContentManager* user_content_mgr;
static bool iptvx_webkit_ready;
static bool iptvx_overlay_rendering;

int iptvx_webkit_width;
int iptvx_webkit_height;

struct png_data{
    unsigned char* data;
    unsigned int length;
} typedef png_data;

static png_data overlay_data;
GByteArray* png_byte_data;
GByteArray* old_png_byte_data;

void (*load_finished_callback)(void*);

bool enable_webkit_inspector;

/*
  Defines whether the dev tool should be enabled or not
  @param    enable_tool    true to enable, false to disable
*/
void iptvx_webkit_enable_devtool(bool enable_tool){
  enable_webkit_inspector = enable_tool;
}

/* 
  returns overlay png data link 
  @return       pointer to PNG data
*/
void* iptvx_get_overlay_ptr(){
  return &overlay_data;
}

/*
  returns a char* with the current title of the webkit window
  @return       char* with the title text
*/
const char* iptvx_get_overlay_title(){
  return webkit_web_view_get_title(WEBKIT_WEB_VIEW(iptvx_gtk_webview));
}

/* 
  returns ptr to bool indicating if busy rendering
  @return        true when busy rendering, otherwise false
*/
bool* iptvx_get_overlay_rendering_ptr(){
  return &iptvx_overlay_rendering;
}

/* 
  returns ptr to bool indicating if busy
  @return        true when busy, otherwise false
*/
bool* iptvx_get_overlay_ready_ptr(){
  return &iptvx_webkit_ready;
}

/*
  function to write PNG data into GArray
*/
static cairo_status_t iptvx_webkit_snapshot_write_png (void *closure, const unsigned char *data, unsigned int length){
  g_byte_array_append (closure, data, length);
  return CAIRO_STATUS_SUCCESS;
}

/*
  callback when snapshot of window finished
*/
static void iptvx_webkit_snapshotfinished_callback(WebKitWebView *webview,GAsyncResult *res,char *destfile)
{
  GError *err = NULL;
  cairo_surface_t *surface = webkit_web_view_get_snapshot_finish(WEBKIT_WEB_VIEW(webview),res,&err);
  if (err) {
    printf("Failed generating the snapshot: %s\n",err->message);
  }

  iptvx_webkit_ready = false;

  /* flush data only when graphics is not currently rendering */
  if(iptvx_overlay_rendering == false){
    old_png_byte_data = png_byte_data;  
    png_byte_data = g_byte_array_new();
    cairo_surface_write_to_png_stream(surface,iptvx_webkit_snapshot_write_png,png_byte_data);
    overlay_data.data = png_byte_data->data;
    overlay_data.length = png_byte_data->len;

    /* free the byte array */
    g_byte_array_free(old_png_byte_data,true);
  }

  /* free the surface */
  cairo_surface_destroy(surface);

  /* set ready indicator to true */
  iptvx_webkit_ready = true;

  /* just wait 5ms to continue */
  usleep(5000);

  /* call for new snapshot */
  webkit_web_view_get_snapshot(webview,WEBKIT_SNAPSHOT_REGION_VISIBLE,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,NULL,
         (GAsyncReadyCallback)iptvx_webkit_snapshotfinished_callback,destfile);
}

/*
  Callback when load of webkit finished
*/
static void iptvx_webkit_loadchanged_callback (WebKitWebView *webview, WebKitLoadEvent status, char *destfile) {
  if (status != WEBKIT_LOAD_FINISHED) {
  	iptvx_webkit_ready = false;
    return;
  }

  /* fire callback when load finished */
  (*load_finished_callback)(webview);

  webkit_web_view_get_snapshot(webview,WEBKIT_SNAPSHOT_REGION_VISIBLE,
         WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND,NULL,
         (GAsyncReadyCallback)iptvx_webkit_snapshotfinished_callback,destfile);
}

/*
  starts webkit
  @param    file      the file to open
*/
int iptvx_webkit_start(void* file){
	iptvx_webkit_ready = false;

	gtk_init(NULL,NULL);
	iptvx_gtk_window = gtk_offscreen_window_new ();
	gtk_window_set_default_size(GTK_WINDOW(iptvx_gtk_window), 
                            iptvx_webkit_width, iptvx_webkit_height);
	
  /* create user content manager and web view with it 
    which is required to be able to fire js and retrieve 
    messages from the js interface */
  user_content_mgr = webkit_user_content_manager_new();
  iptvx_gtk_webview = webkit_web_view_new_with_user_content_manager(user_content_mgr);

	gtk_container_add (GTK_CONTAINER (iptvx_gtk_window), iptvx_gtk_webview);
	g_signal_connect (iptvx_gtk_webview, "load-changed",
	                G_CALLBACK (iptvx_webkit_loadchanged_callback), "");

  /* write all console messages to stdout */
  WebKitSettings *wk_setting = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(iptvx_gtk_webview));
  g_object_set (G_OBJECT(wk_setting), "enable-write-console-messages-to-stdout", TRUE, NULL);

	webkit_web_view_load_uri (WEBKIT_WEB_VIEW (iptvx_gtk_webview),(char*)file);
	gtk_widget_show_all (iptvx_gtk_window);
	gtk_main ();
}

/*
  starts webkit thread
  @param    file                          char ptr with the file path to the html app
  @param    width                         int defining the width of the webkit window
  @param    height                        int defining the height of the webkit window
  @param    load_finished_callback_func      ptr to func to call when load finished
*/
void iptvx_webkit_start_thread(char *file,int width, int height, void (*load_finished_callback_func)(void*)){
  iptvx_webkit_ready = false;
  iptvx_webkit_width = width;
  iptvx_webkit_height = height;
  load_finished_callback = load_finished_callback_func;
  webkit_thread = SDL_CreateThread(iptvx_webkit_start,file);
}