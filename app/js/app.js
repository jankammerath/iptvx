$(document).ready(function(){
	resize();
});

$(window).resize(function(){
	resize();
});

$(window).keydown(function(event){
	var keyCode = event.which;

	/* fadein and -out with space (32) */
	if(keyCode == 32){
		if($("#ui").is(":visible") == true){
			$("#ui").fadeOut();
		}else{
			$("#ui").fadeIn();
		}
	}

	$("#debuginfo").html("Key pressed: "+event.which+", Character: "+event.key);
});

function resize(){
	var controlLeft = ($(window).innerWidth()/2)-($("#control").outerWidth()/2);
	$("#control").css("left",controlLeft+"px");	
}