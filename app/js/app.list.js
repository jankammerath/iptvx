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

app.list = {
	visible: false,
	selectedChannel: -1,

	/* initialises the programme list */
	init: function(){
		
	},

	/* handles key input when present */
	handleKey: function(keyCode){
		var epgData = app.epg.getEpgData();

		/* up key */
		if(keyCode == 38){
			if(app.list.selectedChannel == 0){
				app.list.selectedChannel = epgData.length-1;
			}else{
				app.list.selectedChannel--;
			}			
		}

		/* down key */
		if(keyCode == 40){
			if(app.list.selectedChannel == epgData.length-1){
				app.list.selectedChannel = 0;
			}else{
				app.list.selectedChannel++;
			}
		}
	},

	listen: function(){
		/* launch first update */
		app.list.update();

		/* update from epg every 200ms */
		setInterval(app.list.update,200);
	},

	update: function(){
		var now = Date.now() / 1000;
		var activeChannel = app.epg.getActiveChannelId();
		var epgData = app.epg.getEpgData();
		var listHtml = "";

		for(var c=0;c<epgData.length;c++){
			var chan = epgData[c];
			var showTitle = "";

			for(var p=0;p<chan.programmeList.length;p++){
				var programme = chan.programmeList[p];
				if(programme.start <= now && programme.stop >= now){
					showTitle = programme.title;
				}
			}

			var activeClass = "";
			if(app.list.selectedChannel == c){
				activeClass = " activechannel";
			}if(c == activeChannel && app.list.selectedChannel == -1){
				activeClass = " activechannel";
				app.list.selectedChannel = activeChannel;
			}

			listHtml += '<div class="channel'+activeClass+'">'
						+ '<div class="channellogo" '
						+ 'style="background-image:url(../data/logo/'+chan.logoFile+');">'
						+ '</div>'
						+ '<div class="channelname">'
						+ chan.name
						+ '</div><div class="channelshow">'
						+ showTitle
						+ '</div></div>';
		}
		$("#list").html(listHtml);
	},

	toggle: function(forceOut = false){
		if(app.list.visible == true || forceOut == true){
			/* fade out to bottom */
			$("#list").animate({
				opacity: 0,
				left: ($("#list").outerWidth()+100)*-1
			}, 500);

			/* set indicator to false */
			app.list.visible = false;
		}else{
			/* fade in from bottom */
			$("#list").animate({
				opacity: 0.8,
				left: 20
			}, 500);

			/* set indicator to true */
			app.list.visible = true;
		}		
	}
}