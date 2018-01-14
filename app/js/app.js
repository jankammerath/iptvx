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

var app = {
	/* list ui object */
	list: {},

	/* epg ui object */
	epg: {},

	/* control ui object */
	control: {},

	/* handles volume timeout */
	volumeTimeout: {},

	/* initialises the app */
	init: function(){
		/* attach global error handler */
		app.attachErrorHandler();

		/* attach resize to ready and window resize */
		$(document).ready(function(){app.resize();});
		$(window).resize(function(){app.resize();});

		/* initialise the mouse handler */
		app.mouse.init();

		/* attach key downs */
		$(window).keydown(function(event){
			var keyCode = event.which;

			/* hash key toggles debug */
			if(keyCode == 51){
				$("#debuginfo").toggle();
			}

			/* 	33 = increase volume (pgdown),
				34 = decrease volume (pgup) */
			if(typeof(iptvx)=="object"){ 
				if(keyCode == 33){
					app.adjustVolume(iptvx.volume+10);
				}if(keyCode == 34){
					app.adjustVolume(iptvx.volume-10);
				}
			}

			/* only handle keys when epg is ready */
			if(app.epg.ready){
				/* disable all ui with Escape (27) */
				if(keyCode == 27){
					app.epg.toggle(true);
					app.list.toggle(true);
					app.control.toggle(true);
					app.find.toggle(true);
					app.streamconfig.toggle(true);
					$("#volume").fadeOut();
				}

				/* control ui toggle with alt (18) */
				if(keyCode == 18){
					app.epg.toggle(true);
					app.list.toggle(true);
					app.find.toggle(true);
					app.streamconfig.toggle(true);
					app.control.toggle();
				}

				/* list ui toggle with shift (16) */
				if(keyCode == 16){
					app.epg.toggle(true);
					app.control.toggle(true);
					app.find.toggle(true);
					app.streamconfig.toggle(true);
					app.list.toggle();
				}

				/* epg ui toggle with tab (9) */
				if(keyCode == 9){
					app.list.toggle(true);
					app.control.toggle(true);
					app.find.toggle(true);
					app.streamconfig.toggle(true);
					app.epg.toggle();
				}

				/* toggle config with F2 (113) */
				if(keyCode == 113){
					app.list.toggle(true);
					app.control.toggle(true);
					app.epg.toggle(true);
					app.find.toggle(true);
					app.streamconfig.toggle();
				}

				/* search ui toggle with ctl (17) */
				if(keyCode == 17){
					app.list.toggle(true);
					app.control.toggle(true);
					app.epg.toggle(true);
					app.streamconfig.toggle(true);
					app.find.toggle();
				}


				/* only handle keys when no ui is present to handle */
				if(app.list.visible == false 
					&& app.epg.visible == false
					&& app.find.visible == false
					&& app.streamconfig.visible == false){
					/* switch channel with up (38) and down (40) */
					if(keyCode == 38){app.epg.zapChannel(false);}
					if(keyCode == 40){app.epg.zapChannel(true);}
				}

				/* allow list to handle keys */
				if(app.list.visible){
					app.list.handleKey(keyCode);
				}

				/* allow epg to handle keys */
				if(app.epg.visible){
					app.epg.handleKey(keyCode);
				}

				/* allow stream config to handle keys */
				if(app.streamconfig.visible){
					app.streamconfig.handleKey(keyCode);
				}

				/* allow find to handle keys */
				if(app.find.visible){
					app.find.handleKey(keyCode);
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

	/* adjusts playback volume and shows ui for it */
	adjustVolume: function(volumeValue){
		/* clear any current timeout for fade out */
		clearTimeout(app.volumeTimeout);

		/* ensure volume not going below 0 */
		if(volumeValue < 0){
			volumeValue = 0;
		}

		/* call volume update */
		app.exec("set-volume "+volumeValue);

		/* update volume ui */
		if(volumeValue == 0){
			$("#volume").removeClass("volumenormal");
			$("#volume").addClass("volumemute");
		}else{
			$("#volume").removeClass("volumemute");
			$("#volume").addClass("volumenormal");
		}
		var volumePercent = 60*(volumeValue/150);
		$("#volumevalue").css("width",volumePercent+"px");
		$("#volume").fadeIn();

		/* set fadeout */
		app.volumeTimeout = setTimeout(function(){
			$("#volume").fadeOut();
		},2000);
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

	/* removes html chars from data */
	stripHtml: function(data){
    	var tmp = document.createElement("DIV");
   		tmp.innerHTML = data;
   		return tmp.textContent || tmp.innerText || "";
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

		var volumeLeft = ($(window).innerWidth()/2)-($("#volume").outerWidth()/2);
		$("#volume").css("left",volumeLeft+"px");

		var listMaxHeight = $(window).innerHeight()-40;
		$("#list").css("max-height",listMaxHeight+"px");

		var statusLeft = $(window).innerWidth()/2-$("#status").outerWidth()/2;
		var statusTop = $(window).innerHeight()/2-$("#status").outerHeight()/2;
		$("#status").css("left",statusLeft+"px");
		$("#status").css("top",statusTop+"px");

		/* call find resize */
		app.find.resize();

		/* call epg resize */
		app.epg.resize();

		/* call stream-config resize */
		app.streamconfig.resize();
	}
}

/* initialise the app when document is ready */
$(document).ready(function(){app.init();});