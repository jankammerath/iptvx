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

/*
	Translates an SDL keycode to a GDK_KEY keycode
	@param			sdl_keycode			The SDL keycode
	@return								The JavaScript keycode

	GDK Keysym: 	https://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
	SDL Keysym:		https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlkey.html
					Also check the SDL_keysym.h file for numbers
*/
int keycode_convert_sdl_to_gtk(int sdl_keycode){
	int result = sdl_keycode;

	/* CHARACTER A-Z KEYS */
	if(sdl_keycode >= 97 && sdl_keycode <= 122){
		result = sdl_keycode - 32;
	}

	/* FUNCTION KEYS */
	if(sdl_keycode >= 282 && sdl_keycode <= 293){
		result = sdl_keycode + 65188;
	}	

	/* KEYPAD NUMBERS */
	if(sdl_keycode >= 256 && sdl_keycode <= 265){
		result = sdl_keycode + 65200;
	}

	/* BACKSPACE */
	if(sdl_keycode == 8){result=65288;}

	/* TAB */
	if(sdl_keycode == 9){result=65289;}

	/* ENTER or RETURN */
	if(sdl_keycode == 13){result=65293;}

	/* UP KEY */
	if(sdl_keycode == 273){result=65362;}

	/* DOWN KEY */
	if(sdl_keycode == 274){result=65364;}

	/* RIGHT KEY */
	if(sdl_keycode == 275){result=65363;}

	/* LEFT KEY */
	if(sdl_keycode == 276){result=65361;}

	/* INSERT KEY */
	if(sdl_keycode == 277){result=65379;}

	/* HOME KEY */
	if(sdl_keycode == 278){result=65360;}

	/* END KEY */
	if(sdl_keycode == 279){result=65367;}

	/* PAGE UP */
	if(sdl_keycode == 280){result=65365;}

	/* PAGE DOWN */
	if(sdl_keycode == 281){result=65366;}

	/* CAPS LOCK */
	if(sdl_keycode == 301){result=65509;}

	/* SHIFT */
	if(sdl_keycode == 303 || sdl_keycode == 304){result=65505;}

	/* CTRL */
	if(sdl_keycode == 305 || sdl_keycode == 306){result=65507;}

	/* L-ALT and R-ALT */
	if(sdl_keycode == 308 || sdl_keycode == 307){result=65513;}

	/* ALT-GR */
	if(sdl_keycode == 313){result=65514;}

	/* SUPER */
	if(sdl_keycode == 311 || sdl_keycode == 312){result=65515;}

	return result;
}