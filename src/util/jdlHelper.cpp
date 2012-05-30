/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.
See the License for the specific language governing permissions and
limitations under the License.

END LICENSE */

#include "jdlHelper.h"
#include "cliUtils.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/jdl/RequestAdExceptions.h"
#include <unistd.h>
#include <istream>
#include <fstream>
#include <exception>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include <stdio.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

namespace apiutil = glite::ce::cream_client_api::util;
using namespace std;

//______________________________________________________________________________
jdlHelper::jdlHelper(const char* filename ) 
  : m_filename( filename ), m_absolute_path()
{

  struct stat buf;
  if(stat(filename, &buf) < 0) {
    int saveerr = errno;
    if(saveerr == ENOENT) {
      string err  = string("JDL File ")+filename+" missing on disk";
      throw JDLFileNotFound(err);
    }
    else
      throw JDLFileNotFound(strerror(saveerr));
  }
  
  if(!(buf.st_mode & S_IRUSR))
    throw JDLFileAccessError("JDL file is there but it is not readable");
  
  ifstream is( filename, ios::in );
  glite::jdl::JobAd tempJob;
  //try {
    tempJob.fromStream(is);
  /*} catch(exception
  catch(glite::jdl::AdSyntaxException& ex) {
    throw JDLSyntaxError(ex.what());
  }
  catch(glite::jdl::AdClassAdException& ex) {
    // ERROR
    throw JDLSyntaxError(ex.what());
  }
  catch(glite::jdl::AdMismatchException& ex) {
    // ERROR
    throw JDLSyntaxError(ex.what());
  }
  catch(glite::jdl::AdListException& ex) {
    // ERROR
    throw JDLSyntaxError(ex.what());
  }
  catch(glite::jdl::AdFormatException& ex) {
  
    // ERROR
    throw JDLSyntaxError(ex.what());
  
  } catch(exception& ex) {
  
    throw JDLSyntaxError(ex.what());
  
  } */
  is.close();
  string goodJDL = tempJob.toString();
  m_job = glite::jdl::Ad(goodJDL);
  //delete(is);
  
  m_log_dev = apiutil::creamApiLogger::instance()->getLogger();

  m_jobname = string("JobName_")+boost::lexical_cast<string>((long)(this));
}

