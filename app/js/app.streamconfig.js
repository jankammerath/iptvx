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

   update: function(){
      if(typeof(iptvx)=="object"){
         var html = "";

         /* get all audio tracks available */
         for(var i=0;i<iptvx.trackList.length;i++){
            var track = iptvx.trackList[i];

            var classList = "configitem audiotrackitem";
            if(track.active == true){
               classList += " configitemactive";
            }
            html += "<div class=\""+classList+"\" "
                  + "data-trackid=\"" + track.id + "\">"
                  + track.name + "</div>";
         }

         /* get all subtitle tracks available */
         for(var i=0;i<iptvx.subtitleList.length;i++){
            var subtitle = iptvx.subtitleList[i];

            var classList = "configitem subtitleitem";
            if(subtitle.active == true){
               classList += " configitemactive";
            }
            html += "<div class=\""+classList+"\" "
                  + "data-trackid=\"" + subtitle.id + "\">"
                  + subtitle.name + "</div>";
         }

         /* smash html into the container */
         $("#streamconfig").html(html);
      }
   },

   /* handles the resize of the document */
   resize: function(){
      var configTop = ($(window).innerHeight()/2)-($("#streamconfig").outerHeight()/2);
      if(configTop<0){configTop=10;}
      $("#streamconfig").css("top",configTop+"px");     
   },

   /* toggles the streamconfig ui */
   toggle: function(forceOut = false){
      /* only allow toggle when stream is playing
         and we have got valid channel/ track data */
      if(iptvx.state == 3){
         if(app.streamconfig.visible == true || forceOut == true){
            /* fade out to the right */
            $("#streamconfig").animate({
               opacity: 0,
               right: ($("#streamconfig").outerHeight()+100)*-1
            }, 500);

            /* set indicator to false */
            app.streamconfig.visible = false;
         }else{
            /* fade in from the right */
            $("#streamconfig").animate({
               opacity: 0.8,
               right: 20
            }, 500);

            /* set indicator to true */
            app.streamconfig.visible = true;
         }
      }
   }   
}