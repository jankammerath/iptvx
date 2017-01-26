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
#ifndef JS_H
#define JS_H

#include <JavaScriptCore/JavaScript.h>

/*
   Assigns JS Global Context and initialises
   @param   jsGlobalContext         the global js context to work on
*/
void iptvx_js_init(JSGlobalContextRef jsGlobalContext);


#endif