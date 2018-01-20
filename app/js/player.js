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

var player = {
   /*
      Initialise the player and check its status
   */
   init: function(){
      /* start the instance monitoring interval */
      setInterval(player.monitorStatus,1000);

      /* initially set sizes and positions */
      player.resize();
   },

   /*
      Handle resize of the window
   */
   resize: function(){
      /* adjust the status indicator at the center */
      $("#status").css("top",(($(window).height()/2)
                     -$("#status").outerHeight()/2)+"px");
      $("#status").css("left",(($(window).width()/2)
                     -$("#status").outerWidth()/2)+"px");
   },

   /* 
      Monitors the status of the player instance
   */
   monitorStatus: function(){
      if(player.isReady()){
         if(iptvx.state == 3){
            /* is playing, so hide status */
            $("#status").hide();

            /* update video config */
            // app.streamconfig.update();
         }if(iptvx.state == 0 || iptvx.state == 1 || iptvx.state == 2){
            /* is waiting, opening or buffering
               so we show the load indicator */
            $("#status").removeClass("offline");
            $("#status").addClass("loading");
            $("#status").html("&nbsp;");
            $("#status").show();
         }if(iptvx.state == 7){
            /* there is an error with the playback */
            $("#status").html("NO SIGNAL");
            $("#status").removeClass("loading");
            $("#status").addClass("offline");
            $("#status").show();
         }       
      }
   },

   /*
      Checks if the player instance is ready
   */
   isReady: function(){
      var result = false;

      if(typeof(iptvx) == 'object'){
         result = true;
      }

      return result;
   },

   /*
      Switches to the channel provided
      @param      channelId      id of the channel to switch to
   */
   selectChannelId: function(channelId){
      player.execute("switch-channel "+channelId);
   },

   /*
      Executes a player command
   */
   execute: function(command){
      if(player.isReady()){
         iptvx.exec(command);
      }
   },

   /*
      Returns the channel id of the current channel
   */
   getCurrentChannelId: function(){
      return 0;
   }
}