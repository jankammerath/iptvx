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

var control = {
   epg: [],
   toggleKey: 18, /* ALT */

   init: function(){
      /* update initially */
      control.update();

      /* update regularly */
      setInterval(control.update,3000);

      /* resize initially */
      control.resize();
   },

   /*
      Updates the control with information
   */
   update: function(){
      if(control.epg.length == 0){
         $.getJSON("/epg.json",function(data){
            control.epg = data;
            control.updateFromData(data);
         });
      }else{
         control.updateFromData(control.epg);
      }

   },

   updateFromData: function(data){
      var channelId = player.getCurrentChannelId();
      if(data[channelId] !== void 0){
         var chan = data[channelId];

         /* set the channel name */
         $("#tvcontrol .channelname").text(chan.name);

         /* determine the current and the next programme */
         var currentProg = -1;
         var now = app.now();
         for(var p=0; p<chan.programmeList.length; p++){
            var prog = chan.programmeList[p];

            if(currentProg >= 0 && currentProg < 3){
               $("#nextprogrammetitle"+currentProg).text(prog.title);
               $("#nextprogrammetime"+currentProg).text
                           (app.formatTime(prog.start));
               currentProg++;
            }

            if(prog.start <= now && prog.stop >= now){
               /* this is the current programme */
               control.updateCurrentProgramme(prog);
               currentProg = 0;
            }
         }
      }
   },

   /*
      Updates the current programme
   */
   updateCurrentProgramme: function(prog){
      /* update the progress bar first */
      $("#tvcontrol .programmestart").text(app.formatTime(prog.start));
      $("#tvcontrol .programmestop").text(app.formatTime(prog.stop));
      $("#tvcontrol .progressbartext").text(app.formatTime(app.now()));

      /* calculate expired percentage */
      var runtime = prog.stop - prog.start;
      var expired = app.now() - prog.start;
      var percentage = Math.round((expired / runtime) * 100);
      $("#tvcontrol .progressbarvalue").css("width",percentage+"%");

      /* update programme title and text */
      $("#tvcontrol .programmetitle").text(prog.title);
      $("#tvcontrol .programmetext").text(prog.description);

      /* update video information */
      $("#tvcontrol .channelformat").text(player.getCodecId().toUpperCase());
      $("#tvcontrol .channelresolution").text(player.getVideoSize());
      $("#tvcontrol .channeldatarate").text(player.getBitrate()+" Kbps");

      /* attach text to progress bar */
      control.adjustProgressText();
   },

   /* 
      Attach the progress text properly to the progress bar value 
   */
   adjustProgressText: function(){
      if($("#control").is(":visible")){
         var textLeft = $("#tvcontrol .progressbarvalue").outerWidth()
                        + $("#tvcontrol .progressbarvalue").position().left
                        - ($("#tvcontrol .progressbartext").outerWidth()/2);
         $("#tvcontrol .progressbartext").css("left",textLeft+"px");
      }
   },

   resize: function(){
      $("#control").css("left",(($(window).width()/2)
                     -$("#control").outerWidth()/2)+"px");

      control.adjustProgressText();
   }
}