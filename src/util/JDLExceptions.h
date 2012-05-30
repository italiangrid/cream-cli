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

#ifndef __JDLEXCEPTIONS__
#define __JDLEXCEPTIONS__

#include "JDLEx.h"

class JDLError : public JDLEx {
public:
 JDLError(const char* e) : JDLEx(e) {}
 JDLError(const std::string& e) : JDLEx(e) {}
};


class JDLEXENotFound : public JDLEx {
public:
 JDLEXENotFound(const char* e) : JDLEx(e) {}
 JDLEXENotFound(const std::string& e) : JDLEx(e) {}
};


class JDLFileAccessError : public JDLEx {
public:
 JDLFileAccessError(const char* e) : JDLEx(e) {}
 JDLFileAccessError(const std::string& e) : JDLEx(e) {}
};


class JDLFileNotFound : public JDLEx {
public:
 JDLFileNotFound(const char* e) : JDLEx(e) {}
 JDLFileNotFound(const std::string& e) : JDLEx(e) {}
};


class JDLISBError : public JDLEx {
public:
 JDLISBError(const char* e) : JDLEx(e) {}
 JDLISBError(const std::string& e) : JDLEx(e) {}
};


class JDLJOBTYPEError : public JDLEx {
public:
 JDLJOBTYPEError(const char* e) : JDLEx(e) {}
 JDLJOBTYPEError(const std::string& e) : JDLEx(e) {}
};


class JDLMPIError : public JDLEx {
public:
 JDLMPIError(const char* e) : JDLEx(e) {}
 JDLMPIError(const std::string& e) : JDLEx(e) {}
};


class JDLOSBError : public JDLEx {
public:
 JDLOSBError(const char* e) : JDLEx(e) {}
 JDLOSBError(const std::string& e) : JDLEx(e) {}
};


class JDLParseError : public JDLEx {
public:
 JDLParseError(const char* e) : JDLEx(e) {}
 JDLParseError(const std::string& e) : JDLEx(e) {}
};


class JDLSyntaxError : public JDLEx {
public:
 JDLSyntaxError(const char* e) : JDLEx(e) {}
 JDLSyntaxError(const std::string& e) : JDLEx(e) {}
};


class JDLTYPEError : public JDLEx {
public:
 JDLTYPEError(const char* e) : JDLEx(e) {}
 JDLTYPEError(const std::string& e) : JDLEx(e) {}
};


class JDLVONotFound : public JDLEx {
public:
 JDLVONotFound(const char* e) : JDLEx(e) {}
 JDLVONotFound(const std::string& e) : JDLEx(e) {}
};


#endif
