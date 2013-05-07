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
#include <iostream>
#include <istream>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "util/cliUtils.h"
#include "services/cli_service_joblease.h"
#include "hmessages.h"
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
//#include "boost/lexical_cast.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp() ;
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  time_t now = time(0);
 
  string endpoint;
  bool debug = false;
  string logfile;
  bool redir_out = false;
  bool nomsg=false;
  string outfile;

  string user_conf_file = "";
  string proxyfile, certfile;
  bool specified_proxyfile = false;
  string leaseId;
  time_t leaseTime = 3600;

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
    ("leaseTime,T", po::value<time_t>(&leaseTime), "")
    ("leaseId,D", po::value<string>(&leaseId), "")
    ;
    } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      cerr << "FATAL: " << ex.what() << endl;
      return 1;
    }

    po::positional_options_description p;
    p.add("leaseId", -1);
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
  
  if( !vm.count("leaseId" ) ) {
    cerr << "Lease ID argument is mandatory. Stop." << endl; 
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

  glite::ce::cream_cli::services::cli_service_joblease JL( opt );

  JL.set_lease( leaseId, leaseTime );

  if( JL.execute( ) ) {
    JL.getLogger()->fatal( JL.get_execution_error_message( ) );
    return 1;
  }

  cout << "You requested lease time [" << leaseTime << "] for lease ID [" << leaseId << "]" << endl;

  cout << "CREAM negotiated the lease time to [" << ( JL.get_negotiated_lease_time( ) - now +1 ) << "]" << endl; 

  return 0;










//   while( 1 ) {
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"nomsg", 0, 0, 'n'},
//       {"logfile", 1, 0, 'l'},
//       {"conf", 1, 0, 'c'},
//       {"proxyfile", 1, 0, 'p'},
//       {"leaseTime", 1, 0, 'T'},
//       {"endpoint", 1, 0, 'e'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "nhdAvr:l:c:p:T:e:t:", long_options, &option_index);
//     if ( c == -1 )
//       break;

//     switch(c) {
//     case 'A':
//       verify_ac_sign = false;
//       break;            
//     case 'p':
//       proxyfile = string(optarg);
//       specified_proxyfile = true;
//       break;
//     case 'T':
//       leaseTime = atoi(optarg);
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
//       log_dev->setPriority( log4cpp::Priority::DEBUG );
//       break;
//     case 'l':
//       redir_out = true;
//       logfile = string(optarg);
//       break;
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     case 'e':
//       endpoint = optarg;
//       break;
//     case 't':
//       timeout = atoi(optarg);
//       break;
//     default:
//         cerr << "Type " << argv[0] << " -h for help" << endl;
//         exit(1);
//     }
//   }
//   if(debug && nomsg) {
//       log_dev->fatal("Cannot specify both --debug and --nomsg options. Stop.");
//       exit(1);
//   }

//   if(argv[optind]==NULL) {
//     log_dev->fatal( "LeaseID not specified in the command line arguments. Stop.");
//     exit(1);
//   } else {

//     leaseId = string(argv[optind]);
    
//   }

//   string VO = "";

//   if(!specified_proxyfile) {
//     try {
//       certfile = cliUtils::getProxyCertificateFile();
//     } catch(exception& ex) {
//       logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
//       exit(1);
//     }
//   } else
//     certfile = proxyfile;

//   if ( log_dev->isInfoEnabled() ) {
//     log_dev->info("Using certificate proxy file [%s]", certfile.c_str() );
//   }

//    long int leftcert = certUtil::getProxyTimeLeft(certfile);

//    if(leftcert <= 0) {
//       log_dev->fatal( "Proxy certificate is expired. Please Check it." );
//       exit(1);
//    }

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
   * Handles the configuration loading
   */
//   vector<string> confiles;
//   try {
//     confiles = cliUtils::getConfigurationFiles(VO,
// 					       user_conf_file,
// 					       DEFAULT_CONF_FILE);
//   } catch(no_user_confile_ex& ex) {
//       log_dev->fatal( ex.what() );
//     exit(1);
//   }
// 
//   if(log_dev->isInfoEnabled()) {
//       for(vector<string>::const_iterator fit = confiles.begin();
//           fit != confiles.end();
// 	  fit++) { //unsigned int j=0; j<confiles.size(); j++) {
//           log_dev->info("Loading configurations from [%s]", (*fit).c_str() );
//       }
//       log_dev->info("**************************************************************************");
//       log_dev->info( help_messages::getStartMessage() );
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
//           logfile = logger_instance->getLogFileName(confMgr.getProperty("ALLOWEDSUB_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-job-lease");
//       }
//       log_dev->info( "Logfile is [%s]", logfile.c_str() );
//       cliUtils::mkdir(cliUtils::getPath(logfile));
//       logger_instance->setLogFile(logfile.c_str());
//   }


//   if(argv[optind]==NULL) {
//       log_dev->error( "Endpoint not specified in the command line arguments. Stop.");
//       exit(1);
//   } else {
//     jobToLease = string(argv[optind]);
//   }

  //vector<string> pieces;

  //  CEUrl::parseJobID( jobToLease, pieces, confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443"));
  //string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
  //  pieces[1]+":"+pieces[2]+"/"+
  //  confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);


 //  string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)
//     + endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);

//   string addrlog = string("Service address=[") + serviceAddress + "]";
//   log_dev->info( addrlog );
//   
//   pair<string, time_t> negotiated_lease;
// 
//   AbsCreamProxy* creamClient;

//   creamClient = CreamProxyFactory::make_CreamProxyLease(make_pair(leaseId, time(NULL)+leaseTime), &negotiated_lease, timeout );
//   if(!creamClient)
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
//       return 1;
//     }
//   boost::scoped_ptr< AbsCreamProxy > tmpClient;
//   tmpClient.reset( creamClient );

//   creamClient->setCredential( certfile );
//   
//   try {
// 
//     //creamClient->setRequestParameters(make_pair(leaseId, leaseTime), &negotiated_lease);
//     creamClient->execute(serviceAddress);
//     //creamClient->garbage_collect();
// 
//   } catch (exception& ex) {
// 
//     log_dev->fatal(ex.what());
//     return 1;
//   }
// 
//   //  ostringstream log1, log2;
//   string log1, log2;
//   log1 = string("You requested lease time [") + boost::lexical_cast<string>(leaseTime) 
//     + "] for lease ID [" + leaseId + "]";
//   //  log1 << "You requested lease time [" << leaseTime << "] for lease ID [" << leaseId << "]";
//   log2 = string("CREAM negotiated the lease time to [") + boost::lexical_cast<string>(negotiated_lease.second) 
//     + "]";
//   //  log2 << "CREAM negotiated the lease time to [" << negotiated_lease.second << "]";
// 
//   log_dev->info( log1 );
//   log_dev->info( log2 );

  return 0;
}

void printhelp() {
  printf("%s\n",help_messages::getJobLeaseHelp().c_str());
}
