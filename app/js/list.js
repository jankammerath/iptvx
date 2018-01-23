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

var list = {
   epg: [],
   defaultStarted: false,
   toggleKey: 16, /* SHIFT */

   /*
      Returns the name of the current programme for a channel
      @param      programmeList       list with all programmes
   */
   getCurrentProgrammeName: function(programmeList){
      var result = "No epg data";

      var now = Date.now() / 1000 | 0;
      for(var c=0;c<programmeList.length;c++){
         var prog = programmeList[c];
         if(prog.start <= now && prog.stop >= now){
            result = prog.title;
         }
      }

      return result;
   },

   /*
      Handles key down events for this component
      @param      keyCode     code of the key that raised the event
   */
   handleKey: function(keyCode){
      switch(keyCode){
         case 13: /* KEY RETURN */
            /* return key switches to the selected channel */
            if($(".listchannelselected").length){
               var activeChannel = parseInt($(".listchannelselected")
                                          .attr("data-channelid"));
               player.selectChannelId(activeChannel);
            }

            break;
         case 38: /* KEY UP */
            list.selectChannel(0);
            break;
         case 40: /* KEY DOWN */
            list.selectChannel(1);
            break;
      }
   },

   /*
      Select next or previous channel in the list
      @param      direction      0 = previous channel, 1 = next channel
   */
   selectChannel: function(direction){
      /* get the number of total channels */
      var channelCount = $(".listchannel").length;
      var activeChannel = 0;
      if($(".listchannelselected").length){
         activeChannel = parseInt($(".listchannelselected").attr("data-channelid"));
      }

      if(direction == 1){
         /* select next channel in list */
         if(activeChannel == (channelCount-1)){
            /* last channel hit */
            activeChannel = 0;
         }else{
            activeChannel++;
         }
      }else{
         /* select previous channel in list */
         if(activeChannel == 0){
            /* first channel hit */
            activeChannel = channelCount-1;
         }else{
            activeChannel--;
         }
      }

      /* remove active class from all 
         and assign to newly selected */
      $(".listchannel").removeClass("listchannelselected");
      $(".listchannel[data-channelid='"+activeChannel+"']").addClass("listchannelselected");

      /* scroll list to selected element */
      var elementTop = $(".listchannel[data-channelid='"+activeChannel+"']").position().top;
      $("#list").scrollTop(elementTop);
   },

   /*
      Updates current programme for each channel
   */
   update: function(){
      if(list.epg.length > 0){
         list.updateFromData(list.epg);
      }else{
         $.getJSON("/epg.json",function(data){
            list.epg = data;
            list.updateFromData(data);
         });
      }
   },

   updateFromData: function(data){
      for(var c=0; c<data.length; c++){
         var chan = data[c];
         var prog = list.getCurrentProgrammeName(chan.programmeList);
         $(".listchannel[data-channelid='" + c + "'] "
            + ".listchannelprogramme").text(prog);
      }
   },

   load: function(){
      $.getJSON("/epg.json",function(data){
         var html = "";

         /* get the currently active channel */
         var activeChannelId = player.getCurrentChannelId();
         var defaultChannelId = -1;

         /* create html for the channel list */
         for(var c=0; c<data.length; c++){
            var chan = data[c];
            var currentProgramme = list.getCurrentProgrammeName(chan.programmeList);

            if(chan.default){
               defaultChannelId = c;
            }

            var activeClass = "";
            if(c == activeChannelId){
               activeClass = "listchannelselected";
            }

            html += "<div class=\"listchannel " + activeClass 
                  + "\" data-channelid=\"" + c + "\">"
                  + "<div class=\"listchannellogo\" "
                  + "style=\"background-image:url('/logo/" + chan.logoFile + "');\"></div>"
                  + "<div class=\"listchannelname\">" + chan.name + "</div>"
                  + "<div class=\"listchannelprogramme\">" + currentProgramme + "</div>"
                  + "</div>";
         }

         /* attach new channel html */
         $("#list").html(html);

         /* call resize for properly fit */
         list.resize();

         /* attach click handler */
         $(".listchannel").mouseup(function(){
            $(".listchannel").removeClass("listchannelselected");
            $(this).addClass("listchannelselected");

            var activeChannel = parseInt($(this).attr("data-channelid"));
            player.selectChannelId(activeChannel);
         });

         /* start default channel if not done initially */
         if(list.defaultStarted == false && defaultChannelId >= 0){
            player.selectChannelId(defaultChannelId);
            list.defaultStarted = true;
         }
      });
   },

   resize: function(){
      var maxHeight = $(window).innerHeight()-20;
      $("#list").css("max-height",maxHeight+"px");
   },

   init: function(){
      list.load();

      /* update regularly */
      setInterval(list.update,30000);
   }
}