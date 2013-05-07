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

#include <map>
#include <istream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "util/cliUtils.h"
#include "services/cli_service_jobcancel.h"
#include "hmessages.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/regex.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::cream_exceptions;
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
  string logfile;
  bool redir_out = false;
  bool noint = false;
  string joblist_file = "";
  bool nomsg = false;
  vector<string> all_jobids_to_cancel, all_jobids_to_actually_cancel;
  map <string, vector<string> > url_jobids;
  string    user_conf_file = "";
  string certfile;
  string proxyfile;

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
	 ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint to ask the Job IDs to")
	 (
	  "input,i", po::value<string>(&joblist_file), ""
	 )
	 (
	   "all,a", ""
	 )
	 (
	   "jobid,j", po::value<vector<string> >(&all_jobids_to_cancel), ""
	 )
	 (
	   "noint,N", ""
	 )
        ;
    } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
      cerr << "FATAL: " << ex.what() << endl;
      return 1;
    }
	
    po::positional_options_description p;
    p.add("jobid", -1);

	
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
 
  if( vm.count("noint") )
    noint = true;
    
  if( vm.count("debug") )
    debug = true;
    
  if( vm.count("nomsg") )
    nomsg = true;
    
  if( vm.count("donot-verify-ac-sign") )
    verify_ac_sign = false;
    
  if ( vm.count("help") ) {
    printhelp();
    return 0;
  }
  
  if ( vm.count("version") ) {
    help_messages::printver();
    return 0;
  }

  if( !logfile.empty( ) )
      redir_out = true;
      
  if( ( vm.count("all") && vm.count("input") ) || ( !all_jobids_to_cancel.empty() && vm.count("all") )) {
    // ERROR
    cerr << "--all and --input or --all and specification of JobID(s) as argument are mutually exclusive. Stop."<<endl;
    return 1;
  }
      
  if( (all_jobids_to_cancel.empty() && !vm.count("input")) && !vm.count("all")) {
    //ERROR
    cerr << "No Job ID nor JobList file have been specified in the command line arguments. Stop.";
    return 1;
  } 
     
  if(vm.count("all") && !vm.count("endpoint") ) {
    // ERROR
    cerr << "Option --all requires the specification of the endpoint (option --endpoint) to contact. Stop."<<endl;
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
  
//  list<apiproxy::EventWrapper*> result;
  map< string, string> result;
  
  glite::ce::cream_cli::services::cli_service_jobcancel JC( opt, &result );
  
  if( vm.count("input") )
    JC.set_inputfile( joblist_file );
    
  if( vm.count("jobid") ) {
    vector<string>::iterator it = all_jobids_to_cancel.begin( );
    for( ; it != all_jobids_to_cancel.end(); ++it )
    {
      //cout << *it << endl;
      JC.insert_job_to_cancel( *it );
    }
  }
  
  if(vm.count("all"))
    JC.set_cancel_all( );
    
  if( JC.execute( ) )
  {
    JC.getLogger( )->fatal( JC.get_execution_error_message( ) );
    return 1;
  }
  
  for( map<string, string>::iterator it = result.begin();
       it != result.end();
       ++it )
  {
    cout << "Couldn't cancel job [" << it->first << "]: " << it->second << endl;
  }
  
  return 0;
    
//   if( vm.count("endpoint" ) ) {
//     endpoint = vm["endpoint"].as< string>();
//   }
 //  while(1) {
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"endpoint", 1, 0, 'e'},
//       {"logfile", 1, 0, 'l'},
//       {"input", 1, 0, 'i'},
//       {"noint", 0, 0, 'N'},
//       {"all", 0, 0, 'a'},
//       {"nomsg", 0, 0, 'n'},
//       {"conf", 1, 0, 'c'},
//       {"proxyfile", 1, 0, 'p'},
//       {"timeout", 1, 0, 't'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "p:l:nAahdve:i:Nc:t:", long_options, &option_index);
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
//     case 'a':
//       all=true;
//       break;
//     case 'N':
//       noint=true;
//       break;
//     case 'v':
//       help_messages::printver();
//       exit(0);
//       break;
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
//     case 'n':
//       nomsg = true;
//       log_dev->setPriority( log4cpp::Priority::ERROR );
//       break;
//     case 'i':
//       joblist_from_file=true;
//       joblist_file = string(optarg);
//       break;
//     case 'e':
//       endpoint = string(optarg);
//       hostset=true;
//       break;
//     case 'c':
//       user_conf_file = string(optarg);
//       break;
//     case 't':
//       timeout = atoi(optarg);
//       break;
//     default:
//       cerr << "Type " << argv[0] << " -h for an help"<<endl;
//       exit(1);
// 
//     }
//   }
// 
//   if(nomsg && debug) {
//     // ERROR
//     logger_instance->log(log4cpp::Priority::ERROR, "Cannot specify both --debug and --nomsg options. Stop.", false, true, true);
//     exit(1);
//   }
// 
//   AbsCreamProxy *creamClient;
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
// 
//   if(debug)
//     logger_instance->log(log4cpp::Priority::INFO, string("Using certificate proxy file [")
// 			     + certfile + "]", true, true, true);
    
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
// //       } else
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
     * if all is ON 'all_jobids_to_cancel' is an empty vector
     */

  
  /**
   * Handle the load of configurations
   */
  //  vector<string> confiles = cliUtils::getConfigurationFiles(VO,
  //							    user_conf_file, 
  //							    DEFAULT_CONF_FILE);