//______________________________________________________________________________
void jdlHelper::process(const string& qn, const string& bs, 
			string user_specified_vo) throw(exception&)
{
  vector<string> OSB,OSBDU,OSBBDU,ISB,ISBBU;
  
  // ******************************************************************************
  // MUST HAVE "executable" ATTRIBUTE
  // ******************************************************************************
  if(!m_job.hasAttribute("executable"))
    throw JDLEXENotFound("Missing 'executable' mandatory attribute in the JDL");
  
  
  // ******************************************************************************
  // NOW CHECKING OSB
  // ******************************************************************************
  bool osb    = m_job.hasAttribute("OutputSandbox");
  bool osbbdu = m_job.hasAttribute("OutputSandboxBaseDestURI");
  bool osbdu  = m_job.hasAttribute("OutputSandboxDestURI");
  if(osb) {
    if(!osbbdu && !osbdu) {
      throw JDLOSBError("If 'OutputSandbox' is specified then one (and only one) of the two attributes 'OutputSandboxDestURI' and 'OutputSandboxBaseDestURI' must be specified");
    }
    if(osbbdu && osbdu)
      throw JDLOSBError("Cannot specify both OutputSandboxDestURI and OutputSandboxBaseDestURI");
    
  }
  
  if((osbdu || osbbdu) && !osb) {
    try {
      m_job.delAttribute("OutputSandboxDestURI");
      m_job.delAttribute("OutputSandboxBaseDestURI");
    } catch(...) {}
  }
  
  if(osb) {
    try {
      OSB = m_job.getStringValue("OutputSandbox");
      unsigned int dusize = 0;
      if(osbdu) {OSBDU = m_job.getStringValue("OutputSandboxDestURI");dusize=OSBDU.size();}
      if(osbbdu) {
	OSBBDU = m_job.getStringValue("OutputSandboxBaseDestURI");
	dusize=OSB.size();
	if(OSBBDU.size() > 1)
	  throw JDLOSBError("OutputSandboxBaseDestURI must be a string and NOT a list");
	
      }
      if(OSB.size() != dusize)
	throw JDLOSBError("OutputSandbox and OutputSandboxDestURI must have the same cardinality");
      
    } catch(glite::wmsutils::exception::Exception ex) {
      throw JDLParseError(ex.what());
    }
  }
  
  // ************************************************************************
  // NOW CHECKING TYPE/JOBTYPE
  // ************************************************************************
  vector<string> type;
  try { type = m_job.getStringValue("Type"); }
  catch(glite::wmsutils::exception::Exception ex) {}
  if(type.size()) {

    if( !boost::iequals(type.at(0), "Job") )
      {
        // ERROR
	throw JDLTYPEError("CREAM only supports 'Job' as value of 'Type' attribute");
      }
  }
  vector<string> jtype;
  try { jtype = m_job.getStringValue("JobType"); }
  catch(glite::wmsutils::exception::Exception ex) {}
  if(jtype.size()) {
    if( !boost::iequals(jtype.at(0), "Normal") && !boost::iequals(jtype.at(0), "Mpich") )
      // ERROR
      throw JDLJOBTYPEError("CREAM only supports 'Normal' and 'Mpich' as values of 'JobType' attribute");

    if( boost::iequals(jtype.at(0),"Mpich") ) {
      if( !m_job.hasAttribute("CPUNumber") )
        if ( !m_job.hasAttribute("NodeNumber") )
	  // ERROR
          throw JDLMPIError("Declared Mpich JobType requires specification of CPUNumber or NodeNumber attribute");
    }
  }
  
  // ****************************************************************************
  // NOW CHECKING ISB
  // ****************************************************************************
  bool isb   = m_job.hasAttribute("InputSandbox");
  bool isbbu = m_job.hasAttribute("InputSandboxBaseURI");
  if(isb)
    ISB = m_job.getStringValue("InputSandbox");
  if(isbbu)
    ISBBU = m_job.getStringValue("InputSandboxBaseURI");
  if(isbbu && !isb) {
    m_log_dev->warnStream()
	<< "Specified InputSandboxBaseURI without InputSandbox. Removing it..."
	;
    
    m_job.delAttribute("InputSandboxBaseURI");
  }
  
  if(ISB.size() == 1) {
    try {
      string value = string("{\"") + ISB.at(0) + "\"}";
      m_job.setAttributeExpr("backup", value);
      m_job.delAttribute("InputSandbox");
      m_job.setAttributeExpr("InputSandbox", m_job.delAttribute("backup") );
    } catch(glite::jdl::AdSyntaxException& syn) {
      throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
    } catch(glite::jdl::AdEmptyException& syn) {
      throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
    }
  }
  
  if(m_job.hasAttribute("QueueName")) {
    m_log_dev->warnStream()
	<< "Removing [QueueName] attribute from JDL and adding "
	<< "that one from command line argument..."
	;
    m_job.delAttribute(string("QueueName"));
  }
  if(m_job.hasAttribute("BatchSystem")) {
    m_log_dev->warnStream() << "Removing [BatchSystem] "
	<< "attribute from JDL and adding that one from command "
	<< "line argument..."
	;
    m_job.delAttribute(string("BatchSystem"));
  }
  try {
    m_job.setAttribute(string("QueueName"), qn);
    m_job.setAttribute(string("BatchSystem"), bs);
  } catch(glite::jdl::AdEmptyException& ad) {
    throw JDLError(ad.what());
  }
  catch(glite::jdl::AdFormatException& ad) {
    throw JDLError(ad.what());
  }
  
  if(user_specified_vo!="") {
    if(hasVO()) {
      m_log_dev->warnStream()
	  << "VirtualOrganisation specified in the JDL but overrided with ["
	  << user_specified_vo << "]" ;
      m_job.delAttribute(string("VirtualOrganisation"));
    }
    try { m_job.setAttribute(string("VirtualOrganisation"), user_specified_vo); }
    catch(glite::jdl::AdSyntaxException& syn) {
      throw JDLError(syn.what());
    } catch(glite::jdl::AdEmptyException& syn) {
      throw JDLError(syn.what());
    }
    catch(glite::jdl::AdFormatException& ad) {
      throw JDLError(ad.what());
    }
  }

  // ****************************************************************************
  // NOW CHECKING PERUSAL*
  // ****************************************************************************
  if( m_job.hasAttribute("perusalfileenable") )
  {
    //vector<string> perusal;
    vector<bool> perusal;
    //try {
      perusal = m_job.getBoolValue("PerusalFileEnable");
    //}
    //if( boost::iequals( perusal.at(0), "true") ) {
    if(perusal.at(0)) {
      if( (!m_job.hasAttribute("perusaltimeinterval")) 
          || (!m_job.hasAttribute("perusalfilesdesturi")) 
	  || (!m_job.hasAttribute("perusallistfileuri")) ) 
      {
          //m_log_dev->warnStream() 
	  //  << "PerusalFileEnable is set to true but one or more of PerusalFilesDestURI/PerusalListFileURI/PerusalTimeInterval "
          //  << "parameter are not set. Stop"
          //  ;
        throw JDLError("PerusalFileEnable is set to true but one or more of PerusalFilesDestURI/PerusalListFileURI/PerusalTimeInterval parameter are not set.");
      }
    }
  } 
}

