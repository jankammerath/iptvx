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

app.list = {
	visible: false,
	selectedChannel: -1,

	/* initialises the programme list */
	init: function(){
		app.list.adjustSize();
	},

	/* adjusts size of the list */
	adjustSize: function(){
		var cur_height = $("#list").outerHeight();
		var max_height = $(window).innerHeight()-100;
		if(cur_height > max_height){
			$("#list").css("height",max_height+"px");

			var clist = $("#list .channel");
			for(var c=0;c<clist.length;c++){
				if($(clist[c]).hasClass("activechannel")){
					var elm_h = $(clist[c]).outerHeight()+10;
					$("#list").scrollTop(c*elm_h);
				}
			}
		}
	},

	/* handles key input when present */
	handleKey: function(keyCode){
		var epgData = app.epg.getEpgData();

		/* enter key */
		if(keyCode == 13 && app.list.selectedChannel >= 0){
			app.epg.switchChannel(app.list.selectedChannel);
		}

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

		/* render selected channel */
		$(".channel").removeClass("activechannel");
		$("#listchannel"+app.list.selectedChannel).addClass("activechannel");

		/* adjust size which also scrolls 
			to the currently active channel */
		app.list.adjustSize();
	},

	listen: function(){
		/* launch first update */
		app.list.update();

		/* update from epg every minute */
		setInterval(app.list.update,60000);
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

			listHtml += '<div id="listchannel'+c+'" class="channel'+activeClass+'">'
						+ '<div class="channellogo" '
						+ 'style="background-image:url(/logo/'+chan.logoFile+');">'
						+ '</div>'
						+ '<div class="channelname">'
						+ chan.name
						+ '</div><div class="channelshow">'
						+ showTitle
						+ '</div></div>';
		}
		$("#list").html(listHtml);

		/* adjust the size when needed */
		app.list.adjustSize();
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