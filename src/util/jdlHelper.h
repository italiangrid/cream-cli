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

#ifndef __JDLH_H__
#define __JDLH_H__

#include <set>
#include <vector>
#include <string>
#include "glite/jdl/JobAd.h"
#include "JDLExceptions.h"



namespace log4cpp {
    class Category;
};

class jdlHelper { //: public AbsJdlHelper {

  std::string 		m_filename;
  std::string           m_jobname;
  glite::jdl::Ad 	m_job;
  log4cpp::Category* 	m_log_dev;
  
  std::vector<std::string>  m_absolute_path;
  
 public:

  std::string getJobName( void ) const { return m_jobname; }

  jdlHelper( const char* );// throw(JDLSyntaxError&, JDLFileNotFound&, JDLFileAccessError&) ;
   
  std::string getFileName( void ) const { return m_filename; }
 
  virtual ~jdlHelper() {};
  virtual bool hasOSB() ;
  virtual bool hasOSBDU() ;
  virtual bool hasOSBBDU();
  virtual bool hasEXE() ;
  virtual bool hasTYPE() ;
  virtual bool hasJOBTYPE() ;
  virtual int  nodeNumber() ;
  virtual bool hasISB() ;
  virtual bool hasISBBU() ;
  virtual void del(const std::string&) throw(JDLEx&);
  virtual void add(const std::string&, const std::string&) throw(JDLEx&, JDLError&);
  virtual void addFrontClassAd(const std::string&);// throw(JDLEx&, JDLError&, glite::jdl::AdSyntaxException);
  virtual bool hasVO() ;
  virtual bool isMPIJOB();
  virtual std::string              getVO();
  virtual std::vector<std::string> getOSB() ;
  virtual std::vector<std::string> getOSBDU() ;
  virtual std::vector<std::string> getOSBBDU() ;
  virtual std::vector<std::string> getISB();
  virtual std::vector<std::string> getISBBU();
  virtual bool			   checkPrologue();
  virtual bool                     checkEpilogue();
  void                             getAbsolutePath( std::set<std::string>& );// { return m_absolute_path; }
  int                              getAbsolutePathSize( void ) { return m_absolute_path.size() ; }

  virtual void process(const std::string&, const std::string&,  std::string) throw(std::exception&);

  virtual std::string toString() { return m_job.toString(); }
  void removeWildcardFromISB( void ) throw(JDLISBError&);
  void addFilesToISB( const std::vector<std::string>& ) throw(JDLISBError&);
  
  void processISB(const std::string&/*, const bool*/ ) throw(JDLISBError&) ;
};



#endif
