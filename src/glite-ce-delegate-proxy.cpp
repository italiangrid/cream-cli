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
        glite-ce-delegate-proxy: a command line to delegate a user's proxy to the CE

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-delegate-proxy.cpp,v 1.67.4.1.4.4.2.1.2.3.2.3 2012/01/11 15:57:05 adorigo Exp $
*/

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "hmessages.h"
#include "util/cliUtils.h"
#include "services/cli_service_delegate_proxy.h"

#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include "boost/algorithm/string.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp(void);
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  bool specified_endpoint = false;
  bool debug = false;
  bool nomsg = false;
  bool redir_out=false;
  string DID;
  int option_index;
  string proxyfile;
  string certfile;
  bool specified_proxyfile = false;
  string endpoint;
  string logfile;
  string    user_conf_file = "";
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
    ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint to ask the submission enable status")
    ("delegation-id,D", po::value<string>(&DID), "")
    ;
  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      cerr << "FATAL: " << ex.what() << endl;
      return 1;
  } 
  po::positional_options_description p;
  p.add("delegation-id", -1);
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
  
  if( !vm.count("delegation-id" ) ) {
    cerr << "delegation ID argument is mandatory. Stop." << endl; 
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

  glite::ce::cream_cli::services::cli_service_delegate_proxy DP( opt );
  
  DP.set_delegation_id( DID );
  
  if(DP.execute( ) ) {
    DP.getLogger()->fatal( DP.get_execution_error_message( ) );
    return 1;
  }
  
  cout << "Proxy with delegation id [" << DID << "]"
       << " succesfully delegated to endpoint [" << endpoint << "]" << endl;
  
  return 0;
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
//   while(1) {
//     int c;
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"nomsg", 0, 0, 'n'},
//       {"proxyfile", 1, 0, 'p'},
//       {"endpoint", 1, 0, 'e'},
//       {"logfile", 1, 0, 'l'},
//       {"conf", 1, 0, 'c'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "l:hdvAp:e:nc:t:", long_options, &option_index);
//     if ( c == -1 )
//       break;
//     switch(c) {
//     case 'A':
//       verify_ac_sign = false;
//       break;      
//     case 'd':
//       debug=true;
//       log_dev->setPriority( log4cpp::Priority::DEBUG );
//       break;
//     case 'h':
//       printhelp();
//       return(0);
//       break;
//     case 'n':
//       nomsg=true;
//       log_dev->setPriority( log4cpp::Priority::ERROR );
//       break;
//     case 'v':
//       help_messages::printver();
//       return(0);
//       break;
//     case 'l':
//       redir_out = true;
//       logfile = string(optarg);
//       break;
//     case 'p':
//       proxyfile = string(optarg);
//       specified_proxyfile = true;
//       break;
//     case 'e':
//       endpoint = string(optarg);
//       specified_endpoint = true;
//       break;
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     case 't':
//       timeout = atoi(optarg);
//       break;
//     default:
// 	cerr << "Type " << argv[0] << " -h for an help" << endl;
//       return(1);
//     }
//   }
//   
//   if(nomsg && debug) {
//     // ERROR
//       log_dev->error( "Cannot specify both --debug and --nomsg options. Stop." );
//     return(1);
//   }

//   string VO = "";
//     
//   if(!specified_proxyfile) {
//     try {
//       certfile = cliUtils::getProxyCertificateFile();
//     } catch(exception& ex) {
//       logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
//       exit(1);
//     }
//   } else
//     certfile = proxyfile;
//   
//   log_dev->info( "Using certificate proxy file [%s]", certfile.c_str() );

//     long int leftcert = certUtil::getProxyTimeLeft(certfile);

//     if(leftcert <= 0) {
//       log_dev->fatal( "Proxy certificate is expired. Please Check it." );
//       exit(1);
//     }

//     VO = certUtil::getVOName( certfile );
    
//     VOMSWrapper V( certfile, verify_ac_sign  );
//     if( !V.IsValid( ) ) {
//       
// 	logger_instance->log(log4cpp::Priority::FATAL, 
// 			     string("Problems with proxyfile [")+certfile+"]: "+
// 			     V.getErrorMessage(), true, true, true);
// 	exit(1);
//       
//     }
//       } else
//       logger_instance->log(log4cpp::Priority::ERROR, 
// 			   "Error while reading proxyfile ["+certfile+"]: "+
// 			   V.getErrorMessage(), true, true, true);
// 	exit(1);
//     }

