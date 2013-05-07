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
        glite-ce-service-info: a command line to obtain service's information

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-service-info.cpp,v 1.1.2.6.2.3.2.5 2012/09/14 09:10:30 adorigo Exp $
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
#include "services/cli_service_service_info.h"
#include "boost/algorithm/string.hpp"
#include <boost/program_options.hpp>

#define DEFAULT_CONF_FILE "./glite_cream.conf"
#define DEFAULT_LOG_DIR "/tmp/glite_cream_cli_logs"
#define DEFAULT_PROTO "https://"

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;
namespace po = boost::program_options;
Namespace namespaces[] = {};
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

//________________________________________________________________________
string time_t_to_string( time_t tval ) {
  char buf[26]; // ctime_r wants a buffer of at least 26 bytes
  ctime_r( &tval, buf );
  if(buf[strlen(buf)-1] == '\n')
    buf[strlen(buf)-1] = '\0';
  return string( buf );
}

void printhelp(void);

int main(int argc, char *argv[]) {

  bool    specified_proxyfile = false;
  bool    debug = false;
  bool    nomsg = false;
  bool    redir_out=false;
  string  certfile;
  string  endpoint;
  string  logfile;
  string  user_conf_file = "";
  int     timeout = 30;
  bool    verify_ac_sign = true;
  int     verbosity = 0;
  
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
	  "level,L", po::value<int>(&verbosity)->default_value( 0 )
	 )
	 ("endpoint", po::value<string>(), "Set the endpoint to ask the submission enable status")
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
    //    log_dev->setPriority( log4cpp::Priority::INFO );
  }
    
  if( vm.count("nomsg") ) {
    nomsg = true;
    //   log_dev->setPriority( log4cpp::Priority::ERROR );
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
  
  apiproxy::ServiceInfoWrapper serviceInfoWrapper;

  glite::ce::cream_cli::services::cli_service_service_info SI( opt, &serviceInfoWrapper );
  
  SI.set_verbosity( verbosity );
  
  if( SI.execute( ) ) {
    SI.getLogger()->fatal( SI.get_execution_error_message( ) );
    return 1;
  }
  printf("Interface Version  = [%s]\n", serviceInfoWrapper.interfaceVersion.c_str() );
   printf("Service Version    = [%s]\n", serviceInfoWrapper.serviceVersion.c_str() );
   if(serviceInfoWrapper.description)
     printf("Description        = [%s]\n", serviceInfoWrapper.description->c_str() );
   printf("Started at         = [%s]\n", time_t_to_string(serviceInfoWrapper.startupTime).c_str() );
   printf("Submission enabled = [%s]\n", serviceInfoWrapper.doesAcceptNewJobSubmissions ? "YES" : "NO" );
   if(serviceInfoWrapper.status)
     printf("Status             = [%s]\n", serviceInfoWrapper.status->c_str() );

   vector<CREAMTYPES__Property * >::const_iterator it=serviceInfoWrapper.property.begin();
   while( it!=serviceInfoWrapper.property.end() )
     {
       printf("Service Property   = [%s]->[%s]\n", (*it)->name.c_str(), (*it)->value.c_str());
       ++it;
     }

   vector<class CREAMTYPES__ServiceMessage * >::const_iterator sit=serviceInfoWrapper.message.begin();
   while( sit != serviceInfoWrapper.message.end() )
     {
       printf("Service Message    = [%s]->[%s] [%s]\n", (*sit)->type.c_str(), (*sit)->message.c_str(), time_t_to_string((*sit)->timastamp).c_str());
       ++sit;
     }
  
  return 0;

//   while(1) {
//     int c;
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"nomsg", 0, 0, 'n'},
//       {"proxyfile", 1, 0, 'p'},
//       {"logfile", 1, 0, 'l'},
//       {"conf", 1, 0, 'c'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {"level", 1, 0, 'L'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "l:hAdvp:nc:t:L:", long_options, &option_index);
//     if ( c == -1 )
//       break;
//     switch(c) {
//     case 'A':
//       verify_ac_sign = false;
//       break; 
//     case 'L':
//       verbosity = atoi( optarg );
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
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     default:
// 	cerr << "Type " << argv[0] << " -h for an help" << endl;
//       exit(1);
//     }
//   }
//   
//   if(nomsg && debug) {
//       log_dev->error( "Cannot specify both --debug and --nomsg options. Stop." );
//     exit(1);
//   }
// 
//   string VO = "";
//   try {
//     if(!specified_proxyfile)
//       certfile = cliUtils::getProxyCertificateFile();
//     else
//       certfile = proxyfile;
//     log_dev->info( "Using certificate proxy file [%s]", certfile.c_str() );
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
// 
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
//    if(debug)
//       log_dev->warn( string(ex.what())+". Using built-in configuration" );
//   }
// 
//   if(debug || redir_out) {
//     if(!redir_out)
//       logfile = logger_instance->getLogFileName(confMgr.getProperty("GETSERVICEINFO_LOG_DIR",
// 								    DEFAULT_LOG_DIR).c_str(), 
// 						"glite-ce-service-info");
//     log_dev->info( "Logfile is [%s]", logfile.c_str() );
//     cliUtils::mkdir(cliUtils::getPath(logfile));
//     logger_instance->setLogFile(logfile);
//   }
// 
//   if(!specified_proxyfile)
//     proxyfile = cliUtils::getProxyCertificateFile();
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
//   if(!cliUtils::containsTCPPort( endpoint ) ) {
//     endpoint += ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//   }
//   
//   string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)
//     + endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);
// 
//   string addrlog = string("Service address=[") + serviceAddress + "]";
//   log_dev->info( addrlog );