//______________________________________________________________________________
bool jdlHelper::hasOSB()  {
  return m_job.hasAttribute("OutputSandbox");
}

//______________________________________________________________________________
bool jdlHelper::hasOSBDU()  {
  return m_job.hasAttribute("OutputSandboxDestURI");
}

//______________________________________________________________________________
bool jdlHelper::hasOSBBDU()  {
  return m_job.hasAttribute("OutputSandboxBaseDestURI");
}

//______________________________________________________________________________
bool jdlHelper::hasEXE()  {
   return m_job.hasAttribute("executable");
}

//______________________________________________________________________________
bool jdlHelper::hasTYPE()  {
   return m_job.hasAttribute("Type");
}

bool jdlHelper::hasJOBTYPE()  {
   return m_job.hasAttribute("JobType");
}

//______________________________________________________________________________
int  jdlHelper::nodeNumber()  {
   return m_job.hasAttribute("NodeNumber");
}

//______________________________________________________________________________
bool jdlHelper::hasISB()  {
   return m_job.hasAttribute("InputSandbox");
}

//______________________________________________________________________________
bool jdlHelper::hasISBBU()  {
   return m_job.hasAttribute("InputSandboxBaseURI");
}

//______________________________________________________________________________
void jdlHelper::add(const string& attr, const string& val) 
  throw(JDLEx&, JDLError&) 
{
   try { m_job.setAttribute(attr, val); }
   catch(glite::jdl::AdEmptyException& ad) {
	throw JDLError(ad.what());
   }
   catch(glite::jdl::AdFormatException& ad) {
	throw JDLError(ad.what());
   }
}

//______________________________________________________________________________
void jdlHelper::addFrontClassAd(const string& ad) 
// throw(JDLEx&, JDLError&,glite::jdl::AdSyntaxException) 
{
  //cout << "Will add [" << ad << "]" << endl;
  if( ad.empty() )
    return;

  glite::jdl::Ad newAd(ad);
  newAd.merge(m_job);
  m_job.fromString( newAd.toString() );
}

//______________________________________________________________________________
void jdlHelper::del(const string& attr) throw(JDLEx&) {
   try { m_job.delAttribute(attr); }
   catch(glite::jdl::AdEmptyException& ad) {
	throw JDLError(ad.what());
   }
   catch(glite::jdl::AdFormatException& ad) {
	throw JDLError(ad.what());
   }
}

