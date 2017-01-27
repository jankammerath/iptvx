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

/*
	Translates an SDL keycode to a JavaScript keycode
	@param			sdl_keycode			The SDL keycode
	@return								The JavaScript keycode
*/
int keycode_convert_sdl_to_js(int sdl_keycode){
	int result;

	/* SDL key is default */
	result = sdl_keycode;

	/* calculate the characters A-Z */
	if(sdl_keycode >= 97 && sdl_keycode <= 122){
		result = sdl_keycode - 32;
	}

	/* calculate function keys */
	if(sdl_keycode >= 282 && sdl_keycode <= 293){
		result = sdl_keycode - 170;
	}	

	/* calculate the keypad numbers */
	if(sdl_keycode >= 256 && sdl_keycode <= 265){
		result = sdl_keycode - 208;
	}

	/* define delete key */
	if(sdl_keycode == 127){result=46;}

	/* define up, down, pgup, pgdown, home etc. */
	if(sdl_keycode == 271){result=13;}
	if(sdl_keycode == 273){result=38;}
	if(sdl_keycode == 274){result=40;}
	if(sdl_keycode == 275){result=39;}
	if(sdl_keycode == 276){result=37;}
	if(sdl_keycode == 277){result=45;}
	if(sdl_keycode == 278){result=36;}
	if(sdl_keycode == 279){result=35;}
	if(sdl_keycode == 280){result=33;}
	if(sdl_keycode == 281){result=34;}

	/* define caps, shift, ctrl, alt, super */
	if(sdl_keycode == 301){result=20;}
	if(sdl_keycode == 303 || sdl_keycode == 304){result=16;}
	if(sdl_keycode == 305 || sdl_keycode == 306){result=17;}
	if(sdl_keycode == 307 || sdl_keycode == 308){result=18;}
	if(sdl_keycode == 311 || sdl_keycode == 312){result=91;}

	return result;
}