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

var epg = {
   toggleKey: 113, /* F2 */

   init: function(){
      /* load the epg */
      epg.load();
   },

   /*
      Loads the epg data and generates the ui
   */
   load: function(){
      $.getJSON("/epg.json",function(data){
         var html = epg.getHeaderHtml(data)
                  + epg.getBodyHtml(data);

         $("#epg").html(html);
      });
   },

   /*
      Creates epg body html for all channels
      @return     html code for the body
   */
   getBodyHtml: function(data){
      var result = "<div class=\"body\" style=\"width:"
                     + (data.length*210) + "px\">";

      for(var c=0; c<data.length; c++){
         var channel = data[c];
         
         result += "<div class=\"channel\">";

         /* render channel programme html */
         for(var p=0; p<channel.programmeList.length; p++){
            var prog = channel.programmeList[p];

            result += "<div class=\"programme\">"
                     + "<div class=\"programmeborder\">"
                     + "<div class=\"programmetime\">" 
                     + app.formatTime(prog.start) + "</div>"
                     + "</div>"
                     + "<div class=\"programmebody\">"
                     + "<div class=\"programmetitle\">" 
                     + prog.title + "</div>"
                     + "<div class=\"programmetext\">" 
                     + prog.description + "</div>"
                     + "</div>"
                     + "</div>";
         }

         result += "</div>";
      }

      result += "</div>";

      return result;
   },

   /*
      Creates epg html header for all channels
      @return     html code for the header
   */
   getHeaderHtml: function(data){
      var result = "<div class=\"header\" style=\"width:"
                     + (data.length*210) + "px\">";

      for(var c=0; c<data.length; c++){
         var channel = data[c];
         
         result += "<div class=\"channel\">"
               + "<div class=\"channellogo\" "
               + "style=\"background-image:url('/logo/"
               + channel.logoFile + "');\"></div>"
               + "<div class=\"channelname\">" + channel.name + "</div>"
               + "</div>";
      }

      result += "</div>";

      return result;
   },

   /*
      Returns the header html for the epg
   */
   getHeader: function(data){

   }
}