//______________________________________________________________________________
bool jdlHelper::hasVO() {
   return m_job.hasAttribute("VirtualOrganisation");
}

//______________________________________________________________________________
bool jdlHelper::isMPIJOB() {
   return m_job.hasAttribute("Mpich");
}

//______________________________________________________________________________
string jdlHelper::getVO()  {
  if(hasVO()) return m_job.getStringValue("VirtualOrganisation").at(0);
  else return "";
}

//______________________________________________________________________________
vector<string> jdlHelper::getOSB()  {
  if(hasOSB()) 
    return m_job.getStringValue(string("OutputSandbox"));
  else return vector<string>();
}

//______________________________________________________________________________
vector<string> jdlHelper::getOSBDU()  {
  if(hasOSBDU()) return m_job.getStringValue("OutputSandboxDestURI");
  else return vector<string>();
}

//______________________________________________________________________________
vector<string> jdlHelper::getOSBBDU()  {
  if(hasOSBBDU()) return m_job.getStringValue("OutputSandboxBaseDestURI");
  else return vector<string>();
}

//______________________________________________________________________________
vector<string> jdlHelper::getISB()  {
  if(hasISB()) return m_job.getStringValue("InputSandbox");
  else return vector<string>();
}

//______________________________________________________________________________
vector<string> jdlHelper::getISBBU()  {
  if(hasISBBU()) return m_job.getStringValue("InputSandboxBaseURI");
  else return vector<string>();
}

//______________________________________________________________________________
bool jdlHelper::checkPrologue() {
  if(m_job.hasAttribute("PrologueArguments") && !m_job.hasAttribute("Prologue") )
    return false;
  return true;
}

//______________________________________________________________________________
bool jdlHelper::checkEpilogue() {
  if(m_job.hasAttribute("EpilogueArguments") && !m_job.hasAttribute("Epilogue") )
    return false;
  return true;
}

