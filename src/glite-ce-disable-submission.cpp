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
        glite-ce-disable-submission: a command line interface to set to OFF the submission on a CREAM service

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-disable-submission.cpp,v 1.9.4.1.4.4.2.1.2.3.2.3 2012/09/14 09:10:29 adorigo Exp $
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
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

/**
 *
 * CLI Headers
 *
 */
#include "services/cli_service_disable_submission.h"
#include "util/cliUtils.h"
#include "hmessages.h"

/**
 *
 * BOOST Headers
 *
 */
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

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
//  string outfile;

  bool noint = false;
  string user_conf_file = "";
  creamApiLogger* logger_instance = creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();
  string certfile;
//  bool specified_proxyfile = false;

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
	 ("endpoint", po::value<string>(), "Set the endpoint where disable submission")
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
  
  if( vm.count("noint") )
    noint = true;
    
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
  opt.m_noint = noint;
  opt.m_verify_ac_sign = verify_ac_sign;
  opt.m_redir_out = redir_out;
  opt.m_user_conf_file = user_conf_file;
  opt.m_certfile = certfile;
  opt.m_logfile = logfile;
  opt.m_endpoint = endpoint;
  opt.m_soap_timeout = timeout;  

  glite::ce::cream_cli::services::cli_service_disable_submission DS( opt );
  
  if( DS.execute( ) ) {
    DS.getLogger()->fatal( DS.get_execution_error_message() );
    return 1;
  }
  
  cout << "Operation for disabling new submissions succeeded" << endl;
  
  return 0;
  
  //*******************************************

//   while( 1 ) {
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"nomsg", 0, 0, 'n'},
//       {"logfile", 1, 0, 'l'},
//       {"noint", 0, 0, 'N'},
//       {"conf", 1, 0, 'c'},
//       {"proxyfile", 1, 0, 'p'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "nhdAvr:l:c:p:t:", long_options, &option_index);
//     if ( c == -1 )
//       break;
// 
//     switch(c) {
//     case 'A':
//       verify_ac_sign = false;
//       break;   
//     case 'p':
//       proxyfile = string(optarg);
//       specified_proxyfile = true;
//       break;
//     case 'N':
//       noint = true;
//       break;
//     case 'v':
//       help_messages::printver();
//       exit(0);
//       break;
//     case 'n':
//         nomsg=true; // FIXME: remove??
//         log_dev->setPriority( log4cpp::Priority::ERROR );
//         break;
//     case 'h':
//       printhelp();
//       exit(0);
//       break;
//     case 'd':
//       debug=true;
//       log_dev->setPriority( log4cpp::Priority::INFO );
//       break;
//     case 'l':
//       redir_out = true;
//       logfile = string(optarg);
//       break;
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     case 't':
//       timeout = atoi( optarg );
//       break;
//     default:
//         cerr << "Type " << argv[0] << " -h for help" << endl;
//         exit(1);
//     }
//   }
//   if(debug && nomsg) {
//       log_dev->error("Cannot specify both --debug and --nomsg options. Stop.");
//       exit(1);
//   }
// 
//   //CreamProxy *initSoap;
//   string VO = "";
//   try {
//     //initSoap = new CreamProxy(false);
//     if(!specified_proxyfile)
//       certfile = cliUtils::getProxyCertificateFile();
//     else
//       certfile = proxyfile;
//     if ( log_dev->isInfoEnabled() ) {
//         log_dev->info("Using certificate proxy file [%s]", certfile.c_str() );
//     }
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
//  //      } else
// //       logger_instance->log(log4cpp::Priority::ERROR, 
// // 			   "Error while reading proxyfile ["+certfile+"]: "+
// // 			   V.getErrorMessage(), true, true, true);
// 
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
//     //VO = initSoap->Authenticate(certfile);
//     VO = V.getVOName();
// 
//   } catch(soap_ex& s) {
//       log_dev->error( s.what() );
//       exit(1);
//   } catch(auth_ex& auth) {
//       log_dev->error( auth.what() );
//       exit(1);
//   }
// 
//   /**
//    * Handles the configuration loading
//    */
//   vector<string> confiles;
//   try {
//     confiles = cliUtils::getConfigurationFiles(VO,
// 					       user_conf_file,
// 					       DEFAULT_CONF_FILE);
//   } catch(no_user_confile_ex& ex) {
//       log_dev->error( ex.what() );
//     exit(1);
//   }
//   
//   if(log_dev->isInfoEnabled()) {
//     for(vector<string>::const_iterator fit = confiles.begin();
// 	fit != confiles.end();
// 	fit++) { //unsigned int j=0; j<confiles.size(); j++) {
//       log_dev->info("Loading configurations from [%s]", (*fit).c_str() );
//     }
//     log_dev->info("**************************************************************************");
//     log_dev->info( help_messages::getStartMessage() );
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
//       if(!redir_out) {
//           logfile = logger_instance->getLogFileName(confMgr.getProperty("DISABLE_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-disable-submission");
//       }
//       log_dev->info( "Logfile is [%s]", logfile.c_str() );
//       cliUtils::mkdir(cliUtils::getPath(logfile));
//       logger_instance->setLogFile(logfile.c_str());
//   }
//   
// 
//   if(argv[optind]==NULL) {
//       log_dev->error( "Endpoint not specified in the command line arguments. Stop.");
//       exit(1);
//   } else {
//     endpoint = string(argv[optind]);
//   }
// 
//   if( !cliUtils::checkEndpointFormat( endpoint ) )
//   {
//     log_dev->error( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
//     exit(1);
//   }
// 
//   /**
//    *
//    */
// 
//   //boost::regex tmp2( "^\\w+(:\\d+)" );
//   //boost::smatch what2;
//   //if( !boost::regex_match( endpoint, what2, tmp2 ) ) {
//   if( !cliUtils::containsTCPPort( endpoint ) ) {
//     //if( what2[1] == "" )
//     endpoint = endpoint + ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//   }
// 
//   string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+ endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);//CEUrl::getCreamURI(URL);
// 
//   if (log_dev->isInfoEnabled()) {
//       log_dev->info( "Service address=[%s]", serviceAddress.c_str() );
//   }
// 
//   AbsCreamProxy *creamClient = CreamProxyFactory::make_CreamProxyAcceptNewJobSubmissions( false, timeout );
//   
//   if(!creamClient) 
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, 
// 			   string("FAILED TO CREATE AN AbsCreamProxy object!! STOP!!"), 
// 			   true, 
// 			   true, 
// 			   true);
//       return 1;
//     }
//   
//   boost::scoped_ptr< AbsCreamProxy > tmpRegisterClient;
//   tmpRegisterClient.reset( creamClient );
//   
//   creamClient->setCredential( certfile );
//   
// 
//   try {
//     creamClient->execute( serviceAddress );
//     //initSoap->DisableAcceptJobSubmissions( serviceAddress.c_str() );
//   } catch(exception& ex) {
//       log_dev->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
//       exit(1);
//   } /*catch(InternalException& ex) {
//     log_dev->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
//     exit(1);
//   }*/
// 
//   printf("Operation for disabling new submissions succeeded\n");
// 
//   //delete(initSoap);
//   return 0;
}

void printhelp() {
  printf("%s\n",help_messages::getDisableSubHelp().c_str());
}
