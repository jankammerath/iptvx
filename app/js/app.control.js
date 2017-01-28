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
		setInterval(app.control.update,1000);
	},

	/* called by listen and updates from the epg */
	update: function(){
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

			/* set the programme basic information */
			$("#showtitle").html(data.programme.title);
			var showType = data.programme.category;
			if(data.programme.productionDate!=""){
				showType += " ("+data.programme.productionDate+")";
			}
			$("#showtype").html(showType);
			$("#showtext").html(data.programme.description);
		}
	},

	formatTime: function(dateObj){
		var result = "";



		return result;
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