/*
  Copyright (c) Members of the EGEE Collaboration. 2004. 
  See http://www.eu-egee.org/partners/ for details on the copyright
  holders.  
  
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

#ifndef __NOUSERCONFEX__
#define __NOUSERCONFEX__

#include <string>
#include <exception>

class no_user_confile_ex : public std::exception {
  std::string err;
 public:
  no_user_confile_ex() : err("") { }
  no_user_confile_ex(const char* e) :err(e) { }
  no_user_confile_ex(const std::string& e) : err(e) { }
  virtual ~no_user_confile_ex() throw() {}
  const char* what() throw() { return err.c_str(); }
};

#endif