//______________________________________________________________________________
void jdlHelper::removeWildcardFromISB() throw(JDLISBError&) 
{
  vector<string> tmpISB = this->getISB();
  string newISB = "{";
  if( tmpISB.empty() ) return;
  
  for(vector<string>::const_iterator it = tmpISB.begin();
      it != tmpISB.end();
      ++it)
    {
      
      if(boost::starts_with( *it, "gsiftp://") ||
	 boost::starts_with( *it, "http://") ||
	 boost::starts_with( *it, "https://") )
	newISB += "\"" + *it + "\",";
      else {
	if( (it->find("*", 0) == string::npos) 
	    && (it->find("?", 0) == string::npos) ) // no wildcard present
	  newISB += "\"" + *it + "\",";
      }
    }
  
  boost::trim_if( newISB, boost::is_any_of(",") );
  newISB += "}";
  
  try {
    m_job.delAttribute("InputSandbox");
    m_job.setAttributeExpr("InputSandbox", newISB );
  } catch(glite::jdl::AdSyntaxException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(glite::jdl::AdEmptyException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(std::exception& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  }
}

//______________________________________________________________________________
void jdlHelper::addFilesToISB( const vector<string>& _files) throw(JDLISBError&) 
{
  if( !_files.size() ) return;
  vector<string> tmpISB = this->getISB();
  vector<string> files = _files;
  std::sort( tmpISB.begin(), tmpISB.end() );
  std::sort( files.begin(), files.end() );
  vector<string> missing( tmpISB.size() );
  
  std::set_difference( tmpISB.begin(), tmpISB.end(), 
                       files.begin(), files.end(), 
		       missing.begin() );
  
  for(vector<string>::const_iterator it = missing.begin();
      it != missing.end();
      it++)
  {
    if(it->length()) files.push_back( *it );
  }
  
  std::string newISB = "{";
  for(vector<string>::const_iterator it = files.begin();
      it != files.end();
      ++it)
  {
  
    if ( boost::starts_with( *it, "file://")) continue;
    if ( !boost::starts_with( *it, "/") & 
    	 !boost::starts_with( *it, "gsiftp://") &
	 !boost::starts_with( *it, "http://") &
	 !boost::starts_with( *it, "https://") ) continue;
	 
//    cout << "*it=[" << *it << "]" << endl;
  
    newISB += "\"" + *it + "\",";
  }
  boost::trim_if( newISB, boost::is_any_of(",") );
  newISB += "}";
  
  try {
    m_job.delAttribute("InputSandbox");

    m_job.setAttributeExpr("InputSandbox", newISB );
  } catch(glite::jdl::AdSyntaxException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(glite::jdl::AdEmptyException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(std::exception& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  }
}

//______________________________________________________________________________
void jdlHelper::processISB(const string& CWD /*, const bool isb_base_uri*/)
   throw(JDLISBError&) 
{

  vector<string> tmpISB( this->getISB() );
  
  if(tmpISB.empty()) return;
  
  vector<string> remote_paths;
  vector<string>::const_iterator it = tmpISB.begin();
  
  
  for( ; it!= tmpISB.end(); ++it ) {
  
  
    m_log_dev->debugStream() <<  "Processing file [" << *it << "]...";
  
  
  
    if (boost::starts_with( *it, "gsiftp://") ||
        boost::starts_with( *it, "http://") ||
	boost::starts_with( *it, "https://") )
    {
      remote_paths.push_back( *it );
      continue;
    }
    
    if( boost::starts_with( *it, "file://") ) {
      string thisFile( *it );
      thisFile.replace(0, 7, "");
      m_log_dev->debugStream() <<  "Adding absolute path [" << thisFile << "/" << *it << "]...";
      m_absolute_path.push_back( thisFile );
      continue;
    }
    
    if( boost::starts_with( *it, "/") ) {
      m_absolute_path.push_back( *it );
      continue;
    }
    
    /**
      If here, file path is relative. Then must put prefix CWD is isb_base_uri is FALSE
    */
    if( !this->hasISBBU() ) {
      m_log_dev->debugStream() <<  "Adding absolute path [" << CWD << "/" << *it << "]...";
      m_absolute_path.push_back( CWD + "/" + *it );
      
    } else {
    
      remote_paths.push_back (*it);
    
    }
    
  }
  
  m_absolute_path = cliUtils::expandWildcards( m_absolute_path );
  
  string newISB( "{" );
  for(it=remote_paths.begin(); it != remote_paths.end(); ++it ) {
    newISB += "\"" + *it + "\",";
  }
  
  for(it=m_absolute_path.begin(); it != m_absolute_path.end(); ++it ) {
    newISB += "\"" + *it + "\",";
  }
  
  boost::trim_if( newISB, boost::is_any_of(",") );
  newISB += "}";
  
  m_log_dev->debugStream() <<  "Inserting mangled InputSandbox in JDL: [" << newISB << "]...";
  
  try {
    m_job.delAttribute("InputSandbox");
    m_job.setAttributeExpr("InputSandbox", newISB );
  } catch(glite::jdl::AdSyntaxException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(glite::jdl::AdEmptyException& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  } catch(std::exception& syn) {
    throw JDLISBError(string("Syntax Exception: [") + syn.what() + "]");
  }
  
//   m_log_dev->debugStream() <<  "Insertion completed!";
//   
//   for( it = m_absolute_path.begin(); it!=m_absolute_path.end(); ++it ) {
//     cout << "abs path=" << *it << endl;
//   }
}

//______________________________________________________________________________
void jdlHelper::getAbsolutePath( set<string>& paths ) 
{
  vector<string>::iterator it = m_absolute_path.begin();
  for( ; it != m_absolute_path.end( ); ++it ) {
    //    cout << "******************* Inserting [" << *it << "]" << endl;
    paths.insert( *it );
  }
}
