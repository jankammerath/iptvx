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

app.epg = {
   visible: false,
   ready: false,

	init: function(){
      /* keep checking until epg ready */
      app.epg.waitUntilReady();
   },

   /* forces application to switch to defined channel */
   switchChannel: function(channelId){
      if(typeof(iptvx)=="object"){
         /* check if defined channel within range */
         if(channelId >= 0 && channelId < iptvx.epg.length){
            /* call switch command */
            app.exec("switch-channel "+channelId);

            /* activate status indiactor */
            $("#status").html("&nbsp;");
            $("#status").show();
         }
      }
   },

   waitUntilReady: function(){
      /* check the status of the epg */
      app.epg.ready = app.epg.checkStatus();
      if(!app.epg.ready){
         /* check again until it is ready */
         setTimeout(app.epg.waitUntilReady,500);
      }
   },

   /* returns the complete epg data */
   getEpgData: function(){
      if(typeof(iptvx)=="object"){
         return iptvx.epg;
      } 
   },

   /* returns the active channel id */
   getActiveChannelId: function(){
      var result = 0;

      if(typeof(iptvx)=="object"){
         result = iptvx.channel;
      }

      return result;
   },

   /* gets the current channel and its current show */
   getCurrentChannelShow: function(){
      var result = new Object();
      var now = Date.now() / 1000;

      if(typeof(iptvx)=="object"){
         /* iterate through all channels */
         for(var c=0;c<iptvx.epg.length;c++){
            /* ensure c is the active channel number */
            if(c == iptvx.channel){
               result.channelId = c;
               result.channelName = iptvx.epg[c].name;
               result.programme = null;
               result.nextProgramme = new Array();

               /* determine the current programme */
               for(var p=0;p<iptvx.epg[c].programmeList.length;p++){
                  var programme = iptvx.epg[c].programmeList[p];
                  if(programme.start <= now && programme.stop >= now){
                     /* this is the current show */
                     result.programme = programme;
                  }else{
                     /* attach next three shows */
                     if(result.programme != null){
                        /* check if programme is after current one 
                           and we do not already have the next three shows */
                        if(result.programme.stop <= programme.start
                           && result.nextProgramme.length < 3){
                           /* push it into the next programme array */
                           result.nextProgramme.push(programme);
                        }
                     }
                  }
               }
            }  
         }
      }

      return result;
   },

   /* checks the status of the epg */
   checkStatus: function(){
      var result = false;

      if(typeof(iptvx)=="object"){
         if(iptvx.epgLoaded == 100){
            /* epg is finished */
            result = true;
            $("#status").hide();

            /* tell the control to listen */
            app.control.listen();

            /* tell the list to listen */
            app.list.listen();

            /* show the control */
            app.control.toggle();
         }else{
            if(iptvx.epgLoaded > 0){
               $("#status").html(iptvx.epgLoaded);
            }
            $("#status").show();
         }
      }

      return result;
   }
}