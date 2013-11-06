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

#include <cerrno>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/JobStatusWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/JobInfoWrapper.h"
#include "glite/ce/cream-client-api-c/JobCommandWrapper.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "util/cliUtils.h"
#include "services/cli_service_jobstatus.h"
#include "hmessages.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
//#include <boost/date_time/posix_time/ptime.hpp>

using namespace std;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::cream_exceptions;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp(void);
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

//______________________________________________________________________________
void printStatusResult(const pair<string, boost::tuple<bool, JobStatusWrapper, string> >& res) 
{
  printf("\n******  JobID=[%s]\n", res.first.c_str() );
  if(res.second.get<0>()==0) { // The Status has been returned
    printf("\tStatus        = [%s]\n", res.second.get<1>().getStatusName().c_str() );
    
    if(job_statuses::isFinished( job_statuses::getStatusNum(res.second.get<1>().getStatusName()) )) {
      printf("\tExitCode      = [%s]\n", res.second.get<1>().getExitCode().c_str() );
      if(job_statuses::isFailed( job_statuses::getStatusNum(res.second.get<1>().getStatusName().c_str()) )) {
	printf("\tFailureReason = [%s]\n", res.second.get<1>().getFailureReason().c_str());
      }
      //      request by Massimo Sgravatto
      if ( !((res.second.get<1>().getDescription()).empty()) ) {
      	printf("\tDescription   = [%s]\n", res.second.get<1>().getDescription().c_str());
      }
    }
    
    printf( "\n\n");
    
  } else {
    printf("\tFor this job CREAM has returned a fault: %s\n", res.second.get<2>().c_str());
    
    printf( "\n\n");
  }
}

//______________________________________________________________________________
class printInfoResult {
  
  int m_verbLevel;
  
public:
  
  printInfoResult( const int verbLevel ) : m_verbLevel( verbLevel ) {}
  
