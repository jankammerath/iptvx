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

var app = {
	/* list ui object */
	list: {},

	/* epg ui object */
	epg: {},

	/* control ui object */
	control: {},

	/* initialises the app */
	init: function(){
		/* attach global error handler */
		app.attachErrorHandler();

		/* attach resize to ready and window resize */
		$(document).ready(function(){app.resize();});
		$(window).resize(function(){app.resize();});

		/* attach key downs */
		$(window).keydown(function(event){
			var keyCode = event.which;

			/* only handle keys when epg is ready */
			if(app.epg.ready){
				/* control ui toggle with spacebar (32) */
				if(keyCode == 32){
					app.epg.toggle(true);
					app.list.toggle(true);
					app.control.toggle();
				}

				/* list ui toggle with shift (16) */
				if(keyCode == 16){
					app.epg.toggle(true);
					app.control.toggle(true);
					app.list.toggle();
				}

				/* epg ui toggle with alt (18) */
				if(keyCode == 18){
					app.list.toggle(true);
					app.control.toggle(true);
					app.epg.toggle();
				}

				/* only handle keys when no ui is present to handle */
				if(app.list.visible == false && app.epg.visible == false){
					/* switch channel with up (38) and down (40) */
					if(keyCode == 38){app.exec("channel-prev");}
					if(keyCode == 40){app.exec("channel-next");}
				}

				/* allow list to handle keys */
				if(app.list.visible){
					app.list.handleKey(keyCode);
				}
			}
			
			/* output the debug message */
			app.showDebug(keyCode);
		});

		/* initialise ui objects */
		app.epg.init();
		app.control.init();
		app.list.init();
	},

	attachErrorHandler: function(){
		window.onerror = function (msg, url, lineNo, columnNo, error) {
		    var string = msg.toLowerCase();
		    var substring = "script error";
		    if (string.indexOf(substring) > -1){
		        $("#debuginfo").html('Script Error: See Browser Console for Detail');
		    } else {
		        var message = [
		            'Message: ' + msg,
		            'URL: ' + url,
		            'Line: ' + lineNo,
		            'Column: ' + columnNo
		        ].join(' - ');

		        $("#debuginfo").html(message);
		    }

		    return false;
		};
	},

	/* shows the debug message */
	showDebug: function(keyVal){
		var debugText = "Key: "+keyVal;
		if(typeof(iptvx)=="object"){
			debugText += "; API OK; EPG "
						+iptvx.epgLoaded+"%";
		}else{
			debugText += " - NO API!";
		}
		$("#debuginfo").html(debugText);
	},

	/* executes core message */
	exec: function(command){
		if(typeof(iptvx)=="object"){
			if(typeof(iptvx.exec)=="function"){
				iptvx.exec(command);
			}
		}
	},

	/* handles resize of the window */
	resize: function(){
		var controlLeft = ($(window).innerWidth()/2)-($("#control").outerWidth()/2);
		$("#control").css("left",controlLeft+"px");	

		var listMaxHeight = $(window).innerHeight()-40;
		$("#list").css("max-height",listMaxHeight+"px");

		var statusLeft = $(window).innerWidth()/2-$("#status").outerWidth()/2;
		var statusTop = $(window).innerHeight()/2-$("#status").outerHeight()/2;
		$("#status").css("left",statusLeft+"px");
		$("#status").css("top",statusTop+"px");

		var epgWidth = $(window).innerWidth()-60;
		if(epgWidth>800){epgWidth=800;}
		var epgHeight = $(window).innerHeight()-100;
		$("#epg").css("width",epgWidth+"px");
		$("#epg").css("height",epgHeight+"px");
		var epgLeft = $(window).innerWidth()/2-$("#epg").outerWidth()/2;
		$("#epg").css("left",epgLeft+"px");
		$("#epgcalbody").css("height",$("#epg").innerHeight()-75);
	}
}

/* initialise the app when document is ready */
$(document).ready(function(){app.init();});