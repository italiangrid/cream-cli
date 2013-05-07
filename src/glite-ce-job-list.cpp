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
        glite-ce-job-list: a command line interface to get the list of jobs held by the CREAM service

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-job-list.cpp,v 1.61.4.1.4.4.2.1.2.3.2.4 2012/09/14 09:10:29 adorigo Exp $
*/

/**
 *
 * System Headers
 *
 */
#include <sys/types.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <getopt.h>
#include <istream>
#include <fstream>
#include <stdio.h>


/**
 *
 * Cream Client API Headers
 *
 */
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

/**
 *
 * CLI Headers
 *
 */
#include "util/cliUtils.h"
#include "hmessages.h"
#include "services/cli_service_joblist.h"

/**
 *
 * BOOST Headers
 *
 */
#include <boost/program_options.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

using namespace std;
namespace po = boost::program_options;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::cream_exceptions;

Namespace namespaces[] = {};

void printhelp() ;
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  string 		endpoint;
  bool 			debug = false;
  string 		logfile;
  bool 			redir_out = false;
  bool 			nomsg=false;
  string 		outfile;
  bool 			jobid_on_file = false;
  bool 			noint = false;
//  bool 			WRITE_LOG_ON_FILE = true;
  string 		user_conf_file = "";
  //  creamApiLogger* 	logger_instance = creamApiLogger::instance();
  //  log4cpp::Category* 	log_dev = logger_instance->getLogger();
  string 		certfile;
  int 			soap_timeout = 30;
  bool 			verify_ac_sign = true;

  // Initializes the logger
//   log_dev->setPriority( log4cpp::Priority::NOTICE );
//   logger_instance->setLogfileEnabled( WRITE_LOG_ON_FILE );

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
	 "output,o",
	 po::value<string>(&outfile)->default_value(""),
	 "Activate writing the job IDs on a file instead of stdout"
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
	 po::value<int>(&soap_timeout)->default_value(30),
	 "Set the soap connection timeout in seconds"
	)
        (
         "donot-verify-ac-sign,A",
         "Disable AC's signature verification"
         )
	 ("endpoint", po::value<string>(&endpoint), "Set the endpoint to ask the Job IDs to")
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

 
  if( vm.count("debug") )
    debug = true;
    
  if( vm.count("nomsg") )
    nomsg = true;
    
  if( vm.count("noint") )
    noint = true;
    
  if( vm.count("donot-verify-ac-sign") )
    verify_ac_sign = false;
    
  if ( vm.count("help") ) {
    printhelp();
    exit(0);
  }
  
  if ( vm.count("version") ) {
    help_messages::printver();
    exit(0);
  }

  //cout << "****** LOGFILE=[" << logfile << "]" << endl;

  if( !logfile.empty( ) )
      redir_out = true;
      
  if( !outfile.empty() )
    jobid_on_file = true;

  if( !vm.count("endpoint") )
  {
    cerr << "endpoint argument is mandatory. Stop." << endl;
    return 1;
  }

//  if( vm.count("endpoint" ) )
//    endpoint = vm["endpoint"].as< string>();

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
  opt.m_soap_timeout = soap_timeout;  

  glite::ce::cream_cli::services::cli_service_joblist JL( opt );
  
  if(jobid_on_file)
    JL.set_output( outfile ); 
  
  if(JL.execute( )) {
    JL.getLogger()->fatal( JL.get_execution_error_message( ) );
    return 1;
  }
  
  vector<string> jobs;
  JL.get_job_ids( jobs );
  
  for( vector<string>::iterator it = jobs.begin(); it != jobs.end(); ++it )
    cout << *it << endl;
  
  return 0;