//   vector<string> confiles;
//   try {
//     confiles = cliUtils::getConfigurationFiles(VO,
// 					       user_conf_file, 
// 					       DEFAULT_CONF_FILE);
//   } catch(no_user_confile_ex& ex) {
//     logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
//     exit(1);
//   }
//   
//   if(debug) {
//     for(vector<string>::const_iterator fit = confiles.begin();
//         fit != confiles.end();
//         fit++) //unsigned int j=0; j<confiles.size(); j++)
//       logger_instance->log(log4cpp::Priority::DEBUG, 
// 			   string("Loading configurations from [")
// 			   + *fit +"]...", true, true, true);
//   }
// 
//   ConfigurationManager confMgr(ConfigurationManager::classad);
//   try { confMgr.load(confiles); }
//   catch(file_ex& ex) {
//     if(debug)
//     logger_instance->log(log4cpp::Priority::WARN, 
// 			   string(ex.what())+". Using built-in configuration",
// 			   true,true,!nomsg);
//   }

//   if(debug || redir_out) {
//     if(!redir_out)
//       logfile = logger_instance->getLogFileName(confMgr.getProperty("CANCEL_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-job-cancel");
//     cliUtils::mkdir(cliUtils::getPath(logfile));
//     logger_instance->setLogFile(logfile.c_str());
//   }
//   if(debug) {
//     string os = string("Logfile is [") + logfile + "]";
// 
//     logger_instance->log(log4cpp::Priority::DEBUG, os, false, false, !nomsg);
//   }







  

  
//   if(debug) {
//     logger_instance->log(log4cpp::Priority::INFO, "*************************************", false, true, !nomsg);
//     logger_instance->log(log4cpp::Priority::INFO, help_messages::getStartMessage().c_str(), false, true, !nomsg);
//   }

//   if(hostset) {
// 
//     if( !cliUtils::checkEndpointFormat( endpoint ) )
//     {
//       log_dev->fatal( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
//       exit(1);
//     }

//     boost::regex pattern( "^[^:]+(:[0-9]{1,5})?\\b" );
//     boost::smatch what;
//     if( !boost::regex_match( endpoint, what, pattern ) ) {
//     //if(!string_manipulation::matches(endpoint,"^[^:]+(:[0-9]{1,5})?\\b")) {
//       logger_instance->log(log4cpp::Priority::ERROR,"Specified endpoint has wrong format. Must be <host>:<tcp_port> (tcp_port is a number of 5 digit max length). Stop", true, WRITE_LOG_ON_FILE,true);
//       exit(1);
//     }

//  }

  /**
   * The following piece of code is executed only if --all hasn't been specified
   *
   */
//   while(argv[optind]!=NULL) {
//     all_jobids_to_cancel.push_back(string(argv[optind]));
//     optind++;
//   }

  /**
   *
   * Gets job IDs from input file and asks the user
   * This piece of code is not executed if all is ON (in
   * fact --all disallows --input specification)
   *
   */
//   string serviceAddress = "";
//   unsigned int jcounter = 0;
//   vector<string> jobnumbers_to_cancel;
//   jobnumbers_to_cancel.reserve(100);
//   if(joblist_from_file) {
//     string errMessage;
// 
//     if(!cliUtils::interactiveChoice("Cancel", joblist_file.c_str(), noint, debug, nomsg, cancel_all_jobs_from_file, jobnumbers_to_cancel, all_jobids_to_cancel, errMessage)) {
//       logger_instance->log(log4cpp::Priority::FATAL, errMessage, true, true, true);
//       exit(1);
//     }
// 
//   }

  /**
   * Add the tcpport if not specified on the command line
   */

//   if(!cliUtils::containsTCPPort(endpoint)) {
//     endpoint = endpoint + ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//     //printf("endpoint=[%s]\n", endpoint.c_str());
//   }

