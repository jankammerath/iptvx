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
   lastRenderSize: 0,

	init: function(){
      /* keep checking until epg ready */
      app.epg.waitUntilReady();
   },

   /* handles scrolling epg */
   handleKey: function(keyCode){
      var vertSteps = 120; 

      if(keyCode == 39){
         /* key is RIGHT */
         var hlist = $(".epgcalheaditem");
         var clist = $(".epgcalchannel");

         if($(".epgcalchannel:visible").length>4){
            var moved = false;
            for(var c=0;c<clist.length;c++){
               if($(clist[c]).is(":visible") == true
                  && moved == false){
                  $(clist[c]).hide();
                  $(hlist[c]).hide();
                  moved = true;
               }
            }
         }
      }if(keyCode == 37){
         /* key is LEFT */
         var hlist = $(".epgcalheaditem");
         var clist = $(".epgcalchannel");

         if($(".epgcalchannel:hidden").length>0){
            var moved = false;
            for(var c=clist.length-1;c>=0;c--){
               if($(clist[c]).is(":visible") == false
                  && moved == false){
                  $(clist[c]).show();
                  $(hlist[c]).show();
                  moved = true;
               }
            } 
         }        
      }if(keyCode == 38){
         /* key is UP */
         var calTop = $("#epgcalbody").position().top;
         var minTop = 76;
         if(calTop < minTop && (calTop+vertSteps) < minTop){
            $("#epgcalbody").css("top",(calTop+vertSteps)+"px");
         }else{
            $("#epgcalbody").css("top",minTop+"px");
         }
      }if(keyCode == 40){
         /* key is DOWN */
         var calTop = $("#epgcalbody").position().top;
         calTop = (calTop-vertSteps);
         var calHeight = ($("#epgcalbody").height()-$("#epg").height()+2)*-1;
         
         if(calTop < calHeight){
            $("#epgcalbody").css("top",calHeight+"px");
         }else{
            $("#epgcalbody").css("top",calTop+"px");
         }
      }
   },

   /* renders epg calendar */
   render: function(){
      if(typeof(iptvx)=="object"){         
         var hour = new Date().getHours();

         /* set up the header */
         $("#epgdate").html(new Date().toDateString());

         var calDateHtml = "<div id=\"epgcaldate\">";
         var days = 0;
         var hoursCount = app.epg.getDisplayHoursCount();
         for(var h=hour;h<(hour+hoursCount);h++){
            var hourVal = h-(24*days);
            if(hourVal > 23){
               hourVal = 0;
               days++;
            }

            var hourText = String(hourVal);
            if(hourText.length == 1){
               hourText = "0"+String(hourVal);
            }
            hourText = hourText+":00";

            calDateHtml += "<div class=\"epgcalhour\">"
                        + hourText + "</div>";
         }
         calDateHtml += "</div>";

         /* go through channels */
         var channelHeadHtml = "";
         for(var c=0;c<iptvx.epg.length;c++){
            var chan = iptvx.epg[c];

            /* set up channel heads */
            channelHeadHtml += "<div class=\"epgcalheaditem\">"
                              + "<div class=\"epgcalheaditemlogo\" "
                              + "style=\"background-image:url('../data/logo/"
                              + chan.logoFile + "');\"></div>"
                              + "<div class=\"epgcalheaditemtext\">"
                              + chan.name + "</div></div>";
         }
         
         /* we need the timestamp at the beginning of 
            the hour which means we substract the total 
            seconds passed since the beginning */
         var now_date = new Date();
         var now_ts = Date.now()/1000;
         now_ts = now_ts-(now_date.getMinutes()*60)-now_date.getSeconds();

         var channelHtml = "";
         for(var c=0;c<iptvx.epg.length;c++){
            var chan = iptvx.epg[c];

            channelHtml += "<div class=\"epgcalchannel\">"
                        + "<div class=\"epgcalchannelprogramme\">";
            for(var p=0;p<chan.programmeList.length;p++){
               var prog = chan.programmeList[p];

               if(prog.stop > now_ts){
                  var height = (((prog.stop-prog.start)/60)*4)-2;
                  var top = ((prog.start-now_ts)/60)*4;

                  /* titles of items at the beginning would not be 
                     shown if the show is already on. We change
                     the height so that they begin on the top. */
                  if(top < 0){
                     height = height+top;
                     top = 0;
                  }

                  channelHtml += "<div class=\"epgcalprogramme\" "
                                 +" style=\"height:"+height+"px;top:"+top+"px\">";

                  if(height >= 25){
                     var descText = "";

                     if(typeof(prog.description)=="string"){
                        descText = prog.description.replace(/\n/g, "<br />");
                     }

                     channelHtml += "<div class=\"epgcalprogrammetitle\">" 
                                 + prog.title + "</div>"
                                 + "<div class=\"epgcalprogrammedescription\">" 
                                 + descText + "</div>";
                  }

                  channelHtml += "</div>";
               }
            }

            channelHtml += "</div></div>";
         }

         var epgHtml = "<div id=\"epgcalhead\">" + channelHeadHtml + "</div>"
                     + "<div id=\"epgcalbody\">"
                     + calDateHtml + channelHtml 
                     + "</div>";

         $("#epgcal").html(epgHtml);
      }
   },

   /* gets the number of hours ahead the epg should display */
   getDisplayHoursCount: function(){
      var result = 48;
      var now = Date.now;
      var max = app.epg.getMaxDate();
      var hours = (max - now)/3600;
      if(hours > result){
         result = hours;
      }

      return result;
   },

   /* get the maximum date stored in the epg */
   getMaxDate: function(){
      var result = Date.now+172800;

      if(typeof(iptvx)=="object"){
         for(var c=0;c<iptvx.epg.length;c++){
            for(var p=0;p<iptvx.epg[c].programmeList.length;p++){
               if(iptvx.epg[c].programmeList[p].stop > result){
                  result = iptvx.epg[c].programmeList[p].stop;
               }
            }
         }
      }

      return result;
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

            /* set the size of the epg */
            app.epg.lastRenderSize = app.epg.getEpgSize();

            /* render the epg */
            app.epg.render();

            /* watch the epg size every second
               to check for potential updates */
            setInterval(function(){
               if(app.epg.lastRenderSize != app.epg.getEpgSize()){
                  /* update with new size */
                  app.epg.lastRenderSize = app.epg.getEpgSize();

                  /* start renderer */
                  app.epg.render();
               }
            },1000);
         }else{
            if(iptvx.epgLoaded > 0){
               $("#status").html(iptvx.epgLoaded);
            }
            $("#status").show();
         }
      }

      return result;
   },

   getEpgSize: function(){
      var result = 0;

      if(typeof(iptvx)=="object"){
         result = JSON.stringify(iptvx.epg).length;
      }

      return result;
   },

   /* toggles the epg ui */
   toggle: function(forceOut = false){
      if(app.epg.visible == true
         || forceOut == true){
         /* fade out to bottom */
         $("#epg").animate({
            opacity: 0,
            top: $("#epg").outerHeight()*-1
         }, 500);

         /* set indicator to false */
         app.epg.visible = false;
      }else{
         /* fade in from bottom */
         $("#epg").animate({
            opacity: 0.95,
            top: 0
         }, 500);

         /* set indicator to true */
         app.epg.visible = true;
      }
   }
}