//     long int leftcert = V.getProxyTimeEnd( ) - time( NULL );

//     if(leftcert <= 0) {
//       log_dev->fatal( "Proxy certificate is expired. Please Check it." );
//       exit(1);
//     }

//    VO = V.getVOName( );
  
  /**
   * Handle the load of configurations
   */

//   vector<string> confiles;
//   try {
//     confiles = cliUtils::getConfigurationFiles(VO, 
// 					       user_conf_file, 
// 					       DEFAULT_CONF_FILE);
//   } catch(no_user_confile_ex& ex) {
//       log_dev->error( ex.what() );
//     return(1);
//   }
// 
//   if ( log_dev->isInfoEnabled() ) {
//     for(vector<string>::iterator it=confiles.begin(); it!=confiles.end(); it++)
//       log_dev->info( "Loading configurations from [%s]...", (*it).c_str() );
//   }
// 
//   ConfigurationManager confMgr(ConfigurationManager::classad);
//   try { confMgr.load(confiles); }
//   catch(file_ex& ex) {
//     if(debug)
//       log_dev->warn( string(ex.what())+". Using built-in configuration" );
//   }

//   if(debug || redir_out) {
//     if(!redir_out)
//       logfile = logger_instance->getLogFileName(confMgr.getProperty("DELEGATE_LOG_DIR", 
// 								    DEFAULT_LOG_DIR).c_str(), 
// 						"glite-ce-delegate-proxy");
//     log_dev->info( "Logfile is [%s]", logfile.c_str() );
//     cliUtils::mkdir(cliUtils::getPath(logfile));
//     logger_instance->setLogFile(logfile);
//   }
// 
// 
// 
//   if(!specified_proxyfile)
//     proxyfile = cliUtils::getProxyCertificateFile();
//   
//   if(!specified_endpoint) {
//       log_dev->error( "endpoint not specified in the command line arguments. Stop" );
//     return(1);
//   }
// 
// 
//   if( !cliUtils::checkEndpointFormat( endpoint ) )
//   {
//     log_dev->error( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
//     return(1);
//   }
// 
//   if(argv[optind]==NULL) {
//       log_dev->error( "delegationID not specified in the command line arguments. Stop." );
//     return(1);
//   } else DID = string(argv[optind]);
// 
//   try {
//     
//     if( !cliUtils::containsTCPPort(endpoint) ) {
//       endpoint += ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//     }
//    
//     string service =
//       confMgr.getProperty("CREAMDELEGATION_URL_PREFIX", DEFAULT_PROTO)
//       + endpoint+"/"
//       + confMgr.getProperty("CREAMDELEGATION_URL_POSTFIX", DEFAULT_CEURLDELEGATION_POSTFIX);
//     log_dev->info( "Delegating proxy on service [%s] with ID [%s]",
// 		   service.c_str(), DID.c_str() );
		   
//    string certtxt;

//     AbsCreamProxy* creamClient = CreamProxyFactory::make_CreamProxyDelegate(DID, timeout );
//     if(!creamClient)
//       {
// 	log_dev->fatal( "CreamProxy creation FAILED!! Stop!");
// 	return(1);
//       }
// 
//     boost::scoped_ptr< AbsCreamProxy > tmpRegisterClient;
//     tmpRegisterClient.reset( creamClient );

 //    creamClient->setCredential( certfile );	
//     creamClient->execute( service );

    //creamClient->garbage_collect();
    
    //log_dev->notice( "Proxy with delegation id [%s] succesfully delegated to endpoint [%s]",
//		     DID.c_str(), service.c_str() );
		     
//   } catch(BaseException& ex) {
//     log_dev->error( ex.what() );
//     return 1;
//   } catch(exception& ex) {
//     log_dev->error( ex.what() );
//     return 1;
//   }
  
  //delete initSoap;
}

void printhelp(void) {
  cout << help_messages::getDelegateHelp() << endl;
}
