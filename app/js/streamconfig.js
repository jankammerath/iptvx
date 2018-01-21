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

var streamconfig = {
   selectedTrack: 0,
   trackCount: 0,
   toggleKey: 9, /* TAB */

   init: function(){
      /* start the update interval to monitor 
         the stream and its available tracks */
      setInterval(streamconfig.update,3000);
   },

   /*
      Updates the stream configuration
   */
   update: function(){
      var html = "";
      var trackNum = 0;

      /* render audio tracks first */
      var audioList = player.getAudioList();
      for(var a=0; a<audioList.length; a++){
         html += "<div class=\"track\" "
               + "data-tracktype=\"audio\" ";

         if(audioList[a].active){
            html += "data-active=\"true\" "
         }else{
            html += "data-active=\"false\" "
         }

         if(streamconfig.selectedTrack == trackNum){
            html += "data-selected=\"true\" "
         }else{
            html += "data-selected=\"false\" "
         }

         html += "data-trackid=\"" + audioList[a].id + "\">"
               + audioList[a].name
               + "</div>";

         trackNum++;
      }

      /* next up render subtitle tracks */
      var subtitleList = player.getSubtitleList();
      for(var s=0; s<subtitleList.length; s++){
         html += "<div class=\"track\" "
               + "data-tracktype=\"subtitle\" "
         
         if(subtitleList[s].active){
            html += "data-active=\"true\" "
         }else{
            html += "data-active=\"false\" "
         }

         if(streamconfig.selectedTrack == trackNum){
            html += "data-selected=\"true\" "
         }else{
            html += "data-selected=\"false\" "
         }

         html += "data-trackid=\"" + subtitleList[s].id + "\">"
               + subtitleList[s].name
               + "</div>";

         trackNum++;
      }

      /* update track count */
      streamconfig.trackCount = trackNum;

      /* flush html to component */
      $("#streamconfig").html(html);
   },

   handleKey: function(keyCode){
      if(keyCode == 38){
         /* KEY UP */
         if(streamconfig.selectedTrack == 0){
            streamconfig.selectedTrack = (streamconfig.trackCount-1);
         }else{
            streamconfig.selectedTrack--;
         }

         streamconfig.update();
      }if(keyCode == 40){
         /* KEY DOWN */
         if(streamconfig.selectedTrack == (streamconfig.trackCount-1)){
            streamconfig.selectedTrack = 0;
         }else{
            streamconfig.selectedTrack++;
         }

         streamconfig.update();
      }if(keyCode == 13){
         /* RETURN KEY */
         if($(".track[data-selected='true']").length){
            var element = $(".track[data-selected='true']");
            var trackId = parseInt($(element).attr("data-trackid"));
            var type = $(element).attr("data-tracktype");

            if(type == "audio"){
               player.setAudioTrack(trackId);
            }if(type == "subtitle"){
               player.setSubtitleTrack(trackId);
            }

            setTimeout(streamconfig.update,1000);
         }
      }
   }
}