  void operator()(const pair<string, boost::tuple<bool, JobInfoWrapper, string> >& res) 
  {
    
    int ok = res.second.get<0>();
    string jobId = res.first;
    static struct tm TSTAMP;
    static char tstamp[256];
    
    printf("\n******  JobID=[%s]\n", jobId.c_str() );
    
    if( ok == 0 ) { // The Info has been returned
      JobInfoWrapper info = res.second.get<1>();
      
      vector<JobStatusWrapper> states;
      info.getStatus( states );
      printf("\tCurrent Status = [%s]\n", states.at(states.size()-1).getStatusName().c_str());
      if(m_verbLevel > 1) {
        if( !info.getWorkingDirectory().empty() )
          printf("\tWorking Dir    = [%s]\n", info.getWorkingDirectory().c_str() );
      }
      
      if(job_statuses::isFinished( job_statuses::getStatusNum(states.at(states.size()-1).getStatusName().c_str()) )) {
          printf("\tExitCode       = [%s]\n", states.at(states.size()-1).getExitCode().c_str() );
          if(job_statuses::isFailed( job_statuses::getStatusNum(states.at(states.size()-1).getStatusName().c_str()) )) {
            printf("\tFailureReason  = [%s]\n", states.at(states.size()-1).getFailureReason().c_str());
	  }
 	//comment the verbosity request by Massimo Sgaravatto (mail 12/02/2008)
	
          if ( !((states.at(states.size()-1).getDescription()).empty()) ) {
	  	printf("\tDescription    = [%s]\n", states.at(states.size()-1).getDescription().c_str());
	  }
	
      }
      
      
      if( !info.getGridJobId().empty() )
	printf("\tGrid JobID     = [%s]\n", info.getGridJobId().c_str() );
      

      if(m_verbLevel > 1) {
	if( !info.getLRMSAbsLayerJobId().empty() )
	  printf("\tLRMS Abs JobID = [%s]\n", info.getLRMSAbsLayerJobId().c_str() );
	
	if( !info.getLRMSJobId().empty() )
	  printf("\tLRMS JobID     = [%s]\n", info.getLRMSJobId().c_str() );
      
	if( !info.getDelegationProxyId().empty() )
	  printf("\tDeleg Proxy ID = [%s]\n", info.getDelegationProxyId().c_str() );
	
	string strDelegartionProxyInfoAlign = info.getDelegationProxyInfo();
        boost::replace_all(strDelegartionProxyInfoAlign, "\n", "\n\t\t\t  ");
        
	if( !info.getDelegationProxyInfo().empty() ) 
	  printf("\tDelegProxyInfo = [%s]\n", strDelegartionProxyInfoAlign.c_str() );	
	
        if( !info.getWorkerNode().empty() )
	  printf("\tWorker Node    = [%s]\n", info.getWorkerNode().c_str() );  
	
	if( !info.getLocalUser().empty() )
	  printf("\tLocal User     = [%s]\n", info.getLocalUser().c_str() );

	if( !info.getCreamISBURI().empty() )
	  printf("\tCREAM ISB URI  = [%s]\n", info.getCreamISBURI().c_str() );
	
	if( !info.getCreamOSBURI().empty() )
	  printf("\tCREAM OSB URI  = [%s]\n", info.getCreamOSBURI().c_str() ); 

	if( !info.getJDL().empty() )
	  printf("\tJDL            = [%s]\n", info.getJDL().c_str() );

	if( !info.getType().empty() )
	  printf("\tType           = [%s]\n", info.getType().c_str() );
	
	if( !info.getLeaseId().empty() )
	  printf("\tLease ID       = [%s]\n", info.getLeaseId().c_str() );
	
	if( info.getLeaseTime() ) {
	  time_t leaseT = info.getLeaseTime();
	  localtime_r(&leaseT, &TSTAMP);
	  memset(tstamp, 0, 256);
	  strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	  printf("\tLease Time     = [%s] (%d)\n", tstamp, leaseT );
	}
      }

//      request by Massimo Sgravatto
//      if( !info->getWorkingDirectory().empty() )
//	printf("\tWorking Dir    = [%s]\n", info->getWorkingDirectory().c_str() );
      
      printf("\n\tJob status changes:\n\t-------------------\n");
      
      for(vector<JobStatusWrapper>::const_iterator it=states.begin(); it!= states.end(); ++it) {
	string status = it->getStatusName();
	
	time_t statusChangeTime = it->getTimestamp();
	localtime_r(&statusChangeTime, &TSTAMP);
	memset(tstamp, 0, 256);
	strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	
	printf("\tStatus         = [%s] - [%s] (%d)\n", status.c_str(), tstamp,  it->getTimestamp());
	
      }
      
      printf("\n\tIssued Commands:\n\t-------------------\n");
      vector<JobCommandWrapper> commands;
      info.getCommand( commands );
      
      for(vector<JobCommandWrapper>::const_iterator it=commands.begin(); it!= commands.end(); ++it) {
	printf("\n");
	   
	
	printf("\t*** Command Name              = [%s]\n", it->getCommandName().c_str());
	if( !it->getCommandId().empty() )
	  printf("\t    Command Id                = [%s]\n", it->getCommandId().c_str());
	printf("\t    Command Category          = [%s]\n", it->getCommandCategory().c_str());
	printf("\t    Command Status            = [%s]\n", it->getCommandStatus().c_str());
	
	if( !it->getCommandFailReason().empty() )
	  printf("\t    Command Fail Reason       = [%s]\n", it->getCommandFailReason().c_str());
	
	if(m_verbLevel > 1) {
	  time_t creatTime = it->getCreationTime();
	  time_t startTime = it->getStartSchedulingTime();
	  time_t starPTime = it->getStartProcessingTime();
	  time_t exeCTTime = it->getExecutionCompletedTime();
	
	  if( creatTime ) {
	    localtime_r(&creatTime, &TSTAMP);
	    memset(tstamp, 0, 256);
	    strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	    printf("\t    Creation Time             = [%s] (%d)\n", tstamp, creatTime);
	  }
	
	  if(startTime) {
	    localtime_r(&startTime, &TSTAMP);
	    memset(tstamp, 0, 256);
	    strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	    printf("\t    Start Scheduling Time     = [%s] (%d)\n", tstamp, startTime);
	  }
	
	  if(starPTime) {
	    localtime_r(&starPTime, &TSTAMP);
	    memset(tstamp, 0, 256);
	    strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	    printf("\t    Start Processing Time     = [%s] (%d)\n", tstamp, starPTime);
	  }
	
	  if(exeCTTime) {
	    localtime_r(&exeCTTime, &TSTAMP);
	    memset(tstamp, 0, 256);
	    strftime(tstamp, sizeof(tstamp), "%a %d %b %Y %T", &TSTAMP);
	    printf("\t    Execution Completed Time  = [%s] (%d)\n", tstamp, exeCTTime);
	  }
	}
	printf("\n");
      }
      
    } else {
      
      string errMex = res.second.get<2>();
      //      if( errMex )
      printf("\tFor this job CREAM has returned a fault: %s\n", errMex.c_str());
	//      else
	//	printf("\tFor this job CREAM has returned a fault, but there's not the fault's error message\n");
      
    }
  }
};

