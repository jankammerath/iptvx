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

app.streamconfig = {
   visible: false,
   animating: false,
   count: 0,
   selected: 0,

   update: function(){
      if(typeof(iptvx)=="object"){
         var html = "";

         /* reset element counter */
         app.streamconfig.count = iptvx.trackList.length 
                              + iptvx.subtitleList.length;
         var trackid = 0;

         /* get all audio tracks available */
         for(var i=0;i<iptvx.trackList.length;i++){
            var track = iptvx.trackList[i];

            var classList = "configitem audiotrackitem";
            if(track.active == true){
               classList += " configitemactive";
            }

            if(app.streamconfig.selected == trackid){
               classList += " configitemselected";
            }

            html += "<div id=\"track" + trackid + "\" "
                  + "class=\""+classList+"\" "
                  + "data-tracktype=\"audiotrack\" "
                  + "data-trackid=\"" + track.id + "\">"
                  + track.name + "</div>";
            trackid++;
         }

         /* get all subtitle tracks available */
         for(var i=0;i<iptvx.subtitleList.length;i++){
            var subtitle = iptvx.subtitleList[i];

            var classList = "configitem subtitleitem";
            if(subtitle.active == true){
               classList += " configitemactive";
            }

            if(app.streamconfig.selected == trackid){
               classList += " configitemselected";
            }

            html += "<div id=\"track" + trackid + "\" " 
                  + "class=\""+classList+"\" "
                  + "data-tracktype=\"subtitle\" "
                  + "data-trackid=\"" + subtitle.id + "\">"
                  + subtitle.name + "</div>";
            trackid++;
         }

         /* smash html into the container */
         $("#streamconfig").html(html);
         app.streamconfig.resize();
      }
   },

   /* handles switching audio tracks and subtitles */
   switchTrack: function(){
      var trackid = 0;

      /* go through all audio tracks to check for selected */
      for(var i=0;i<iptvx.trackList.length;i++){
         if(trackid == app.streamconfig.selected){
            var at_id = $("#track"+trackid).attr("data-trackid");
            app.exec("set-audiotrack "+at_id);
         }
         trackid++;
      }

      /* go through all subtitles to check for selected */
      for(var i=0;i<iptvx.subtitleList.length;i++){
         if(trackid == app.streamconfig.selected){
            var st_id = $("#track"+trackid).attr("data-trackid");
            app.exec("set-subtitle "+st_id);
         }
         trackid++;
      }
   },

   /* handles key input */
   handleKey: function(keyCode){
      /* enter key selects track to switch */
      if(keyCode == 13){
         app.streamconfig.switchTrack();
      }

      /* up key */
      if(keyCode == 38){
         if(app.streamconfig.selected == 0){
            app.streamconfig.selected = app.streamconfig.count-1;
         }else{
            app.streamconfig.selected--;
         }
      }

      /* down key */
      if(keyCode == 40){
         if(app.streamconfig.selected == app.streamconfig.count-1){
            app.streamconfig.selected = 0;
         }else{
            app.streamconfig.selected++;
         } 
      }      
   },

   /* handles the resize of the document */
   resize: function(){
      /* in case the ui needs to resize */
   },

   /* toggles the streamconfig ui */
   toggle: function(forceOut = false){
      /* only allow toggle when stream is playing
         and we have got valid channel/ track data */
      if(iptvx.state == 3 && app.streamconfig.animating == false){
         if(app.streamconfig.visible == true || forceOut == true){
            /* fade out to the right */
            app.streamconfig.animating = true;
            $("#streamconfig").animate({
               opacity: 0,
               right: ($("#streamconfig").outerHeight()+20)*-1
            }, 500, "swing", function(){
                  app.streamconfig.animating = false;
               });

            /* set indicator to false */
            app.streamconfig.visible = false;
         }else{
            /* fade in from the right */
            app.streamconfig.animating = true;
            $("#streamconfig").animate({
               opacity: 0.8,
               right: 20
            }, 500, "swing", function(){
                  app.streamconfig.animating = false;
               });

            /* set indicator to true */
            app.streamconfig.visible = true;
         }
      }
   }   
}