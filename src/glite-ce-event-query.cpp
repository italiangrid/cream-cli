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
#include <list>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <iostream>

#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"

#include "util/cliUtils.h"
#include "hmessages.h"
#include "services/cli_service_event_query.h"
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"
#include <boost/program_options.hpp>

void printhelp(void) ;

using namespace std;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::soap_proxy;
namespace po = boost::program_options;
using boost::lexical_cast;
using boost::bad_lexical_cast;

Namespace namespaces[] = {};
    void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

void execute( AbsCreamProxy* client, const string& url ) throw() {
  try {
    
    client->execute(url);
    
  } catch(exception& ex) {
    creamApiLogger::instance()->getLogger()->fatal( ex.what() );
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  
  boost::cmatch what;
  boost::regex pattern;

  string  endpoint;
  bool    debug = false;
  string  logfile;
  bool    redir_out = false;
  bool    nomsg = false;
  string  user_conf_file = "";
  string  certfile;
  int     timeout = 30;
  string  eventid_range;
  int nevents = 100;
  bool verify_ac_sign = true;

  bool specified_status = false;
  string states;

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
	  "status,s", po::value<string>(&states), "Filter on certain jobs's states"
	 )
	 (
	  "nevents,N", po::value<int>(&nevents), "Set maximum number or events CREAM must return"
	 )
	 (
	  "event-id,E", po::value<string>(&eventid_range), "Set the range extremes of events CREAM must return"
	 )
        ;
	
    po::positional_options_description p;
    p.add("event-id", -1);

	
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
    
  if( !vm.count("endpoint") )
  {
    cerr << "option --endpoint and its argument are mandatory. Stop." << endl;
    return 1;
  }
    
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
      
  if( vm.count("endpoint" ) )
    endpoint = vm["endpoint"].as< string>();
  
  if( vm.count("event-id") ) {
    eventid_range = vm["event-id"].as<string>( );
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
  
  list<apiproxy::EventWrapper*> result;
  glite::ce::cream_cli::services::cli_service_event_query EQ( opt, &result );
  
  pattern = "^([0-9]+)-[0-9]*";
  if( !boost::regex_match(eventid_range.c_str(), what, pattern) ) {
    EQ.getLogger()->fatal("Specified event id range is wrong; must have the format 'number-number'. Stop");
    return 1;
  }
  
  
  vector<string> event_ids;
  boost::split(event_ids, eventid_range, boost::is_any_of("-"));
  
  if(event_ids.size()<2)
    EQ.set_events( event_ids[0], "-1" );
  else
    EQ.set_events( event_ids[0], event_ids[1] );
  
  if( vm.count("nevents") ) {
    EQ.set_num_events( vm["nevents"].as<int>( ) );
  }
  
  if( vm.count("status") ) {
  
    vector<pair<string,string> > Status;
    vector<string> states_pieces;
    if(states.compare("") != 0) {

      boost::split(states_pieces, states, boost::is_any_of(":"));
      for(vector<string>::const_iterator sit = states_pieces.begin();
	    sit != states_pieces.end();
	    sit++ ) 
	  {
            int stnum = glite::ce::cream_client_api::job_statuses::getStatusNum( *sit );
            if(stnum<0) {
	      EQ.getLogger()->fatal( string("Invalid status [")+ *sit +"]. Stop" );
	      exit(1);
            }
            Status.push_back( make_pair("STATUS", *sit) );
        }
      
      EQ.set_status( Status );
    
    } 
  }
  
  if(EQ.execute( ))
  {
    EQ.getLogger()->fatal( EQ.get_execution_error_message( ) ) ;
    return 1;
  }
  
  
//  EQ.get_events( result );
  
  for(list<apiproxy::EventWrapper*>::const_iterator it = result.begin();
      it != result.end();
      ++it)
    {
      cout << "EventID=[" << (*it)->id << "]" << endl;
      
      string ts;
      try {
        ts = boost::lexical_cast<string>( (*it)->timestamp );
      } catch( boost::bad_lexical_cast& ) {}
      
      cout << "timestamp=[" << ts << "]" << endl;
      map<string, string> properties;
      (*it)->get_event_properties( properties );
      for(map<string, string>::const_iterator pit = properties.begin();
	  pit != properties.end();
	  ++pit)
	{
	  if(pit->first == "type")
	    cout << "  \t[status]=[" << glite::ce::cream_client_api::job_statuses::job_status_str[atoi(pit->second.c_str())] << "]" << endl;
	  else {
	    if(pit->first == "jobId") {
	      string completeID = EQ.getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO) + endpoint + "/" + pit->second;
	      cout << "  \t[" << pit->first << "]=[" << completeID << "]"<<endl;
	    } else 
	      cout << "  \t[" << pit->first << "]=[" << pit->second << "]"<<endl;
	  }
	}
	cout << endl;
    }
  
  return 0;  
  
//   while(1) {
//     static struct option long_options[] = {
//       {"help", 0, 0, 'h'},
//       {"debug", 0, 0, 'd'},
//       {"version", 0, 0, 'v'},
//       {"endpoint", 1, 0, 'e'},
//       {"logfile", 1, 0, 'l'},
//       {"nomsg", 0, 0, 'n'},
//       {"conf", 1, 0, 'c'},
//       {"proxyfile", 1, 0, 'p'},
//       {"timeout", 1, 0, 't'},
//       {"nevents", 1, 0, 'N'},
//       {"status", 1, 0, 's'},
//       {"donot-verify-ac-sign", 0, 0, 'A'},
//       {0, 0, 0, 0}
//     };
//     c = getopt_long(argc, argv, "s:p:l:nhdAve:c:t:N:", long_options, &option_index);
//     if ( c == -1 )
//       break;
// 
//     switch(c) {
//     case 's':
//       statuses = string(optarg);
//       specified_status = true;
//       break;
//     case 'A':
//       verify_ac_sign = false;
//       break;
//     case 'p':
//       proxyfile = string(optarg);
//       specified_proxyfile = true;
//       break;
//     
//     
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
//     case 'N':
//       nevents = string( optarg );
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

//   if(nomsg && debug) {
//     // ERROR
//     logger_instance->log(log4cpp::Priority::FATAL, "Cannot specify both --debug and --nomsg options. Stop.", false, true, true);
//     exit(1);
//   }
// 
//   AbsCreamProxy* creamClient;
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
//   if(debug)
//     logger_instance->log(log4cpp::Priority::INFO, string("Using certificate proxy file [")
// 			   + certfile + "]", true, true, true);
// 
//   VOMSWrapper V( certfile, verify_ac_sign  );
//   if( !V.IsValid( ) ) {
//     
//     logger_instance->log(log4cpp::Priority::FATAL, 
// 			 string("Problems with proxyfile [")+certfile+"]: "+
// 			 V.getErrorMessage(), true, true, true);
//     exit(1);
//     
//   }
//   
//   VO = V.getVOName( );
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
//     logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
//     exit(1);
//   }
//   
//   if(debug) {
//     for(vector<string>::const_iterator fit = confiles.begin();
//         fit != confiles.end();
//         fit++)
//       logger_instance->log(log4cpp::Priority::DEBUG, 
// 			   string("Loading configurations from [")
// 			   + *fit +"]...", true, true, true);
//   }
  
  
//   ConfigurationManager confMgr(ConfigurationManager::classad);
//   try { confMgr.load(confiles); }
//   catch(file_ex& ex) {
//     if(debug)
//     logger_instance->log(log4cpp::Priority::WARN, 
// 			 string(ex.what())+". Using built-in configuration",
// 			 true, true,!nomsg);
//   }
//   
//   if(debug || redir_out) {
//     if(!redir_out)
//       logfile = logger_instance->getLogFileName(confMgr.getProperty("EVENTQUERY_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-event-query");
//     cliUtils::mkdir(cliUtils::getPath(logfile));
//     logger_instance->setLogFile(logfile.c_str());
//   }
//   if(debug) {
//     string os = string("Logfile is [") + logfile + "]";
//     
//     logger_instance->log(log4cpp::Priority::DEBUG, os, false, false, !nomsg);
//   }

//   if( argv[optind]==NULL ) {
//     //ERROR
//     logger_instance->log(log4cpp::Priority::FATAL,
// 			 "Must specify the event_id range in the format 'number-number'", false, true, true);
//     exit(1);
//   } else {
// 
//     eventid_range = argv[optind];
//     pattern = "^([0-9]+)-[0-9]*";
//     if( !boost::regex_match(eventid_range.c_str(), what, pattern) ) {
//       log_dev->fatal("Specified event id range is wrong; must have the format 'number-number'. Stop");
//       exit(1);
//     }
// 
//   }

//   if(debug) {
// 
//     logger_instance->log(log4cpp::Priority::INFO, 
// 			 "*************************************", 
// 			 false, true, !nomsg);
// 
//     logger_instance->log(log4cpp::Priority::INFO, 
// 			 help_messages::getStartMessage().c_str(), 
// 			 false, true, !nomsg);
//   }
// 
// 
// 
//   if(hostset) {
//     if( !cliUtils::checkEndpointFormat( endpoint ) )
//       {
// 	log_dev->fatal( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
// 	exit(1);
//       }
//   }

//   vector<string> event_ids;
//   boost::split(event_ids, eventid_range, boost::is_any_of("-"));
// 
//   time_t exec_time;
//   string db_id;
//   list<EventWrapper*> result;
//   string evtType = "JOB_STATUS";
// 
//   string fromid = event_ids[0];
// 
//   string toid;
//   if(event_ids.size() < 2)
//     toid = "-1";
//   else
//     toid   = event_ids[1];
// 
//     vector<pair<string,string> > Status;
//     vector<string> states;
//     if(statuses.compare("") != 0) {
// 
//       boost::split(states, statuses, boost::is_any_of(":"));
//         for(vector<string>::const_iterator sit = states.begin();
// 	    sit != states.end();
// 	    sit++ ) 
// 	  {
//             int stnum = glite::ce::cream_client_api::job_statuses::getStatusNum( *sit );
//             if(stnum<0) {
// 	      log_dev->fatal( string("Invalid status [")+ *sit +"]. Stop" );
// 	      exit(1);
//             }
//             Status.push_back( make_pair("STATUS", *sit) );
//         }
//     }
//   
//   creamClient = 
//     CreamProxyFactory::make_CreamProxy_QueryEvent( make_pair(fromid, toid),
// 						   make_pair( (time_t)-1, (time_t)-1 ), 
// 						   evtType, 
// 						   atoll(nevents.c_str()), 
// 						   0, 
// 						   Status,
// 						   exec_time, 
// 						   db_id, 
// 						   result, 
// 						   timeout );
// 
//   if(!creamClient)
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
//       return 1;
//     }
//   boost::scoped_ptr< AbsCreamProxy > tmpClient;
//   tmpClient.reset( creamClient );
//   
//   creamClient->setCredential( certfile );
//   
//   string serviceAddress = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO);
//   serviceAddress = serviceAddress + endpoint + "/" + confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
//   
//   string baseServiceAddr = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO) + endpoint +"/";
// 
//   
//   log_dev->debug( string("Connecting to service [") + serviceAddress + "]" );
//   execute( creamClient, serviceAddress );
//   
//   log_dev->debug( string("Database ID = [") + db_id +"]");
//   
//   string et;
//   try {
//     et = boost::lexical_cast<string>(exec_time);
//   }catch( boost::bad_lexical_cast&) {}
//   
//   log_dev->debug( string("Exec time   = [") + et/*tmp.str()*/ + "ms]");
//   for(list<EventWrapper*>::const_iterator it = result.begin();
//       it != result.end();
//       ++it)
//     {
//       logger_instance->log(log4cpp::Priority::INFO, string("EventID=[") + (*it)->id + "]", true, true, true);
//       
//       string ts;
//       try {
//         ts = boost::lexical_cast<string>( (*it)->timestamp );
//       } catch( boost::bad_lexical_cast& ) {}
//       
//       logger_instance->log(log4cpp::Priority::INFO, string("timestamp=[") + ts + "]", true, true, true);
//       map<string, string> properties;
//       (*it)->get_event_properties( properties );
//       for(map<string, string>::const_iterator pit = properties.begin();
// 	  pit != properties.end();
// 	  ++pit)
// 	{
// 	  if(pit->first == "type")
// 	    logger_instance->log( log4cpp::Priority::INFO, string("  \t[status]=[") + job_statuses::job_status_str[atoi(pit->second.c_str())] + "]", true, true, true);
// 	  else {
// 	    if(pit->first == "jobId") {
// 	      string completeID = baseServiceAddr + pit->second;
// 	      logger_instance->log( log4cpp::Priority::INFO, string("  \t[") + pit->first + "]=[" + completeID + "]", true, true, true);
// 	    } else 
// 	      logger_instance->log( log4cpp::Priority::INFO, string("  \t[") + pit->first + "]=[" + pit->second + "]", true, true, true);
// 	  }
// 	}
//     }
}

void printhelp() {
  printf("%s\n",help_messages::getQueryEventHelp().c_str());
}
