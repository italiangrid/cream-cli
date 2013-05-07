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

/*
        glite-ce-enable-submission: a command line interface to set to ON the submission on a CREAM service

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-enable-submission.cpp,v 1.12.4.1.4.4.2.1.2.3.2.4 2012/09/14 09:10:29 adorigo Exp $
*/

/**
 *
 * System Headers
 *
 */
#include <iostream>
#include <istream>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

/**
 *
 * CLI Headers
 *
 */
#include <boost/program_options.hpp>
#include "services/cli_service_enable_submission.h"
#include "util/cliUtils.h"
#include "hmessages.h"

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

/**
 *
 * BOOST Headers
 *
 */
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

//#define DEFAULT_CONF_FILE "./glite_cream.conf"
//#define DEFAULT_LOG_DIR "/tmp/glite_cream_cli_logs"
//#define DEFAULT_CEURL_POSTFIX "/ce-cream/services/CREAM"
//#define DEFAULT_PROTO "https://"

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::cream_exceptions;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp() ;
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  string endpoint;
  bool debug = false;
  string logfile;
  bool redir_out = false;
  bool nomsg=false;
//  bool noint=false;

  bool WRITE_LOG_ON_FILE = true;
  string user_conf_file = "";
  creamApiLogger* logger_instance = creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();
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
	 ("endpoint", po::value<string>(), "Set the endpoint where enable submission")
	;
    } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      cerr << "FATAL: " << ex.what() << endl;
      return 1;
    }

    po::positional_options_description p;
    p.add("endpoint", -1);
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


  if( vm.count("debug") ) {
    debug = true;
    log_dev->setPriority( log4cpp::Priority::INFO );
  }
    
  if( vm.count("nomsg") ) {
    nomsg = true;
    log_dev->setPriority( log4cpp::Priority::ERROR );
  }
  
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
    
  if( vm.count("endpoint" ) )
    endpoint = vm["endpoint"].as< string>();
  else {
    cerr << "Endpoint not specified in the command line arguments. Stop." << endl;
    return 1;
  }

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

  glite::ce::cream_cli::services::cli_service_enable_submission ES( opt );
  
  int res = ES.execute( );

  if(res) {
    ES.getLogger()->fatal( ES.get_execution_error_message( ) );
    return 1;
  }
  
  //if( ES.get_result( ) )
  cout << "Operation for enabling new submissions succeeded" << endl;
  
  return 0;

}

void printhelp() {
  printf("%s\n",help_messages::getEnableSubHelp().c_str());
}
