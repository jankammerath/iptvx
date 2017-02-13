/*

   Copyright 2017   Jan Kammerath

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
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>

WebKitWebView* js_view;
void (*control_message_callback)(void*);

static void iptvx_control_message_received_cb (WebKitUserContentManager *manager,
                                                 WebKitJavascriptResult *message,
                                                 gpointer user_data){      
   /* check if the message is valid */
   if (!message) {
      printf("Received JS API message invalid\n");
      return;
   }

   JSGlobalContextRef context = webkit_javascript_result_get_global_context(message);
   JSValueRef value = webkit_javascript_result_get_value(message);
 
   /* ensure received message is a string */
   if (JSValueIsString (context, value)) {
      JSStringRef js_str_value;
      gchar      *str_value;
      gsize       str_length;

      js_str_value = JSValueToStringCopy (context, value, NULL);

      str_length = JSStringGetMaximumUTF8CStringSize(js_str_value); // crashes here
      str_value = (gchar *)g_malloc (str_length);

      JSStringGetUTF8CString (js_str_value, str_value, str_length);
      JSStringRelease (js_str_value);

      /* execute the callback for the control message */
      control_message_callback(str_value);
   } else {
      printf("Unexpected JS API message return value\n");
   }
 
   webkit_javascript_result_unref (message);
}

/*
   Initialises the JavaScript API interface
   @param   webview                          the webview to work on
   @param   control_message_callback_func    callback func when control msgs arrive
*/
void iptvx_js_init(WebKitWebView* webView,void (*control_message_callback_func)(void*)){
   /* define the control message callback */
   control_message_callback = control_message_callback_func;

   /* set global js webview variable */
   js_view = webView;

   /* attach the JS message handler */
   WebKitUserContentManager* user_content = webkit_web_view_get_user_content_manager(js_view);
   g_signal_connect (user_content, "script-message-received::iptvxexec",
                        G_CALLBACK (iptvx_control_message_received_cb), NULL);
   webkit_user_content_manager_register_script_message_handler(user_content, "iptvxexec");

   /* define the basic js object */
   char* jsObject =  " var iptvx = { epgLoaded: 0, epg: [], "
                     " channel: 0, state: 0, volume: 100, "
                     " exec: function(cmd){ "
                     " window.webkit.messageHandlers.iptvxexec.postMessage(cmd); "
                     " } "
                     " }";

   /* fire the js object definition */
   webkit_web_view_run_javascript(js_view,jsObject,NULL,NULL,NULL);
}

/*
  Updates the volume info in the JS api
  @param      percentage        current volume percentage
*/
void iptvx_js_update_volume(int percentage){
  char jsCode[100];

  sprintf(jsCode,"iptvx.volume = %d;",percentage);
  webkit_web_view_run_javascript(js_view,jsCode,NULL,NULL,NULL);  
}

/*
  Signals current epg status to the js app
  @param      percentage        int percentage value of progress
*/
void iptvx_js_update_epg_status(int percentage){
  char jsCode[100];

  sprintf(jsCode,"iptvx.epgLoaded = %d;",percentage);
  webkit_web_view_run_javascript(js_view,jsCode,NULL,NULL,NULL);
}

/*
  Signals media state to the js api
  @param      current_state        defines current media state
*/
void iptvx_js_update_state(int current_state){
  char jsCode[100];

  sprintf(jsCode,"iptvx.state = %d;",current_state);
  webkit_web_view_run_javascript(js_view,jsCode,NULL,NULL,NULL);
}

/*
  Signals the current channel to the JS app
  @param      channel_id        int value with channel list index
*/
void iptvx_js_set_current_channel(int channel_id){
  char jsCode[100];

  sprintf(jsCode,"iptvx.channel = %d;",channel_id);
  webkit_web_view_run_javascript(js_view,jsCode,NULL,NULL,NULL);
}

/*
  Signals the epg data to the JS app
  @param      epg_data        JSON Array with complete epg data
*/
void iptvx_js_set_epg_data(GString* epg_data){
  GString* jsCode = g_string_new("");
  g_string_printf(jsCode,"iptvx.epg = %s;",epg_data->str);
  webkit_web_view_run_javascript(js_view,jsCode->str,NULL,NULL,NULL);
  g_string_free(jsCode,true);
}

/*
   Sends a key down event to the js application
   @param   keyCode           the code of the key to transmit
*/
void iptvx_js_sendkey(int keyCode){
  char scriptKeyEvent[512];

  sprintf(scriptKeyEvent,"var e = new Event(\"keydown\");"
        "e.key=\"\";e.keyCode=%d;"
        "e.which=%d;e.altKey=false;e.ctrlKey=false;"
        "e.shiftKey=false;e.metaKey=false;e.bubbles=true;"
        "window.dispatchEvent(e);",keyCode,keyCode);

  webkit_web_view_run_javascript(js_view,scriptKeyEvent,NULL,NULL,NULL);
}