//    ServiceInfoWrapper serviceInfoWrapper;
//    
//    AbsCreamProxy* creamClient;
//    creamClient = CreamProxyFactory::make_CreamProxyServiceInfo(&serviceInfoWrapper, verbosity, timeout );
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
//      creamClient->execute(serviceAddress);
// 
//    } catch (BaseException& ex) {
// 
//      log_dev->error(ex.what());
//      return 1;
//    } catch (exception& ex) {
// 
//      log_dev->error(ex.what());
//      return 1;
//    }
// 
//    //printf("%s\n", serviceInfoWrapper.getCEMonURL().c_str());
// 
//    printf("Interface Version  = [%s]\n", serviceInfoWrapper.interfaceVersion.c_str() );
//    printf("Service Version    = [%s]\n", serviceInfoWrapper.serviceVersion.c_str() );
//    if(serviceInfoWrapper.description)
//      printf("Description        = [%s]\n", serviceInfoWrapper.description->c_str() );
//    printf("Started at         = [%s]\n", time_t_to_string(serviceInfoWrapper.startupTime).c_str() );
//    printf("Submission enabled = [%s]\n", serviceInfoWrapper.doesAcceptNewJobSubmissions ? "YES" : "NO" );
//    if(serviceInfoWrapper.status)
//      printf("Status             = [%s]\n", serviceInfoWrapper.status->c_str() );
// 
//    vector<CREAMTYPES__Property * >::const_iterator it=serviceInfoWrapper.property.begin();
//    while( it!=serviceInfoWrapper.property.end() )
//      {
//        printf("Service Property   = [%s]->[%s]\n", (*it)->name.c_str(), (*it)->value.c_str());
//        ++it;
//      }
// 
//    vector<class CREAMTYPES__ServiceMessage * >::const_iterator sit=serviceInfoWrapper.message.begin();
//    while( sit != serviceInfoWrapper.message.end() )
//      {
//        printf("Service Message    = [%s]->[%s] [%s]\n", (*sit)->type.c_str(), (*sit)->message.c_str(), time_t_to_string((*sit)->timastamp).c_str());
//        ++sit;
//      }
}

void printhelp(void) {
  cout << help_messages::ServiceInfo_help << endl;
}
