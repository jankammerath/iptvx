$(document).ready(function(){
	resize();
});

$(window).resize(function(){
	resize();
});

function resize(){
	var controlLeft = ($(window).innerWidth()/2)-($("#control").outerWidth()/2);
	$("#control").css("left",controlLeft+"px");	
}