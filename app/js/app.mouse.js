app.mouse = {
	init: function(){
		/* attach mouse ups */
		$(window).mousemove(function(event){
			app.mouse.checkControlHover(event.clientX,event.clientY);
			app.mouse.checkListHover(event.clientX,event.clientY);
		});
		$(window).mouseup(function(event){
			/* allow mouse scroll wheel for channel list */
			app.mouse.checkChannelScroll(event.clientX,event.clientY,event.button);

			/* show EPG when click into empty space */
			if(document.elementFromPoint(event.clientX,event.clientY).tagName == "HTML"
				&& event.button == 1){
				/* left mouse click in empty space */
				app.list.toggle(true);
				app.find.toggle(true);
				app.control.toggle(true);
				app.epg.toggle();
			}
		});
	},

	checkChannelScroll: function(mouseX,mouseY,button){
		/* get element width and height first */
		elmWidth = $("#list").outerWidth();
		elmHeight = $("#list").outerHeight();

		/* get element position */
		elmPosY = $("#list").position().top;
		elmPosX = 0;

		/* check if list needs to be enabled */
		if(app.list.visible == true){
			if(mouseX > elmPosX && mouseX < (elmPosX+elmWidth+20)
				&& mouseY > elmPosY && mouseY < (elmPosY+elmHeight)){
				/* mouse is inside perimeter of list */
				if(button == 4){
					/* scroll up */
					app.list.handleKey(38);
				}if(button == 5){
					/* scroll down */
					app.list.handleKey(40);
				}if(button == 2){
					/* select current channel */
					app.list.handleKey(13);
				}

				/* check if it is a click of the left button */
				if(button == 1){
					var epgData = app.epg.getEpgData();
					for(var c=0;c<epgData.length;c++){
						var chan_elm = $("#listchannel"+c);
						if(mouseX > $(chan_elm).position().left 
							&& mouseX < ($(chan_elm).position().left+$(chan_elm).outerWidth())
							&& mouseY > $(chan_elm).position().top  
							&& mouseY < ($(chan_elm).position().top+$(chan_elm).outerHeight())){
							/* obviously channel was clicked */
							app.list.selectedChannel = c;
							app.epg.switchChannel(app.list.selectedChannel);
							app.list.update();
						}
					}
				}
			}
		}
	},

	/* check if the mouse hovers at the bottom 
		and we might need to enbable control */
	checkControlHover: function(mouseX,mouseY){
		/* get element width and height first */
		elmWidth = $("#control").outerWidth();
		elmHeight = $("#control").outerHeight();

		/* get element position */
		elmPosX = $("#control").position().left;
		elmPosY = $(window).innerHeight()-elmHeight;

		if(app.epg.visible == false){
			if(app.control.visible == false){
				if(mouseX > elmPosX && mouseX < (elmPosX+elmWidth)
					&& mouseY > elmPosY && mouseY < (elmPosY+elmHeight)){
					/* mouse is inside perimeter of control app */
					app.control.toggle(false);
				}
			}else{
				/* fade out control when out of bounding box */
				if(mouseX < elmPosX || mouseX > (elmPosX+elmWidth)
					|| mouseY < elmPosY || mouseY > (elmPosY+elmHeight)){
					/* mouse is inside perimeter of control app */
					app.control.toggle(true);
				}
			}
		}
	},

	checkListHover: function(mouseX,mouseY){
		/* get element width and height first */
		elmWidth = $("#list").outerWidth();
		elmHeight = $("#list").outerHeight();

		/* get element position */
		elmPosY = $("#list").position().top;
		elmPosX = 0;

		/* check if list needs to be enabled */
		if(app.epg.visible == false){
			if(app.list.visible == false){
				if(mouseX > elmPosX && mouseX < (elmPosX+elmWidth+20)
					&& mouseY > elmPosY && mouseY < (elmPosY+elmHeight)){
					/* mouse is inside perimeter of list */
					app.list.toggle(false);
				}
			}else{
				/* fade out list when out of bounding box */
				if(mouseX < elmPosX || mouseX > (elmPosX+elmWidth+20)
					|| mouseY < elmPosY || mouseY > (elmPosY+elmHeight)){
					/* mouse is inside perimeter of list app */
					app.list.toggle(true);
				}
			}
		}
	}
}