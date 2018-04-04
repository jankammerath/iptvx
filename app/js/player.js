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
   /* volume fadeout timeout */
   volumeTimeout: null,

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

      /* adjust the volume indicator */
      $("#volume").css("top",(($(window).height()/2)
                     -$("#volume").outerHeight()/2)+"px");
      $("#volume").css("left",(($(window).width()/2)
                     -$("#volume").outerWidth()/2)+"px");
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
      Returns the FourCC id of the codec
   */
   getCodecId: function(){
      result = "UNKNOWN";

      if(player.isReady()){
         if("codec" in iptvx.videoinfo){
            if(iptvx.videoinfo["codec"] != ""){
               result = iptvx.videoinfo["codec"];
            }
         }
      }      

      return result;
   },

   /* 
      Returns the bit rate of the current playback in Kbps as int
   */
   getBitrate: function(){
      result = 0;

      if(player.isReady()){
         if("bitrate" in iptvx.videoinfo){
            if(iptvx.videoinfo["bitrate"] > 0){
               result = iptvx.videoinfo["bitrate"];
            }
         }
      }

      return result;
   },

   /*
      Returns the size of the video as text (e.g. 1280x720)
   */
   getVideoSize: function(){
      result = "0x0";

      if(player.isReady()){
         if("width" in iptvx.videoinfo
            && "height" in iptvx.videoinfo){
            result = iptvx.videoinfo["width"]
                  + "x" + iptvx.videoinfo["height"];
         }
      }

      return result;
   },

   getSubtitleList: function(){
      if(player.isReady()){
         return iptvx.subtitleList;
      }else{
         return [];
      }
   },

   getAudioList: function(){
      if(player.isReady()){
         return iptvx.trackList;
      }else{
         return [];
      }
   },

   /*
      Sets the audio track by its id
   */
   setAudioTrack: function(trackId){
      player.execute("set-audiotrack " + trackId);
   },

   /*
      Sets the subtitle track by its id
   */
   setSubtitleTrack: function(trackId){
      player.execute("set-subtitle " + trackId);
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
      Adjusts the volume level of the current playback
      @param      adjustmentLevel   percentage value of adjustment between -100 and 100
   */
   adjustVolume: function(adjustmentLevel){
      /* calculate new volume level */
      var newVolumeValue = player.getVolume()+adjustmentLevel;
      
      /* make sure volume does not exceed limits */
      if(newVolumeValue > 150){newVolumeValue = 150;}
      if(newVolumeValue < 0){newVolumeValue = 0;}

      /* request player to switch to new volume */
      player.execute("set-volume "+newVolumeValue);

      /* change appearance based on volume level */
      $("#volume").removeClass();
      if(newVolumeValue == 0){
         /* this is mute */
         $("#volume").addClass("volumemute");
      }else{
         /* this is normal */
         $("#volume").addClass("volumenormal");
      }

      /* update the volume information */
      clearTimeout(player.volumeTimeout);
      $("#volume").fadeIn();
      $("#volumelevel").css("width",((newVolumeValue/150)*100)+"%");
      player.volumeTimeout = setTimeout(function(){
         $("#volume").fadeOut();
      },2000);
   },

   /*
      Returns the current volume value in percent
   */
   getVolume: function(){
      if(typeof(iptvx) == 'object'){
         return iptvx.volume;
      }

      return 0;
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
      var result = 0;

      if(player.isReady()){
         result = iptvx.channel;
      }

      return result;
   }
}