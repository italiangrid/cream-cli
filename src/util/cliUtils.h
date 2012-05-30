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


#ifndef _CLIUTILS_H_
#define _CLIUTILS_H_

#include <vector>
#include <string>
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "no_user_confile_ex.h"
#include "glite/ce/cream-client-api-c/auth_ex.h"
#include "internal_ex.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"

#define DEFAULT_CONF_FILE "./glite_cream.conf"
#define DEFAULT_LOG_DIR "/tmp/glite_cream_cli_logs"
#define DEFAULT_CEURL_POSTFIX "/ce-cream/services/CREAM2"
#define DEFAULT_CEURLDELEGATION_POSTFIX "/ce-cream/services/gridsite-delegation"
#define DEFAULT_PROTO "https://"

#define DEFAULT_ES_POSTFIX_CREATE "/ce-cream-es/services/CreationService"
#define DEFAULT_ESDELEGATION_POSTFIX "/ce-cream-es/services/DelegationService"

using namespace std;

namespace util = glite::ce::cream_client_api::util;

namespace cliUtils {
  

  
  void         getJobIDFromFile(std::vector<std::string>&, const char* filename);
  bool         isACreamJobListFile(const char*) throw(util::file_ex&);
  int          openJobListFile(const char*, const bool&) throw(util::file_ex&);
  int          appendToJobListFile(const char*) throw(util::file_ex&);
  void         writeJobID(const int&, const std::string&) throw(util::file_ex&);
  void         removeJobIDFromFile(const std::vector<std::string>&, 
					  const char* filename) throw(util::file_ex&);
  
  std::string  getFileName(const std::string&);
  std::string  getCompleteHostName(const std::string&);
  bool         fileExists(const char*) throw(util::file_ex&);
  bool         fileIsReadable(const char*) throw(util::file_ex&);
  bool         fileIsWritable(const char*) throw(util::file_ex&);

  bool         checkEndpointFormat(const std::string& endpoint) throw(internal_ex&);
  bool         containsTCPPort(const std::string& endpoint) throw(internal_ex&);

  std::vector<std::string>  
    getConfigurationFiles(const std::string& VO, 
			  const std::string& user_specified, 
			  const std::string& _default) throw(no_user_confile_ex&);

  std::string getProxyCertificateFile(void)
    throw (glite::ce::cream_client_api::soap_proxy::auth_ex&);

  std::string getPath(const std::string&);
  void        mkdir(const std::string&);
  std::vector<std::string> expandWildcards(const std::vector<std::string>& ) throw(internal_ex&);
  void        expand(const std::string&, std::vector<std::string>& ) throw(internal_ex&);

  void printResult( const glite::ce::cream_client_api::soap_proxy::ResultWrapper& result ) throw();

  void processResult( const glite::ce::cream_client_api::soap_proxy::ResultWrapper& result, std::map<std::string, std::string>& target ) throw();

  bool interactiveChoice( const char* commandName, 
			  const char* joblist_file,
			  const bool nointeractive, 
			  const bool debug,
			  const bool quiet,
			  bool& process_all_jobs_from_file,
			  std::vector<std::string>& target, 
			  std::vector<std::string>& source, 
			  std::string& errMex ) throw();
};

//______________________________________________________________________________
class stripCreamURL {
  
  std::vector<glite::ce::cream_client_api::soap_proxy::JobIdWrapper> *m_jobids;
  glite::ce::cream_client_api::util::ConfigurationManager *m_confMgr;
  
public:
  
  stripCreamURL( std::vector<glite::ce::cream_client_api::soap_proxy::JobIdWrapper>* target,
		 glite::ce::cream_client_api::util::ConfigurationManager *confMgr ) 
    throw() : m_jobids( target ), m_confMgr(confMgr) 
  {
  }
  
  ~stripCreamURL( void ) throw() {}

  void operator()( const std::string& jid ) ;
};

#endif

