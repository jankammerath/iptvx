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

app.control = {
	visible: false,

	/* initialises the control ui */
	init: function(){
		/* start the EPG data interval */
	},

	/* starts the interval that listens to the epg */
	listen: function(){
		/* launch first update */
		app.control.update();

		/* update from epg every second */
		setInterval(app.control.update,500);
	},

	/* called by listen and updates from the epg */
	update: function(){
		if(typeof(iptvx)=="object"){
			if(iptvx.state == 3){
				/* is playing, hide load info */
				$("#status").hide();

				/* update video config */
				app.streamconfig.update();
			}if(iptvx.state == 0 
				|| iptvx.state == 1
				|| iptvx.state == 2){
				/* is waiting, opening or buffering
					so we show the load indicator */
				$("#status").removeClass("statusoffline");
				$("#status").addClass("statusloading");
				$("#status").html("&nbsp;");
				$("#status").show();
			}if(iptvx.state == 7){
				/* there is an error with the playback */
				$("#status").html("NO SIGNAL");
				$("#status").removeClass("statusloading");
				$("#status").addClass("statusoffline");
				$("#status").show();
			}		
		}

		/* get the EPG data */
		var data = app.epg.getCurrentChannelShow();
		$("#channelname").html(data.channelName);

		if(data.programme != null){
			/* show the time and progress */
			var now = Date.now();

			/* set the width of the progress bar */
			var startTs = data.programme.start*1000;
			var stopTs = (data.programme.stop*1000)-startTs;
			var progressVal = (((now-startTs) / stopTs)*100);

			$("#progressbarvalue").css("width",progressVal+"%");
			$("#progresstime").css("left",progressVal+"%");

			/* show time on the ui */
			var progStart = new Date(data.programme.start*1000);
			var progStop = new Date(data.programme.stop*1000);
			
			$("#showstart").html(progStart.toTimeString().substring(0,5));
			$("#showend").html(progStop.toTimeString().substring(0,5));
			$("#progresstime").html(new Date().toTimeString().substring(0,5));

			/* prevent overlapping with start and stop display */
			if(progressVal >= 11 && progressVal <= 89){
				$("#progresstime").show();
			}else{
				/* hide to prevent overlap */
				$("#progresstime").hide();
			}

			/* set the window title */
			document.title = data.channelName + ": "
							+ data.programme.title;

			/* set the programme basic information */
			$("#showtitle").html(data.programme.title);
			var showInfo = data.programme.category;
			if(data.programme.productionDate!=""){
				showInfo += " ("+data.programme.productionDate+")";
			}if(data.programme.description != ""){
				showInfo += " - " + data.programme.description;
			} 
			$("#showinfo").html(showInfo);
			
			/* render upcoming shows */
			var nextProgrammeHtml = "";
			for(var n=0;n<data.nextProgramme.length;n++){
				var nextProgTitle = data.nextProgramme[n].title;
				var nextProgStart = new Date(data.nextProgramme[n].start*1000);
				var startTimeText = nextProgStart.toTimeString().substring(0,5);
				nextProgrammeHtml += '<div class="shownextitem">'
							+ '<div class="shownextitemtime">' + startTimeText + '</div>'
							+ '<div class="shownextitemtitle">' + nextProgTitle + '</div>'
							+ '</div>'
			}
			$("#shownext").html(nextProgrammeHtml);
		}
	},

	/* toggles the control ui */
	toggle: function(forceOut = false){
		if(app.control.visible == true
			|| forceOut == true){
			/* fade out to bottom */
			$("#control").animate({
				opacity: 0,
				bottom: ($("#control").outerHeight()+100)*-1
			}, 500);

			/* set indicator to false */
			app.control.visible = false;
		}else{
			/* fade in from bottom */
			$("#control").animate({
				opacity: 0.8,
				bottom: 30
			}, 500);

			/* set indicator to true */
			app.control.visible = true;
		}
	}
}