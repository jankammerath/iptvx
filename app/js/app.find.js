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

app.find = {
	visible: false,

	navigate: function(down = true){
		var itemCount = $(".founditem").length;
		var activeItem = parseInt($(".activefounditem").attr("data-resultnum"));
		var nextActiveItem = 0;

		/* remove active from element */
		$(".founditem").removeClass("activefounditem");

		if(down == true){
			/* navigate down */
			if(activeItem != itemCount-1){
				/* go down one element */
				nextActiveItem = activeItem + 1;
			}else{
				listScrollTop = 0;
			}
		}else{
			/* navigate up */
			if(activeItem == 0){
				/* this is the first, go to the last */
				nextActiveItem = itemCount - 1;
			}else{
				/* go up one element */
				nextActiveItem = activeItem - 1;
			}
		}

		/* set active item */
		$($(".founditem").get(nextActiveItem)).addClass("activefounditem");

		/* scroll down one item */
		$("#findresult").scrollTop(nextActiveItem*75);
	},

	handleKey: function(keyCode){
		/* get current search text */
		var text = $("#findinputtext").html();

		/* allow to navigate items */
		if(keyCode == 38 || keyCode == 40){
			if(keyCode == 38){
				app.find.navigate(false);
			}if(keyCode == 40){
				app.find.navigate(true);
			}
		}else{
			/* check if key is char */
			if(keyCode >= 65 && keyCode <= 90){
				text += String.fromCharCode(keyCode);
			}

			/* check if key is number */
			if(keyCode >= 48 && keyCode <= 57){
				text += String.fromCharCode(keyCode);
			}

			/* check if key is space */
			if(keyCode == 32){
				text += " ";
			}

			/* check if key is backspace */
			if(keyCode == 8){
				text = text.substring(0,text.length-2);
			}

			/* update current search text */
			$("#findinputtext").html(text.toLowerCase());

			/* query the epg */
			app.find.query(text);
		}
	},

	query: function(text){
		var html = "";

		if(typeof(iptvx)=="object" && text.length >= 3){  
			var resultNum = 0;
			for(var c=0;c<iptvx.epg.length;c++){
            	var chan = iptvx.epg[c];
            	for(var p=0;p<chan.programmeList.length;p++){
            		var prog = chan.programmeList[p];
            		var match = false;
            		var keyword = text.toLowerCase();
            		var progStart = new Date(prog.start*1000);
            		var descText = "";

            		if(progStart > Date.now()){
	            		if(prog.title.toLowerCase().includes(keyword)){
	            			match = true;
	            		}

	            		if(typeof(prog.description)=="string"){
	            			if(prog.description.toLowerCase().includes(keyword)){
	            				match = true;
	            			}
	            			descText = prog.description;
	            		}

	            		if(match){
	            			html += "<div class=\"founditem\" "
	            					+ "data-resultnum=\"" + resultNum + "\" "
	            					+ "data-channel=\"" + c + "\" "
	            					+ "data-programme=\"" + p + "\" "
	            					+ ">"
	            			        + "<div class=\"foundtitle\">" + prog.title + "</div>"
	          						+ "<div class=\"foundchannel\">" + chan.name + "</div>"
	          						+ "<div class=\"foundtext\">";

	          				html += progStart.toISOString().substring(8,10)
	          						+ "." + progStart.toISOString().substring(5,7) + ". "
	          						+ progStart.toTimeString().substring(0,5);

	          				if(descText.length > 0){
	          					html += " <span class=\"founditemdesc\">" + descText + "</span>";
	          				}

	          				html += "</div></div>";

	          				resultNum++;
	            		}
            		}
            	}
        	}
		}

		/* flush html */
		$("#findresult").html(html);

		/* mark first element as active */
		if($(".founditem").length){
			var firstItem = $(".founditem").get(0);
			$(firstItem).addClass("activefounditem");
		}
	},

	toggle: function(forceOut = false){
      	if(app.find.visible == true || forceOut == true){
      		$("#find").fadeOut();

        	/* set indicator to false */
        	app.find.visible = false;
    	}else{
    		$("#find").fadeIn();

         	/* set indicator to true */
        	app.find.visible = true;
    	}
	},

	resize: function(){
		var findLeft = ($(window).innerWidth()/2)-($("#find").outerWidth()/2);
		$("#find").css("left",findLeft+"px");	
		var findTop = ($(window).innerHeight()/2)-($("#find").innerHeight()/2);
		$("#find").css("top",findTop+"px");	
	},
}