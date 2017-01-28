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

	/* initialises the programme list */
	init: function(){
		/* determine max-height and set it */
		var max_h = $(window).innerHeight()-40;
		$("#list").css("max-height",max_h+"px");
	},

	toggle: function(forceOut = false){
		if(app.list.visible == true
			|| forceOut == true){
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