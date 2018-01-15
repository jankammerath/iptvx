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

app.record = {
   visible: false,
   list: {},

   init: function(){
      /* TODO */
   },

   /* toggles the visibility of the record ui */
   toggle: function(forceOut = false){
      if(app.record.visible == true || forceOut == true){
         /* fade out to bottom */
         $("#record").animate({
            opacity: 0,
            top: $("#record").outerHeight()*-1
         }, 500);

         /* set indicator to false */
         app.record.visible = false;
      }else{
         /* fade in from bottom */
         $("#record").animate({
            opacity: 0.95,
            top: 0
         }, 500);

         /* set indicator to true */
         app.record.visible = true;
      }      
   }
}