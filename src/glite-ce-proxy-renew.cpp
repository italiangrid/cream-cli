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

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <istream>
#include <fstream>
#include <sys/types.h>

#include "hmessages.h"
#include "util/cliUtils.h"
#include "services/cli_service_proxy_renew.h"

#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp(void) ;
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {
  
  string jobID;
  string endpoint;
  bool debug = false;
  bool hostset=false;
  char c;
  int option_index = 0;
  string logfile;
  bool redir_out = false;
  bool WRITE_LOG_ON_FILE = true;
  bool nomsg = false;
  bool noint = false;
  vector<string> all_delegationids_to_renew;
  string    user_conf_file = "";
  string certfile;

  int timeout = 30;
  
  bool verify_ac_sign = true;
  
  po::options_description desc("Usage");
  try {
  desc.add_options()
    ("help,h", "display this help and exit")
    (
     "debug,d",
     "run in daemon mode and save the pid in this file"
     )
    (
     "version,v", "display version and exit"
     )
    (
     "nomsg,n",
     "Do not write any message on stdout"
     )
    (
     "logfile,l",
     po::value<string>(&logfile)->default_value(""),
     "Set the lof filename"
     )
    (
     "conf,c",
     po::value<string>(&user_conf_file)->default_value(""),
     "Set the configuration filename"
     )
    (
     "proxyfile,p",
     po::value<string>(&certfile)->default_value(cliUtils::getProxyCertificateFile()),
     "Set the proxy filename"
     )
    (
     "timeout,t",
     po::value<int>(&timeout)->default_value(30),
     "Set the soap connection timeout in seconds"
     )
    (
     "donot-verify-ac-sign,A",
     "Disable AC's signature verification"
     )
    (
     "endpoint,e", po::value<string>(&endpoint), "Set the endpoint to ask the submission enable status"
     )
    (
     "delegationId,D", po::value<vector<string> >(&all_delegationids_to_renew), ""
     )
    ;
  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    cerr << "FATAL: " << ex.what() << endl;
    return 1;
  } 
  
  po::positional_options_description p;
  p.add("delegationId", -1);
  po::variables_map vm;
  
  try {
    
    po::store(po::command_line_parser(argc, argv).
	      options(desc).positional(p).run(), vm);
    po::notify( vm );
    
  } catch( std::exception& ex ) {
    
    cerr << "There was an error parsing the command line. "
	 << "Error was: " << ex.what() << endl
	 << "Type " << argv[0] 
	 << " --help for the list of available options"
	 << endl;
    exit( 1 );
    
  }
  
  if( vm.count("debug") && vm.count("nomsg") ) {
    cerr << "Cannot specify both --debug and --nomsg options. Stop." << endl;
    return 1;
  }

  debug = vm.count("debug") != 0;
  nomsg = vm.count("nomsg") != 0;
    
  if( vm.count("donot-verify-ac-sign") )
    verify_ac_sign = false;
    
  if ( vm.count("help") ) {
    printhelp();
    return(0);
  }
  
  if ( vm.count("version") ) {
    help_messages::printver();
    return(0);
  }
  
    if( !logfile.empty( ) )
      redir_out = true;
    
  if( !vm.count("endpoint" ) ) {
    cerr << "endpoint argument is mandatory. Stop." << endl; 
    return 1;
  }
  
  if( !vm.count("delegationId" ) ) {
    cerr << "delegation ID argument is mandatory. Stop." << endl; 
    return 1;
  }
  
  //  endpoint = vm["endpoint"].as< string>();

  glite::ce::cream_cli::services::cli_service_common_options opt;
  opt.m_debug = debug;
  opt.m_nomsg = nomsg;
  opt.m_noint = false;
  opt.m_verify_ac_sign = verify_ac_sign;
  opt.m_redir_out = redir_out;
  opt.m_user_conf_file = user_conf_file;
  opt.m_certfile = certfile;
  opt.m_logfile = logfile;
  opt.m_endpoint = endpoint;
  opt.m_soap_timeout = timeout;  

  int ok = 0;

  for(vector<string>::const_iterator it = all_delegationids_to_renew.begin();
      it != all_delegationids_to_renew.end();
      ++it )
    {
      glite::ce::cream_cli::services::cli_service_proxy_renew PR( opt );
      
      PR.set_delegation_id( *it );
      
      if(PR.execute( ) ) {
	PR.getLogger()->fatal( PR.get_execution_error_message( ) );
	ok = 1;
        continue;
      }
      
      cout << "Proxy with delegation id [" << *it 
	   << " succesfully renewed to endpoint [" << endpoint << "]" << endl;
      
    }
  
  return ok;
}

void printhelp() {
  printf("%s\n",help_messages::getRenewHelp().c_str());
}
