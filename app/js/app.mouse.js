app.mouse = {
	init: function(){
		/* attach mouse ups */
		$(window).mousemove(function(event){
			app.mouse.checkHoverBottom(event.clientX,event.clientY);
		});
		$(window).mouseup(function(event){
			$("#debuginfo").html($("#control").css("opacity"));
		});
	},

	/* check if the mouse hovers at the bottom 
		and we might need to enbable control */
	checkHoverBottom: function(mouseX,mouseY){
		/* get element width and height first */
		elmWidth = $("#control").outerWidth();
		elmHeight = $("#control").outerHeight();

		/* get element position */
		elmPosX = $("#control").position().left;
		elmPosY = $(window).innerHeight()-elmHeight;
		
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
}