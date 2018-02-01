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

   /*
      Start and stop time of the epg content
   */
   start: 0,
   stop: 0,

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

   handleKey: function(keyCode){
      var pageBrowseHeight = 60*5;

      switch(keyCode){
         case 38: /* KEY UP */
            /* select previous programme */
            var selectedProg = "#epg .channel .programme[data-selected=\"true\"]";
            if($(selectedProg).prev()){
               var nextElement = $(selectedProg).prev();

               $(selectedProg).removeAttr("data-selected");
               $(nextElement).attr("data-selected", "true");
            }            
            break;
         case 40: /* KEY DOWN */
            /* select next programme */
            var selectedProg = "#epg .channel .programme[data-selected=\"true\"]";
            if($(selectedProg).next()){
               var nextElement = $(selectedProg).next();

               $(selectedProg).removeAttr("data-selected");
               $(nextElement).attr("data-selected", "true");
            }
            break;
      }
   },

   /*
      Creates epg body html for all channels
      @return     html code for the body
   */
   getBodyHtml: function(data){
      var result = "<div class=\"body\" style=\"width:"
                     + (data.length*210) + "px\">"
                     + "<div class=\"content\">";

      /* top is now minus 10 minutes (600 secs) */
      epg.start = app.getLastHour();

      for(var c=0; c<data.length; c++){
         var channel = data[c];
         
         result += "<div class=\"channel\">";

         /* render channel programme html */
         var progNumber = 0;
         for(var p=0; p<channel.programmeList.length; p++){
            var prog = channel.programmeList[p];           

            if(prog.stop > epg.start){
               var height = (Math.round((prog.stop - prog.start)/60)*5)-2;
               var top = ((prog.start-epg.start)/60)*5;

               if(prog.stop > epg.stop){
                  epg.stop = prog.stop;
               }

               var selectedAttr = "";
               if(c == 0 && progNumber == 0){
                  selectedAttr = "data-selected=\"true\" ";
               }

               result += "<div class=\"programme\" "
                        + selectedAttr
                        + "style=\"top:" 
                        + top + "px;height:" 
                        + height + "px;\">"
                        + "<div class=\"programmeborder\" style=\"height:" 
                        + height + "px\">"
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
               
               progNumber++;
            }
         }

         result += "</div>";
      }

      /* close content element */
      result += "</div>";

      /* render epg ruler */
      result += epg.getTimelineHtml(epg.start,epg.stop);

      result += "</div>";

      return result;
   },

   /*
      Returns html for the ruler on the left
      @returns    rendered html code for the ruler
   */
   getTimelineHtml: function(start,stop){
      result = "<div class=\"timeline\">";

      var hour_ts = app.getLastHour();
      for(var t = hour_ts; t < stop; t = t + 3600){
         var timeObj = new Date((t)*1000);

         result += "<div class=\"hour\">" + timeObj.getHours() + ":00</div>";
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
   }
}