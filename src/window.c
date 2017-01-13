#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

static pthread_t win_thread;

GtkWidget *window;
GtkApplication *app;

int iptvx_window_xid;

static void iptvx_window_activate (GtkApplication* app, gpointer user_data){
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "iptvx");
  gtk_window_set_default_size (GTK_WINDOW (window), 1280, 720);
  gtk_widget_show_all (window);
  iptvx_window_xid = GDK_WINDOW_XID(gtk_widget_get_window(window));
}

/* creates the main window for this application */
static void * iptvx_create_window(void* window_argument){
    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (iptvx_window_activate), NULL);
    g_application_run (G_APPLICATION (app),0,NULL);
    g_object_unref (app);
}

void iptvx_create_window_thread(){
    iptvx_window_xid = -1;

    if(pthread_create(&win_thread,NULL,iptvx_create_window,NULL) != 0) {
        fprintf (stderr, "Failed to launch thread for main window.\n");
        exit (EXIT_FAILURE);
    }   
}

int iptvx_get_window_xid(){
    return iptvx_window_xid;
}
