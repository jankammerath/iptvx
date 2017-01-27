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
void (*controlMessageCallback)(void*);

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
      controlMessageCallback(str_value);
   } else {
      printf("Unexpected JS API message return value\n");
   }
 
   webkit_javascript_result_unref (message);
}

/*
   Initialises the JavaScript API interface
   @param   webview                          the webview to work on
   @param   controlMessageCallbackFunc       callback func when control msgs arrive
*/
void iptvx_js_init(WebKitWebView* webView,void (*controlMessageCallbackFunc)(void*)){
   /* define the control message callback */
   controlMessageCallback = controlMessageCallbackFunc;

   /* set global js webview variable */
   js_view = webView;

   /* attach the JS message handler */
   WebKitUserContentManager* user_content = webkit_web_view_get_user_content_manager(js_view);
   g_signal_connect (user_content, "script-message-received::iptvxexec",
                        G_CALLBACK (iptvx_control_message_received_cb), NULL);
   webkit_user_content_manager_register_script_message_handler(user_content, "iptvxexec");

   /* define the basic js object */
   char* jsObject =  " var iptvx = { epg: [], channel: '', "
                     " exec: function(cmd){ "
                     " window.webkit.messageHandlers.iptvxexec.postMessage(cmd); "
                     " } "
                     " }";

   /* fire the js object definition */
   webkit_web_view_run_javascript(js_view,jsObject,NULL,NULL,NULL);
}

/*
   Sends a key down event to the js application
   @param   keyCode           the code of the key to transmit
*/
void iptvx_js_sendkey(int keyCode){
  char scriptKeyEvent[512];

  char keyName[2];
  keyName[0] = (char)keyCode;
  keyName[1] = '\0';
  sprintf(scriptKeyEvent,"var e = new Event(\"keydown\");"
        "e.key=\"%s\";e.keyCode=%d;"
        "e.which=%d;e.altKey=false;e.ctrlKey=false;"
        "e.shiftKey=false;e.metaKey=false;e.bubbles=true;"
        "window.dispatchEvent(e);",keyName,keyCode,keyCode);

  webkit_web_view_run_javascript(js_view,scriptKeyEvent,NULL,NULL,NULL);
}