//______________________________________________________________________________
// bool execute( AbsCreamProxy* client, 
// 	      const string& url, string & errormex ) throw()
// {
//   try {
//     creamApiLogger::instance()->getLogger()->debug("Contacting service [%s]", url.c_str());
//     client->execute(url);
// 
//   } catch(BaseException& ex) {
//     errormex = ex.what();
//     return false;
//   } catch(exception& ex) {
//     errormex = ex.what();
//     return false;
//   }
// 
//   return true;
// }

/****************************************************************************
 *
 *       M A I N    F U N C T I O N
 *
 ***************************************************************************/
//______________________________________________________________________________
int main(int argc, char *argv[]) {

    string jobID;
    string endpoint;
    string param;
    bool debug = false;
//    bool hostset=false;
    //bool jobset=false;
    //bool specified_status = false;
    //int option_index = 0;
    string logfile, statuses = "";
    //bool jobids_from_file = false;
    vector<string> jobids;
    string input = "";
    map <string, vector<string> > url_jobids;
    vector<string> Status;
    string outputLevel;// = "0";
    int oLevel;
//    bool status_all = false;
    string fromDate = "";
    string toDate = "";
    int TO_timestamp = -1;
    int SINCE_timestamp = -1;
    bool redir_out = false;
    string    user_conf_file = "";
    boost::cmatch what;
    boost::regex pattern;
    string proxyfile, certfile;
    //bool specified_proxyfile = false;
    //creamApiLogger* logger_instance = creamApiLogger::instance();
    //log4cpp::Category* log_dev = logger_instance->getLogger();
    int timeout = 30;

    // Initialize the logger
    //log_dev->setPriority( log4cpp::Priority::NOTICE );
    //logger_instance->setLogfileEnabled( true );

    bool verify_ac_sign = true;
    bool nomsg = false;













    string states;

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
	  "status,s", po::value<string>(&statuses), "Filter on certain jobs's states"
	 )
	 (
	  "input,i", po::value<string>(&input), ""
	 )
	 (
	   "all,a", ""
	 )
	 (
	   "jobid,j", po::value<vector<string> >(&jobids), ""
	 )
	 (
	   "fromDate,f", po::value<string>(&fromDate), ""
	 )
	 (
	   "toDate,t", po::value<string>(&toDate), ""
	 )
	 (
	   "level,L", po::value<string>(&outputLevel), ""
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
      
  if( vm.count("endpoint" ) )
    endpoint = vm["endpoint"].as< string>();
  
  if( vm.count("input") && vm.count("all") ) {
    cerr << "Cannot specify --input and --all options, they're mutually exclusive. Stop." << endl;
    return 1;
  } 
  
  if( !jobids.empty() && vm.count("all") ) {
    cerr << "Cannot specify JobIDs as command line argument and --all option, they're mutually exclusive. Stop" << endl;
    return 1;
  }
  
  if( !vm.count("input") && !vm.count("all") && jobids.empty() ) {
    cerr << "You didn't provide one or more JobID(s) to get status of. Must specify at least one option between --all or --input or specify a JobID as argument. Stop" << endl;
    return 1;
  }
  
  if( vm.count("all") && !vm.count("endpoint") ) {
    cerr << "--all option requires to specify also --endpoint option. Stop." << endl;
    return 1;
  }
  tzset();
  if( !fromDate.empty( ) ) {
      pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
      if( !boost::regex_match(fromDate.c_str(), what, pattern) ) {
        cerr << "Specified from date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop" << endl;
        return 1;
      }
/*      struct tm tmp;
      strptime(fromDate.c_str(), "%Y-%m-%d %T", &tmp);
      SINCE_timestamp = mktime(&tmp); */
    boost::posix_time::ptime since( boost::posix_time::time_from_string(fromDate) );
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration dur = since - epoch;
    SINCE_timestamp = dur.total_seconds() +timezone;
    //cout << "SINCE   = " << fromDate << endl;
    //cout << "TSTAMP  = " << SINCE_timestamp  << endl;
    //cout << "asctime = " << asctime((const struct tm*)(localtime((const time_t*)&tstamp))) << endl;
  }
    
  //cout << "toDate = " << toDate << endl;

  if( !toDate.empty( ) ) {

      pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
      if( !boost::regex_match(toDate.c_str(), what, pattern) ) {
	cerr << "Specified to date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop" << endl;
	return(1);
      }
/*      struct tm tmp;
      strptime(toDate.c_str(), "%Y-%m-%d %T", &tmp);
      TO_timestamp = mktime(&tmp);
*/
    boost::posix_time::ptime to( boost::posix_time::time_from_string(toDate) );
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration dur = to - epoch;
    TO_timestamp = dur.total_seconds() + timezone;
    //cout << "TO      = " << toDate << endl;
    //cout << "TSTAMP  = " << TO_timestamp  << endl;
    //cout << "asctime = " << asctime((const struct tm*)(localtime((const time_t*)&tstamp))) << endl;
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

  apiproxy::AbsCreamProxy::StatusArrayResult Sresult;
  apiproxy::AbsCreamProxy::InfoArrayResult Iresult;

  glite::ce::cream_cli::services::cli_service_jobstatus JS( opt, &Sresult, &Iresult );

  if( vm.count("level") )
    JS.set_verbosity_level( outputLevel );
  
  if( vm.count("input") )
    JS.set_inputfile( input );

  if( vm.count("all") )
    JS.set_status_all( );
  
  JS.set_timerange( SINCE_timestamp, TO_timestamp );
  
  if( vm.count("status" ) ){
    vector<string> states;
    if(statuses.compare("") != 0) {

      boost::split(states, statuses, boost::is_any_of(":"));
        for(vector<string>::const_iterator sit = states.begin();
	    sit != states.end();
	    sit++ ) 
	  {
            int stnum = glite::ce::cream_client_api::job_statuses::getStatusNum( *sit );
            if(stnum<0) {
	      cerr << "Invalid status [" << *sit << "] in argument of option --status. Stop" << endl;
	      return 1;
            }
            //JS.insert_job_to_query( *sit );
	  }
	JS.set_state_filter( states );
    }
  }
      
  for( vector<string>::const_iterator jit = jobids.begin();
       jit != jobids.end();
       ++jit ) 
    {
      JS.insert_job_to_query( *jit );
    }

  if( JS.execute( ) ) {
    JS.getLogger( )->fatal( JS.get_execution_error_message( ) );
    return 1;
  }
  
  oLevel = boost::lexical_cast<int>( JS.get_verbosity_level( ) );

  if(oLevel>0) {
    printInfoResult printInfo( oLevel );
    for_each(Iresult.begin(), Iresult.end(), printInfo);
  } else for_each(Sresult.begin(), Sresult.end(), printStatusResult);


  return 0;


//     while( 1 ) {
//         int c;
//         static struct option long_options[] = {
//             {"help", 0, 0, 'h'},
//             {"debug", 0, 0, 'd'},
//             {"version", 0, 0, 'v'},
//             {"nomsg", 0, 0, 'n'},
//             {"endpoint", 1, 0, 'e'},
//             {"logfile", 1, 0, 'l'},
//             {"status", 1, 0, 's'},
//             {"input", 1, 0, 'i'},
//             {"level", 1, 0, 'L'},
//             {"all",0 ,0 ,'a'},
//             {"from", 1, 0, 'f'},
//             {"to", 1, 0, 't'},
//             {"conf", 1, 0, 'c'},
// 	    {"proxyfile", 1, 0, 'p'},
// 	    {"timeout", 1, 0, 'T'},
// 	    {"donot-verify-ac-sign", 0, 0, 'A'},
//             {0, 0, 0, 0}
//         };
//         c = getopt_long(argc, argv, "hdAve:l:ns:i:L:af:t:c:p:T:", long_options, &option_index);
//         if ( c == -1 )
//             break;
//         switch(c) {
// 	case 'A':
// 	  verify_ac_sign = false;
// 	  break;
// 	case 'p':
// 	  proxyfile = string(optarg);
// 	  specified_proxyfile = true;
// 	  break;
//         case 'l':
// 	  logfile = string(optarg);
// 	  redir_out = true;
// 	  break;
//         case 'f':
// 	  fromDate = string(optarg);
// 	  break;
//         case 't':
// 	  toDate = string(optarg);
// 	  break;
//         case 'a':
// 	  status_all = true;
// 	  break;
//         case 'h':
// 	  printhelp();
// 	  exit(0);
// 	  break;
//         case 's':
// 	  statuses = string(optarg);
// 	  specified_status = true;
// 	  break;
//         case 'i':
// 	  jobids_from_file = true;
// 	  input = string(optarg);
// 	  break;
//         case 'v':
// 	  help_messages::printver();
// 	  exit(0);
// 	  break;
// 	  break;
//         case 'n':
// 	  log_dev->setPriority( log4cpp::Priority::ERROR );
// 	  break;
//         case 'L':
// 	  outputLevel = string(optarg);
// 	  break;
//         case 'd':
// 	  debug=true;
// 	  log_dev->setPriority( log4cpp::Priority::DEBUG );
// 	  break;
//         case 'e':
// 	  endpoint = string(optarg);
// 	  hostset=true;
// 	  break;
//         case 'c':
// 	  user_conf_file = string(optarg);
// 	  break;
// 	case 'T':
// 	  timeout = atoi(optarg);
// 	  break;
//         default:
// 	  cerr << "Type " << argv[0] << " -h for an help" << endl;
// 	  exit(1);
//         }
//     }
 

    /**
     *
     * Initializes CreamProxy interface and setup authN
     *
     */
//    AbsCreamProxy *creamClient;

//     string VO = "";
//     
//     if(!specified_proxyfile) {
//       try {
// 	certfile = cliUtils::getProxyCertificateFile();
//       } catch(exception& ex) {
// 	logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
// 	exit(1);
//       }
//     } else
//       certfile = proxyfile;
//     
//       log_dev->info("Using certificate proxy file [%s]", certfile.c_str() );
//       
//       VOMSWrapper V( certfile, verify_ac_sign  );
//       if( !V.IsValid( ) ) {
// 	
// 	  logger_instance->log(log4cpp::Priority::FATAL, 
// 			       string("Problems with proxyfile [")+certfile+"]: "+
// 			       V.getErrorMessage(), true, true, true);
// 	  exit(1);
// 	
//       }
//       
//       VO = V.getVOName( );
//   
//     /**
//      * Handle the load of configurations
//      */
//     vector<string> confiles;
//     try {
// 
//         confiles = cliUtils::getConfigurationFiles(VO, 
//                                                    user_conf_file, 
//                                                    DEFAULT_CONF_FILE);
// 
//     } catch(no_user_confile_ex& ex) {
//         log_dev->fatal( ex.what() );
//         exit(1);
//     }
// 
//     if ( log_dev->isInfoEnabled() ) {
//       for(vector<string>::const_iterator fit = confiles.begin();
// 	  fit != confiles.end();
// 	  fit++) { 
// 	log_dev->info( "Loading configurations from [%s]...", (*fit).c_str() );
//       }
//     }
// 
//     ConfigurationManager confMgr(ConfigurationManager::classad);
//     try { confMgr.load(confiles); }
//     catch(file_ex& ex) {
//       if(debug)
//         log_dev->warn( string(ex.what())+". Using built-in configuration" );
//     }

//    outputLevel     = confMgr.getProperty("STATUS_VERBOSITY_LEVEL", outputLevel);
//    pattern = "^[0-2]\\b";
//    if( !boost::regex_match(outputLevel.c_str(), what, pattern) ) {
//      log_dev->fatal("Specified output level is wrong: must be a number between 0 and 2. Stop" );
//      exit(1);
//    }
//    oLevel = atoi(outputLevel.c_str());

//    if(debug || redir_out) {
//         if(!redir_out) {
//             logfile = logger_instance->getLogFileName(confMgr.getProperty("STATUS_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-job-status");
//         }
//         log_dev->info( "Logfile is [%s]", logfile.c_str() );
// 	cliUtils::mkdir(cliUtils::getPath(logfile));
//         logger_instance->setLogFile(logfile.c_str());
//     }

//     if( (jobids_from_file && status_all) ||
//         ((argv[optind]!=NULL) && status_all) ) {
//         log_dev->fatal( "Cannot specify Job IDs as command line argument with --all option, they're exclusive. Stop" );
//         exit(1);
//     }
//   
//     if(!jobids_from_file && !status_all && (argv[optind]==NULL)) {
//         log_dev->fatal( "You didn't provide one or more JobID(s) to get status of. Must specify at least on option between --all or --input or specify a JobID as argument. Stop");
//         exit(1);
//     }
//   
//     if(status_all && !hostset) {
//         log_dev->fatal( "--all option requires --endpoint. Stop");
//         exit(1);
//     }

    /** ******************************************************************
     *
     * Load JobIDs from command line argument if any
     *
     */
//     if(argv[optind]!=NULL) {
//         jobset = true;
//         while(argv[optind]!=NULL) {
//             jobids.push_back(string(argv[optind]));
//             optind++;
//         }
//     } 
//     if(jobids_from_file) {
//         bool isjoblist = true;
//         int f;
//         log_dev->info( "Getting job list from file [%s]", input.c_str() );
// 
//         if((f=open(input.c_str(), O_RDONLY))==-1) {
//             int saveerr = errno;
//             log_dev->fatal( "Error accessing file [%s]: %s. Stop.", input.c_str(), strerror(saveerr));
//             exit(1);
//         }
//         close(f);
//     
//         isjoblist = cliUtils::isACreamJobListFile(input.c_str());
//         if(!isjoblist) {
//             log_dev->fatal( "File [%s] is not a CREAM job list file. Stop.", input.c_str() );
//             exit(1);
//         }
//         cliUtils::getJobIDFromFile(jobids, input.c_str());
//     }
  
//     if(status_all && !hostset) {
//         log_dev->fatal( "Option --all requires the specification of the endpoint (option --endpoint) to contact. Stop." );
//         exit(1);
//     }
  

//     if(hostset) {
//       
//       if( !cliUtils::checkEndpointFormat( endpoint ) )
// 	{
// 	  log_dev->fatal( "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.");
// 	  exit(1);
// 	}
//       
//       pattern = "^[^:]+(:[0-9]{1,5})?\\b";
//       if( !boost::regex_match(endpoint.c_str(), what, pattern) ) {
// 	log_dev->fatal("Specified endpoint has wrong format. Must be <host>[:<tcp_port>] (tcp_port is a number of 5 digit max length). Stop" );
// 	exit(1);
//       }
//       pattern = "^[^:]+:[0-9]{1,5}\\b";
//       if( !boost::regex_match(endpoint.c_str(), what, pattern) ) {
// 	endpoint += ":" + confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443");
//       }
//     }

   



//     if(fromDate.compare("")!=0) {
//       pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
//       if( !boost::regex_match(fromDate.c_str(), what, pattern) ) {
//         log_dev->fatal("Specified from date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop" );
//         exit(1);
//       }
//       struct tm tmp;
//       strptime(fromDate.c_str(), "%Y-%m-%d %T", &tmp);
//       SINCE_timestamp = mktime(&tmp);
//     }
//     
// 
// 
//     if(toDate.compare("")!=0) {
//       pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
//       if( !boost::regex_match(toDate.c_str(), what, pattern) ) {
// 	log_dev->fatal("Specified to date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop");
// 	exit(1);
//       }
//       struct tm tmp;
//       strptime(toDate.c_str(), "%Y-%m-%d %T", &tmp);
//       TO_timestamp = mktime(&tmp);
//     }

   

    /**
     *
     * Prepares STATUSes vector to send to CREAM
     *
     */
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
//             Status.push_back( *sit );
//         }
//     }
  
    /**
     *
     * Prepare an hash table [CE-URL]->array_of[JobIDs]
     *
     */
//     for(vector<string>::const_iterator jit = jobids.begin();
//         jit != jobids.end();
// 	jit++) {
//         if(log_dev->isInfoEnabled()) {
//             string os = string("Processing job [") + *jit + "]...";
//             log_dev->debug( os );
//         }
//         if(hostset) {
//             string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
//                 endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
// 
// 	    
// 	    //stripCreamURL stripper( &confMgr);
//             url_jobids[tmp].push_back( *jit );
//         }
//         else {
//             vector<string> pieces;
//             try { CEUrl::parseJobID( *jit , pieces, confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443")); }
//             catch(CEUrl::ceid_syntax_ex& ex) {
//                 log_dev->error( ex.what() );
//                 log_dev->error( "This JobID doesn't contain the CREAM service address. Skipping it..." );
//                 continue;
//             } 
//             string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
//                 pieces[1]+':'+pieces[2]+
//                 "/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
//             url_jobids[tmp].push_back( *jit );
//         }
//     }

//     AbsCreamProxy::StatusArrayResult Sresult;//std::map< std::string, std::pair<bool, StatusResult > >
//     AbsCreamProxy::InfoArrayResult Iresult;//std::map< std::string, std::pair<bool, InfoResult > >

    /*******************************************
     * 
     * ASKS  THE  STATUS  OF  ALL  JOBS
     *
     ******************************************/
//     if(status_all) { // asks for ALL jobs
// 
//       if(oLevel>0) { //invokes the JobInfo (the heavy one)
// 	std::vector< JobIdWrapper> empty;
// 	JobFilterWrapper filter(std::vector< JobIdWrapper>(), Status, SINCE_timestamp, TO_timestamp, "", "");
// 	//getInfo(creamClientendpoint, &confMgr, &filter, &Iresult);
// 
// 	creamClient = CreamProxyFactory::make_CreamProxyInfo( &filter, &Iresult, timeout );
// 	if(!creamClient)
// 	  {
// 	    logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
// 	    return 1;
// 	  }
// 	boost::scoped_ptr< AbsCreamProxy > tmpClient;
// 	tmpClient.reset( creamClient );
// 	creamClient->setCredential( certfile );
// 
// 	endpoint = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
// 	  endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
// 
// 	string result;
// 	if( execute(creamClient, endpoint, result) ) {
// 
// 	  printInfoResult printInfo( oLevel );
// 	  for_each(Iresult.begin(), Iresult.end(), printInfo);//( result );
// 	}else {
// 	      creamApiLogger::instance()->getLogger()->error( result );
// 	    }
// 
//       } else { //invokes the JobStatus (the light one)
// 
// 	std::vector< JobIdWrapper> empty;
// 	JobFilterWrapper filter(std::vector< JobIdWrapper>(), Status, SINCE_timestamp, TO_timestamp, "", "");
// 	//while(1) {
// 	//getStatus(endpoint, &confMgr, &filter, &Sresult);
// 
// 	creamClient = CreamProxyFactory::make_CreamProxyStatus( &filter, &Sresult, timeout );
// 	if(!creamClient)
// 	  {
// 	    logger_instance->log(log4cpp::Priority::FATAL, 
// 				 "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", 
// 				 true, true, true);
// 	    return 1;
// 	  }
// 	boost::scoped_ptr< AbsCreamProxy > tmpClient;
// 	tmpClient.reset( creamClient );
// 	creamClient->setCredential( certfile );
// 	//	creamClient->execute( endpoint );
// 	endpoint = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+endpoint+"/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
// 	string result;
// 	if( execute( creamClient, endpoint, result ) ) {
// 
// 	  for_each(Sresult.begin(), Sresult.end(), printStatusResult);//( result );
// 	}else {
// 	      creamApiLogger::instance()->getLogger()->error( result );
// 	    }
//       }
//       
//       /*******************************************
//        * 
//        * ASKS  THE  STATUS  OF  SELECTED  JOBS
//        *
//        ******************************************/
//     }//  else { // asks for selected jobs
//       
//       if(oLevel>0) { //invokes the JobInfo (the full one)
// 	for(map<string, vector<string> >::iterator it = url_jobids.begin();
//  	    it != url_jobids.end();
//  	    it++) // iterates over endpoints
// 	  {
// 	    vector<JobIdWrapper> target;
// 	    
// 	    stripCreamURL strip( &target, &confMgr );
// 	    for_each(it->second.begin(), it->second.end(), strip);
// 	    JobFilterWrapper filter(target, Status, SINCE_timestamp, TO_timestamp, "", "");
// 	    string url = it->first;
// 	    
// 	    creamClient = CreamProxyFactory::make_CreamProxyInfo( &filter, &Iresult, timeout );
// 	    if(!creamClient)
// 	      {
// 		logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
// 		return 1;
// 	      }
// 	    boost::scoped_ptr< AbsCreamProxy > tmpClient;
// 	    tmpClient.reset( creamClient );
// 	    creamClient->setCredential( certfile );
// 	    string result;
// 	    if(execute( creamClient, url, result )) {
// 
// 	      printInfoResult printInfo( oLevel );
// 	      for_each(Iresult.begin(), Iresult.end(), printInfo);//( result );
// 	    }else {
// 	      creamApiLogger::instance()->getLogger()->error( string("An error occurred with CREAM URL [") + url + "]: " + result );
// 	    }
// 	  }
// 
//       } else {//invokes the JobStatus (the light one)
// 	
// 	for(map<string, vector<string> >::iterator it = url_jobids.begin();
// 	    it != url_jobids.end();
// 	    it++) // iterates over endpoints
// 	  {
// 	    if ( log_dev->isInfoEnabled() ) {
// 	      log_dev->info( string("Sending JobStatus request to [")+(*it).first+"]" );
// 	    }	  
// 	    vector<JobIdWrapper> target;
// 	    
// 	    stripCreamURL strip( &target, &confMgr );
// 	    for_each(it->second.begin(), it->second.end(), strip);	  
// 	    JobFilterWrapper filter(target, Status, SINCE_timestamp, TO_timestamp, "", "");	  
// 	    string url = it->first;
// 
// 	    creamClient = CreamProxyFactory::make_CreamProxyStatus( &filter, &Sresult, timeout );
// 	    if(!creamClient)
// 	      {
// 		logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
// 		return 1;
// 	      }
// 	    boost::scoped_ptr< AbsCreamProxy > tmpClient;
// 	    tmpClient.reset( creamClient );
// 	    creamClient->setCredential( certfile );
// 	    string result;
// 	    if (execute( creamClient, url, result )) {
// 	    
// 	      for_each(Sresult.begin(), Sresult.end(), printStatusResult);	  
// 	    } else {
// 	      creamApiLogger::instance()->getLogger()->error( string("An error occurred with CREAM URL [") + url + "]: " + result );
// 	    }
// 	  }
// 	
//       }
//       
//       return 0;
//     }
}

void printhelp(void) {
    cout << help_messages::getStatusHelp() << endl;
}

