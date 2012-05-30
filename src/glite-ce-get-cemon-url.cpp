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
        glite-ce-get-cemon-url: a command line to obtain the CEMon's URL of a CREAM

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-get-cemon-url.cpp,v 1.11.4.1.4.4.2.2.2.3.2.2 2012/01/11 15:57:05 adorigo Exp $
*/

/**
 *
 * System Headers
 *
 */
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/ServiceInfoWrapper.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
/**
 *
 * CLI Headers
 *
 */
#include "hmessages.h"
#include "util/cliUtils.h"
#include "services/cli_service_get_cemon_url.h"
#include "boost/algorithm/string.hpp"
#include <boost/program_options.hpp>

#define DEFAULT_CONF_FILE "./glite_cream.conf"
#define DEFAULT_LOG_DIR "/tmp/glite_cream_cli_logs"
#define DEFAULT_CEURL_POSTFIX "ce-cream/services/CREAM2"
#define DEFAULT_PROTO "https://"

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

  bool specified_proxyfile = false;
  bool debug = false;
  bool nomsg = false;
  bool redir_out=false;
  int option_index;
  string proxyfile;
  string certfile;
  string endpoint;
  string logfile;
  bool WRITE_LOG_ON_FILE = true;
  string    user_conf_file = "";
  creamApiLogger* logger_instance = creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();
  
  // Initialize the logger
  log_dev->setPriority( log4cpp::Priority::NOTICE );
  logger_instance->setLogfileEnabled( WRITE_LOG_ON_FILE );

  int timeout = 30;

  bool verify_ac_sign = true;

  po::options_description desc("Usage");
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
	 "noint,N",
	 "Assume yes to any question"
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

  glite::ce::cream_cli::services::cli_service_get_cemon_url GU( opt );
  
  int res = GU.execute( );

  if(res) {
    GU.getLogger()->error( GU.get_execution_error_message( ) );
    return 1;
  }
  
  //if( GU.get_result( ) )
  //printf("Operation for enabling new submissions succeeded\n");
  
  cout << GU.get_url( ) << endl;
  
  return 0;