//   if(debug && nomsg) {
//       log_dev->error("Cannot specify both --debug and --nomsg options. Stop.");
//       exit(1);
//   }
// 
//   if(debug)
//     log_dev->setPriority( log4cpp::Priority::INFO );
//   if(nomsg)
//     log_dev->setPriority( log4cpp::Priority::ERROR );
// 
//   string VO = "";
//   try {
// 
//     if ( log_dev->isInfoEnabled() ) {
//         log_dev->info("Using certificate proxy file [%s]", certfile.c_str() );
//     }
// 
// 
//     VOMSWrapper V( certfile, verify_ac_sign );
//     if( !V.IsValid( ) ) {
//       
//       logger_instance->log(log4cpp::Priority::FATAL, 
// 			   string("Problems with proxyfile [")+certfile+"]: "+
// 			   V.getErrorMessage(), true, true, true);
//       exit(1);
//     }    
//     
//     VO = V.getVOName( );
// 
//   } catch(soap_ex& s) {
//       log_dev->error( s.what() );
//       exit(1);
//   } catch(auth_ex& auth) {
//       log_dev->error( auth.what() );
//       exit(1);
//   }
// 
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
//       for(vector<string>::const_iterator fit = confiles.begin();
//           fit != confiles.end();
// 	  fit++) 
//       { 
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
//           logfile = logger_instance->getLogFileName(confMgr.getProperty("LIST_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-job-list");
//       }
//       log_dev->info( "Logfile is [%s]", logfile.c_str() );
//       cliUtils::mkdir(cliUtils::getPath(logfile));
//       logger_instance->setLogFile(logfile.c_str());
//   }
// 
//   if( !cliUtils::checkEndpointFormat( endpoint ) )
//     {
//       log_dev->error( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
//       exit(1);
//     }
// 
//   /**
//    *
//    */
//   if(!cliUtils::containsTCPPort( endpoint )) {
//     endpoint = endpoint + ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//   }
// 
//   string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
//     endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);//CEUrl::getCreamURI(URL);
// 
//   if (log_dev->isInfoEnabled()) {
//       log_dev->info( "Service address=[%s]", serviceAddress.c_str() );
//   }
//   
//   vector<JobIdWrapper> jobIdWrapperVetor;
//   
//   AbsCreamProxy* creamClient;
//   creamClient = CreamProxyFactory::make_CreamProxyList(&jobIdWrapperVetor, soap_timeout );
//   if(!creamClient)
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
//       return 1;
//     }
// 
//   creamClient->setCredential( certfile );
//   try {
// 
//      creamClient->execute(serviceAddress);
//      //creamClient->garbage_collect();
// 
//    } catch (exception& ex) {
// 
//      log_dev->error(ex.what());
//      return 1;
//    }
// 
//    string prefixJobId = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+ endpoint + "/";
// 
//    if(jobid_on_file) {
//      bool exists = false;
// 
//      if((exists=cliUtils::fileExists(outfile.c_str()))==true) {
// 	if(!cliUtils::fileIsWritable(outfile.c_str())){
//             log_dev->error("Output file ["+outfile+"] exists but is not accessible. Stop" );
// 	  exit(1);
// 	}
//      }
//      // OK, the output file does exist or doesn't
//      // if it does let's check if the first line has the magic string
//      int f;
//    
//      if(exists) {
//         bool isjoblist = cliUtils::isACreamJobListFile(outfile.c_str());
// 	if(!isjoblist) {
// 	
// 	if(!noint) {
// 	   printf("\nWARNING: file [%s] already exists and is not a CREAM job list file. Overwrite it (y/n) ? ", outfile.c_str());
// 	   char c; cin >> c;
// 	   if(c!='y') {
// 	     printf("\nListing aborted. Bye!\n"); exit(0);
// 	   }
//          }
// 	 // Is NOT a JobList => opens file and truncs it
// 	 try { f=cliUtils::openJobListFile(outfile.c_str(), true); }
// 	 catch(file_ex& fex) {
//              log_dev->error( fex.what() );
//              exit(1);
// 	 }
//         } else {  // if(!isjoblist)
// 	  // opens file in append mode
// 	  try { f=cliUtils::appendToJobListFile(outfile.c_str()); }
// 	  catch(file_ex& fex) {
//               log_dev->error( fex.what() );
//               exit(1);
// 	  }
// 	}
//      } // if(exists)
//      else { //if(exist)
//      	// create file from scratch
// 	try { f=cliUtils::openJobListFile(outfile.c_str(), false); }
// 	catch(file_ex& fex) {
// 	    log_dev->error( fex.what() );
// 	    exit(1);
// 	}
//      }
//      // Writes JobIDs into file; but before doing that
//      // retrieves the list of jobs already in the file to prevent
//      // duplications
//      vector<string> alreadyThere;
//      alreadyThere.reserve(1000);
//      cliUtils::getJobIDFromFile(alreadyThere, outfile.c_str());
//      int count = 0;
//      for(vector<JobIdWrapper>::const_iterator rit = jobIdWrapperVetor.begin();
//          rit != jobIdWrapperVetor.end();
// 	 rit++) {
//        
//        string creamJobId = (*rit).getCreamURL();
//        string::size_type loc = creamJobId.find(confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
//        if( loc != string::npos) {
//          creamJobId = creamJobId.substr(0,loc);
//         }
//        creamJobId += "/";
//        creamJobId.append((*rit).getCreamJobID());
//        if( find(alreadyThere.begin(), alreadyThere.end(), creamJobId ) != alreadyThere.end()){
//            log_dev->warn( "JobID [" + creamJobId + "] already in the list. Skipping it..." );
//            continue;
//         }
// 	try { cliUtils::writeJobID(f, creamJobId ); }
// 	catch(file_ex& fex) {
//             log_dev->error( fex.what() );
// 	  close(f);
// 	  exit(1);
// 	}
// 	count++;
//      }
//      if(log_dev->isInfoEnabled()) {
//        log_dev->info( "Wrote %d job(s) into file [%s]", count, outfile.c_str() );
//      }
//   } else {
//     for(vector<JobIdWrapper>::const_iterator rit = jobIdWrapperVetor.begin();
//         rit != jobIdWrapperVetor.end();
// 	rit++) { //unsigned int j=0; j<result.size(); j++) {
//         string creamJobId = (*rit).getCreamURL();
//         string::size_type loc = creamJobId.find(confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
//         if( loc != string::npos) {
//           creamJobId = creamJobId.substr(0,loc);
//         }
// 	creamJobId += "/";
//         creamJobId.append((*rit).getCreamJobID());  
//         cout << creamJobId.c_str() << endl;
// 
//     }
//    }
//   return 0;
}

void printhelp() {
  printf("%s\n",help_messages::getListHelp().c_str());
}