//   if(!all) {
//     /**
//      *
//      * Asks confirmation about cancel of selected job IDs
//      *
//      */
//     char confirm;
//     if(!noint) {
//       printf("\nAre you sure you want to cancel specified job(s) [y/n]: ");
//       cin >> confirm;
//     } else confirm ='y';
// 
//     if(confirm != 'y') {
//       printf("Cancel aborted. Bye.\n");
//       exit(0);
//     }
// 
//     /**
//      *
//      * Filters out the job IDs the user didn't select
//      *
//      */
//     for(jcounter=0; jcounter<all_jobids_to_cancel.size(); jcounter++) {
//       if(joblist_from_file) {
//         if(!cancel_all_jobs_from_file) {
// 	  bool isthere = false;
// 	  for(
// 	      vector<string>::iterator it = jobnumbers_to_cancel.begin();
// 	      it!=jobnumbers_to_cancel.end();
// 	      it++)
// 	    if( jcounter == (unsigned int)atoi( (*it).c_str()) ) { isthere = true; break; }
// 	  if(!isthere) continue;
//         }
// 	if(debug)
// 	  logger_instance->log(log4cpp::Priority::DEBUG, string("Will cancel job [")+all_jobids_to_cancel.at(jcounter)+"]", true, true, true);
// 	all_jobids_to_actually_cancel.push_back(all_jobids_to_cancel.at(jcounter));
//       }
//       else all_jobids_to_actually_cancel.push_back(all_jobids_to_cancel.at(jcounter));
//     }
//     /**
//      *
//      * Create an hash map CEId -> JobIDs
//      *
//      */
//     vector<string> pieces;
//     pieces.reserve(10);
// 
//     for(vector<string>::const_iterator jit = all_jobids_to_actually_cancel.begin();
// 	jit != all_jobids_to_actually_cancel.end();
// 	jit++) { //unsigned int j=0; j<all_jobids_to_actually_cancel.size(); j++) {
//       if(hostset) {
// 	string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
// 	  endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
//         url_jobids[tmp].push_back( /*all_jobids_to_actually_cancel.at(j)*/ *jit );
//       }
//       else {
//         pieces.clear();
// 	//	pieces.reserve(10);
//         try { CEUrl::parseJobID(/*all_jobids_to_actually_cancel.at(j)*/ *jit, pieces, confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443")); }
//         catch(CEUrl::ceid_syntax_ex& ex) {
//           logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, true, true);
//           logger_instance->log(log4cpp::Priority::ERROR,"This JobID doesn't contain the CREAM service address. Skipping it...",true, true, true);
// 	  continue;
//         }
// 	string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
// 	  pieces[1]+":"+pieces[2]+"/"+
// 	  confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
// 	 url_jobids[tmp].push_back( *jit /*all_jobids_to_actually_cancel.at(j)*/ );
//       }
//     }
//   } else {
//     serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO);
//     serviceAddress = serviceAddress + endpoint + "/" + confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
//   }


//  try {
//     if(all && !noint) {
//       char confirm;
//       printf("\nGoing to cancel all job from CE [%s]. Do you want to proceed [y/n]? ",
// 	     serviceAddress.c_str());
//       cin >> confirm;
//       if(confirm!='y') {
//         printf("Cancel aborted. Bye.\n\n");
//         exit(0);
//       }
//     }

//     JobFilterWrapper *wr = NULL;
//     ResultWrapper result;
// 
//     if(all) {
//       
//       logger_instance->log(log4cpp::Priority::INFO, 
// 			   string("Cancel all jobs on [")+serviceAddress+"]...", 
// 			   true, true, true);
//       wr = new JobFilterWrapper(vector<JobIdWrapper>(), vector<string>(), -1, -1, "", "");
//       creamClient = CreamProxyFactory::make_CreamProxyCancel( wr, &result, timeout );
//       if(!creamClient)
// 	{
// 	  logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
// 	  return 1;
// 	}
//       boost::scoped_ptr< AbsCreamProxy > tmpClient;
//       tmpClient.reset( creamClient );
//       creamClient->setCredential( certfile );
//       creamClient->execute( serviceAddress );
//       cliUtils::printResult( result );
//       //creamClient->garbage_collect();
//       delete wr;
//     }
//     else {
//       for(map<string, vector<string> >::iterator it=url_jobids.begin(); it!=url_jobids.end(); it++) {
// 	if(debug)
// 	  logger_instance->log(log4cpp::Priority::INFO, 
// 			       string("Cancel selected jobs on [")+(*it).first+"]...", 
// 			       true, true, true);
// 	vector<JobIdWrapper> target;
// 	stripCreamURL strip( &target, &confMgr );
// 	for_each(it->second.begin(), it->second.end(), strip);
// 	wr = new JobFilterWrapper(target, vector<string>(), -1, -1, "", "");       
// 	creamClient = CreamProxyFactory::make_CreamProxyCancel( wr, &result, timeout );
// 	if(!creamClient)
// 	  {
// 	    logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
// 	    return 1;
// 	  }
// 	boost::scoped_ptr< AbsCreamProxy > tmpClient;
// 	tmpClient.reset( creamClient );
// 	creamClient->setCredential( certfile );
// 	creamClient->execute( (*it).first.c_str() );
// 	cliUtils::printResult( result );
// 	//creamClient->garbage_collect();
// 	delete wr;
//       }
//     }
//   } catch(exception& s) {
//     logger_instance->log(log4cpp::Priority::FATAL, s.what(), true, true, true );
//     exit(1);
//   } 
//   return 0;
}

void printhelp() {
  printf("%s\n",help_messages::getCancelHelp().c_str());
}
