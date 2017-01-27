app.control = {
	toggle: function(){
		if($("#control").is(":visible") == true){
			$("#control").fadeOut();
		}else{
			$("#control").fadeIn();
		}
	}
}