//   while(1) {
//     int c;
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"nomsg", 0, 0, 'n'},
//       {"proxyfile", 1, 0, 'p'},
//       //{"endpoint", 1, 0, 'e'},
//       {"logfile", 1, 0, 'l'},
//       {"conf", 1, 0, 'c'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "l:hAdvp:nc:t:", long_options, &option_index);
//     if ( c == -1 )
//       break;
//     switch(c) {
//     case 'A':
//       verify_ac_sign = false;
//       break; 
//     case 'd':
//       debug=true;
//       log_dev->setPriority( log4cpp::Priority::INFO );
//       break;
//     case 'h':
//       printhelp();
//       exit(0);
//       break;
//     case 'n':
//       nomsg=true;
//       log_dev->setPriority( log4cpp::Priority::ERROR );
//       break;
//     case 'v':
//       help_messages::printver();
//       exit(0);
//       break;
//     case 'l':
//       redir_out = true;
//       logfile = string(optarg);
//       break;
//     case 'p':
//       proxyfile = string(optarg);
//       specified_proxyfile = true;
//       break;
//     case 't':
//       timeout = atoi(optarg);
//       break;
// //     case 'e':
// //       endpoint = string(optarg);
// //       specified_endpoint = true;
// //       break;
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     default:
// //       string s = string("Unrecognized option ");
// //       s = s + string_manipulation::make_string((char)c) + "\n" + "Type " + argv[0] + " -h for an help";
// //       printf("%s\n", s.c_str());
// 	cerr << "Type " << argv[0] << " -h for an help" << endl;
//       exit(1);
//     }
//   }
//   
//   if(nomsg && debug) {
//     // ERROR
//       log_dev->error( "Cannot specify both --debug and --nomsg options. Stop." );
//     exit(1);
//   }
// 
// //  CreamProxy *initSoap;
//   string VO = "";
//   try {
//   //  initSoap = new CreamProxy(false);
//     if(!specified_proxyfile)
//       certfile = cliUtils::getProxyCertificateFile();
//     else
//       certfile = proxyfile;
//     log_dev->info( "Using certificate proxy file [%s]", certfile.c_str() );
//     //VO = initSoap->Authenticate(certfile);
// 
//     VOMSWrapper V( certfile, verify_ac_sign  );
//     if( !V.IsValid( ) ) {
//      
// 	logger_instance->log(log4cpp::Priority::FATAL, 
// 			     string("Problems with proxyfile [")+certfile+"]: "+
// 			     V.getErrorMessage(), true, true, true);
// 	exit(1);
//       
//     }
// //       } else
// //       logger_instance->log(log4cpp::Priority::ERROR, 
// // 			   "Error while reading proxyfile ["+certfile+"]: "+
// // 			   V.getErrorMessage(), true, true, true);
// // 	exit(1);
// //     }
//     
// //     long int leftcert = V.getProxyTimeEnd( ) - time( NULL );
//     
// //     if(leftcert <= 0) {
// //       log_dev->fatal( "Proxy certificate is expired. Please Check it." );
// //       exit(1);
// //     }
//     
//     VO = V.getVOName( );
// 
//   } catch(soap_ex& ex) {
//       log_dev->error( ex.what() );
//     exit(1);
//   } catch(auth_ex& auth) {
//     log_dev->error( auth.what() );
//     exit(1);
//   } catch(exception& ex) {
//     log_dev->error( ex.what() );
//     exit(1);
//   }
// 
//   /**
//    * Handle the load of configurations
//    */
// 
//   vector<string> confiles;
//   //  confiles.reserve(10);
//   try {
//     confiles = cliUtils::getConfigurationFiles(VO, 
// 					       user_conf_file, 
// 					       DEFAULT_CONF_FILE);
//   } catch(no_user_confile_ex& ex) {
//       log_dev->error( ex.what() );
//     exit(1);
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
// 
//   if(debug || redir_out) {
//     if(!redir_out)
//       logfile = logger_instance->getLogFileName(confMgr.getProperty("GETCEMONURL_LOG_DIR",
// 								    DEFAULT_LOG_DIR).c_str(), 
// 						"glite-ce-get-cemon-url");
//     log_dev->info( "Logfile is [%s]", logfile.c_str() );
//     cliUtils::mkdir(cliUtils::getPath(logfile));
//     logger_instance->setLogFile(logfile);
//   }
// 
//   if(!specified_proxyfile)
//     proxyfile = cliUtils::getProxyCertificateFile();
//   
// //   if(!specified_endpoint) {
// //       log_dev->error( "endpoint not specified in the command line arguments. Stop" );
// //     exit(1);
// //   }
//   
//   if(argv[optind]==NULL) {
//       log_dev->error( "endpoint not specified in the command line arguments. Stop." );
//     exit(1);
//   } else endpoint = string(argv[optind]);
// 
//   if( !cliUtils::checkEndpointFormat( endpoint ) )
//   {
//     log_dev->error( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
//     exit(1);
//   }
// 
//   /**
//    * check that has been passed an endpoint with format host:port
//    */
//   //if(string_manipulation::matches(endpoint, "^\\w*://"))
// //   boost::regex tmp( "^\\w*://" );
// //   boost::smatch what;
// //   if( boost::regex_match( endpoint, what, tmp ) )
// //     {
// //       log_dev->error( "endpoint must NOT start with any <protocol>:// . Stop" );
// //       exit(1);
// //     }
// 
//   try {
// 
//     //boost::regex tmp2( "^\\w+(:\\d+)" );pattern = "^[^:]+(:[0-9]{1,5})?\\b";
//     //    tmp = "^[^:]+(:[0-9]{1,5})?\\b";
//     //boost::smatch what2;
// //     if( !boost::regex_match( endpoint, what, tmp ) ) {
// //       log_dev->error("Specified endpoint has wrong format. Must be <host>[:<tcp_port>] (tcp_port is a number of 5 digit max length). Stop" );
// //       exit(1);
// //     }
// //   tmp = "^[^:]+:[0-9]{1,5}\\b";
// //   if( !boost::regex_match(endpoint, what, tmp) ) {
//     if(!cliUtils::containsTCPPort( endpoint ) ) {
//       endpoint += ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//     }
// 
//   string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)
//     + endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);
// 
//   string addrlog = string("Service address=[") + serviceAddress + "]";
//   log_dev->info( addrlog );
// 
//    ServiceInfoWrapper serviceInfoWrapper;
//    
//    AbsCreamProxy* creamClient;
//    creamClient = CreamProxyFactory::make_CreamProxyServiceInfo(&serviceInfoWrapper, 2, timeout );
//    if(!creamClient)
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
//       return 1;
//     }
//    boost::scoped_ptr< AbsCreamProxy > tmpClient;
//    tmpClient.reset( creamClient );
// 
//    creamClient->setCredential( certfile );
// 
//    try {
// 
//      //creamClient->setRequestParameters(make_pair(leaseId, leaseTime), &negotiated_lease);
//      creamClient->execute(serviceAddress);
//      //creamClient->garbage_collect();
// 
//    } catch (exception& ex) {
// 
//      log_dev->error(ex.what());
//      return 1;
//    }
// 
// 
// //   initSoap->GetCEMonURL(service.c_str(), cemonurl);
//    
//     printf("%s\n", serviceInfoWrapper.getCEMonURL().c_str());
//   } catch(exception& ex) {
//     log_dev->error( ex.what() );
//     exit(1);
//   } /*catch(exception& ex) {
//     log_dev->error( ex.what() );
//     exit(1);
//   }*/
}

void printhelp(void) {
  cout << help_messages::getCEMonURLHelp() << endl;
}
